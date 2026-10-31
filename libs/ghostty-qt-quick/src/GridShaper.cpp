// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "GridShaper_p.h"
#include "Symbols_p.h"

#include <QGlyphRun>
#include <QHash>
#include <QRectF>
#include <QTextLayout>
#include <QTextOption>
#include <algorithm>
#include <array>
#include <utility>

namespace GhosttyQt::Quick
{
bool isFontIcon(const QString &text)
{
    if (text.isEmpty()) {
        return false;
    }

    const char32_t cp{text.size() > 1 && text[0].isHighSurrogate() && text[1].isLowSurrogate() ? QChar::surrogateToUcs4(text[0], text[1]) : text[0].unicode()};

    return QChar::category(cp) == QChar::Other_PrivateUse && !(cp >= 0xe0b0 && cp <= 0xe0d7) && !isProceduralSymbol(cp);
}

void GridShaper::setMetrics(const FontMetrics &metrics, qreal devicePixelRatio)
{
    m_metrics = metrics;
    m_dpr = devicePixelRatio;
    m_cache.clear();
}

int GridShaper::cacheSize() const
{
    return m_cache.size();
}

QVector<PositionedGlyph> GridShaper::shape(const QVector<Cell> &cells, int begin, int end, bool *cacheMiss)
{
    int iconColumns{isFontIcon(cells[begin].text) ? int(cells[begin].width) : 0};

    if (iconColumns == 1 && end < cells.size() && (begin == 0 || !isFontIcon(cells[begin - 1].text))) {
        const Cell &next = cells[end];

        if (next.width == 1 && !(next.flags & ImagePlaceholder) && (next.text.isEmpty() || next.text == QLatin1String(" ") || next.text == QChar(0x2002))) {
            iconColumns = 2;
        }
    }

    // LRO keeps Qt from reordering cells in Arabic/Hebrew runs while still shaping them.
    QString text(QChar(0x202d));
    QVector<int> columns{0};
    QString key;
    key += QChar(ushort(iconColumns));
    key += QChar(cells[begin].flags & (Bold | Italic));

    for (int column{begin}; column < end; ++column) {
        const Cell &cell = cells[column];

        if (cell.width == 0) {
            continue;
        }

        const QString contents{cell.text.isEmpty() ? QStringLiteral(" ") : cell.text};
        key += QChar(ushort(contents.size() & 0xffff));
        key += QChar(ushort(quint64(contents.size()) >> 16));
        key += QChar(ushort(cell.width));
        key += contents;
        text += contents;

        for (qsizetype unit{0}; unit < contents.size(); ++unit) {
            columns += column - begin;
        }
    }

    if (auto *cached = m_cache.object(key)) {
        if (cacheMiss) {
            *cacheMiss = false;
        }

        return *cached;
    }

    if (cacheMiss) {
        *cacheMiss = true;
    }

    const int textEnd{int(text.size())};
    columns += end - begin;
    text += QChar(0x202c);
    columns += end - begin;

    QFont font{m_metrics.pixelFont};

    if (cells[begin].flags & Bold) {
        constexpr std::array weights{QFont::Thin, QFont::Light, QFont::Normal, QFont::Bold, QFont::Black};
        const auto heavier = std::upper_bound(weights.cbegin(), weights.cend(), font.weight());
        const auto weight = heavier != weights.cend() ? *heavier : font.weight();

        if (weight != font.weight()) {
            // A named face can override weight/style matching. Keep it only for the unchanged base face.
            font.setStyleName({});
            font.setWeight(weight);
        }
    }

    if ((cells[begin].flags & Italic) && font.style() == QFont::StyleNormal) {
        font.setStyleName({});
        font.setStyle(QFont::StyleItalic);
    }

    QTextLayout layout{text, font};
    QTextOption option{Qt::AlignLeft};
    option.setWrapMode(QTextOption::NoWrap);
    option.setTextDirection(Qt::LeftToRight);
    option.setFlags(QTextOption::IncludeTrailingSpaces);
    layout.setTextOption(option);
    layout.beginLayout();
    QTextLine line{layout.createLine()};
    line.setLineWidth(1e9);
    layout.endLayout();
    const auto runs =
        layout.glyphRuns(1, textEnd - 1, QTextLayout::RetrieveGlyphIndexes | QTextLayout::RetrieveGlyphPositions | QTextLayout::RetrieveStringIndexes);
    QVector<int> boundaries{textEnd};

    for (const auto &run : runs) {
        const auto stringIndexes = run.stringIndexes();

        for (const auto index : stringIndexes) {
            boundaries += int(index);
        }
    }

    std::sort(boundaries.begin(), boundaries.end());
    boundaries.erase(std::unique(boundaries.begin(), boundaries.end()), boundaries.end());
    QVector<PositionedGlyph> shaped;
    const qreal cellWidth{m_metrics.cellSize.width() * m_dpr};
    const qreal cellHeight{m_metrics.cellSize.height() * m_dpr};
    const qreal baseline{m_metrics.baseline * m_dpr};
    const QRawFont primary{QRawFont::fromFont(font)};
    struct FittedCluster {
        QRectF bounds;
        int columns{1};
    };
    QHash<int, FittedCluster> fittedClusters;

    for (const auto &run : runs) {
        const auto indexes = run.glyphIndexes();
        const auto positions = run.positions();
        const auto sources = run.stringIndexes();
        const QRawFont raw{run.rawFont()};
        const bool fallback{raw.familyName() != primary.familyName()};

        for (qsizetype i{0}; i < indexes.size(); ++i) {
            const int source{int(sources[i])};

            if (source < 1 || source >= textEnd) {
                continue;
            }

            const int next{*std::upper_bound(boundaries.cbegin(), boundaries.cend(), source)};
            const int firstColumn{columns[source]};
            const int span{qMax(1, columns[next] - firstColumn)};
            const qreal origin{line.cursorToX(source)};

            if (fallback || iconColumns) {
                fittedClusters.tryEmplace(firstColumn);
            }

            shaped.append(PositionedGlyph{raw,
                                          indexes[i],
                                          QPointF(firstColumn * cellWidth + positions[i].x() - origin, baseline + positions[i].y() - line.ascent()),
                                          1,
                                          firstColumn,
                                          span});
        }
    }

    if (!fittedClusters.isEmpty()) {
        // Include attached marks from other font runs in the cluster's ink bounds.
        for (const auto &glyph : std::as_const(shaped)) {
            const auto cluster = fittedClusters.find(glyph.firstColumn);

            if (cluster == fittedClusters.end()) {
                continue;
            }

            const QRectF bounds{glyph.font.boundingRect(glyph.index)};

            if (!bounds.isEmpty()) {
                cluster->bounds = cluster->bounds.united(bounds.translated(glyph.baseline - QPointF(glyph.firstColumn * cellWidth, 0)));
            }

            cluster->columns = qMax(cluster->columns, glyph.columnCount);
        }

        for (auto &glyph : shaped) {
            const auto cluster = fittedClusters.constFind(glyph.firstColumn);

            if (cluster == fittedClusters.cend() || cluster->bounds.isEmpty()) {
                continue;
            }

            const qreal width{cluster->columns * cellWidth};
            qreal scale{std::min({qreal(1), width / cluster->bounds.width(), cellHeight / cluster->bounds.height()})};

            if (iconColumns) {
                const qreal singleHeight{m_metrics.iconHeight * m_dpr};
                const qreal singleScale{qMin(cellWidth / cluster->bounds.width(), singleHeight / cluster->bounds.height())};
                const qreal availableHeight{iconColumns > 1 ? cellHeight : singleHeight};
                // Fit available space, but only enlarge small icons enough to fill one cell.
                scale = qMin(qMax(qreal(1), singleScale), qMin(iconColumns * cellWidth / cluster->bounds.width(), availableHeight / cluster->bounds.height()));
            }

            const QRectF ink{QPointF(cluster->bounds.left() * scale, baseline + (cluster->bounds.top() - baseline) * scale), cluster->bounds.size() * scale};
            const qreal horizontalSpace{qMax(qreal(0), width - ink.width())};
            const qreal verticalSpace{qMax(qreal(0), cellHeight - ink.height())};
            const qreal left{iconColumns ? qMax(qreal(0), (cellWidth - ink.width()) / 2)
                                         : (cluster->columns > 1 ? horizontalSpace / 2 : qBound(qreal(0), ink.left(), horizontalSpace))};
            const qreal top{iconColumns ? verticalSpace / 2 : qBound(qreal(0), ink.top(), verticalSpace)};
            const QPointF anchor{glyph.firstColumn * cellWidth, baseline};

            glyph.baseline = anchor + (glyph.baseline - anchor) * scale + QPointF(left - ink.left(), top - ink.top());
            glyph.scale = scale;
        }
    }

    m_cache.insert(key, new QVector<PositionedGlyph>(shaped), key.size());

    return shaped;
}
}
