// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "GlyphAtlas_p.h"
#include "Symbols_p.h"

#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QQuickWindow>
#include <QtGui/private/qrawfont_p.h>
#include <QtMath>
#include <numbers>
#include <numeric>
#include <utility>

namespace GhosttyQt::Quick
{
namespace
{
void drawPaddedImage(QPainter &painter, const QImage &image, QPoint position)
{
    painter.drawImage(position, image);

    // Repeat edge pixels for transformed sprites that use linear filtering.
    for (const int y : {-1, image.height()}) {
        const int sourceY{qBound(0, y, image.height() - 1)};
        painter.drawImage(QRect(position.x(), position.y() + y, image.width(), 1), image, QRect(0, sourceY, image.width(), 1));
        painter.fillRect(QRect(position.x() - 1, position.y() + y, 1, 1), image.pixelColor(0, sourceY));
        painter.fillRect(QRect(position.x() + image.width(), position.y() + y, 1, 1), image.pixelColor(image.width() - 1, sourceY));
    }

    for (const int x : {-1, image.width()}) {
        painter.drawImage(QRect(position.x() + x, position.y(), 1, image.height()), image, QRect(qBound(0, x, image.width() - 1), 0, 1, image.height()));
    }
}
}

GlyphAtlas::GlyphAtlas(QQuickWindow *window, quint64 *rasterizedGlyphs, std::unique_ptr<GlyphAtlas> source)
    : m_window{window}
    , m_rasterized{rasterizedGlyphs}
    , m_source{std::move(source)}
{
}

GlyphImage GlyphAtlas::find(const Key &key)
{
    const auto it = m_glyphs.constFind(key);

    if (it == m_glyphs.cend()) {
        if (m_source) {
            const auto old = m_source->find(key);

            if (old.page) {
                return insert(key, old.page->image.copy(old.pixels), old.bearing, old.format, false);
            }
        }

        return {};
    }

    if (auto page = it->page.lock()) {
        page->lastUsed = m_frame;

        return {std::move(page), it->pixels, it->bearing, it->format};
    }

    m_glyphs.remove(key);

    return {};
}

GlyphImage GlyphAtlas::insert(const Key &key, const QImage &image, QPointF bearing, GlyphImage::Format format, bool rasterized)
{
    if (image.isNull()) {
        return {};
    }

    std::shared_ptr<AtlasPage> page;
    const int padding{key.kind == Kind::Font ? 0 : 1};
    const QSize size{image.size() + QSize(2 * padding, 2 * padding)};

    for (auto it = m_pages.rbegin(); it != m_pages.rend(); ++it) {
        const auto &candidate = *it;

        if (candidate->x + size.width() + 1 > candidate->image.width()) {
            candidate->y += candidate->rowHeight + 1;
            candidate->x = 1;
            candidate->rowHeight = 0;
        }

        if (candidate->x + size.width() + 1 <= candidate->image.width() && candidate->y + size.height() + 1 <= candidate->image.height()) {
            page = candidate;
            break;
        }
    }

    if (!page) {
        page = std::make_shared<AtlasPage>();

        // Start small for ASCII sessions, then grow pages to reduce CJK texture batches.
        const int pageSize{m_pages.empty() ? 256 : 512};
        const int extent{qMax(pageSize, int(qNextPowerOfTwo(quint32(qMax(size.width(), size.height()) + 2))))};
        page->image = QImage(extent, extent, QImage::Format_ARGB32_Premultiplied);
        page->image.fill(Qt::transparent);
        m_pages.push_back(page);
    }

    const QRect rect(page->x + padding, page->y + padding, image.width(), image.height());
    QPainter painter{&page->image};
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    if (padding) {
        drawPaddedImage(painter, image, rect.topLeft());
    } else {
        painter.drawImage(rect.topLeft(), image);
    }

    painter.end();

    page->x += size.width() + 1;
    page->rowHeight = qMax(page->rowHeight, size.height());
    page->dirty = true;
    page->lastUsed = m_frame;
    m_glyphs.insert(key, CachedGlyph{page, rect, bearing, format});

    if (m_rasterized) {
        *m_rasterized += rasterized;
    }

    return {std::move(page), rect, bearing, format};
}

GlyphImage GlyphAtlas::glyph(const QRawFont &font, quint32 index, bool allowSubpixel)
{
    if (!font.isValid()) {
        return {};
    }

    // Request LCD rendering only when enabled by the platform's font policy.
    auto *engine = QRawFontPrivate::get(font)->fontEngine;
    const bool subpixel{allowSubpixel && engine->glyphFormat == QFontEngine::Format_A32};
    const Key key{font, index, Kind::Font, subpixel};

    if (auto found = find(key); found.page) {
        return found;
    }

    QImage source{font.alphaMapForGlyph(index, subpixel ? QRawFont::SubPixelAntialiasing : QRawFont::PixelAntialiasing)};
    const bool colored = source.format() == QImage::Format_ARGB32_Premultiplied || source.format() == QImage::Format_ARGB32
        || source.format() == QImage::Format_RGBA8888_Premultiplied || source.format() == QImage::Format_RGBA8888;
    const auto format = colored ? QFontEngine::Format_ARGB
        : subpixel              ? QFontEngine::Format_A32
        : source.depth() == 1   ? QFontEngine::Format_Mono
                                : QFontEngine::Format_A8;
    // Font metrics can include rows or columns missing from the rasterized glyph.
    const auto bounds = engine->alphaMapBoundingBox(index, {}, {}, format);
    const QPointF bearing(bounds.x.truncate(), bounds.y.truncate());

    if (subpixel && source.format() == QImage::Format_RGB32) {
        // Opaque alpha prevents premultiplication from altering the RGB coverage channels.
        return insert(key, source.convertToFormat(QImage::Format_ARGB32_Premultiplied), bearing, GlyphImage::Format::Subpixel);
    }

    if (colored) {
        return insert(key, source.convertToFormat(QImage::Format_ARGB32_Premultiplied), bearing, GlyphImage::Format::Color);
    }

    QImage image(source.size(), QImage::Format_ARGB32_Premultiplied);

    for (int y{0}; y < source.height(); ++y) {
        auto *destination = reinterpret_cast<QRgb *>(image.scanLine(y));

        for (int x{0}; x < source.width(); ++x) {
            const int alpha{source.format() == QImage::Format_Alpha8 ? source.constScanLine(y)[x] : qGray(source.pixel(x, y))};
            destination[x] = qRgba(alpha, alpha, alpha, alpha);
        }
    }

    return insert(key, image, bearing, GlyphImage::Format::Grayscale);
}

GlyphImage GlyphAtlas::symbol(char32_t cp, QSizeF cellSize, int strokeWidth)
{
    const Key key{{}, quint32(cp), Kind::Symbol, false, cellSize.width(), cellSize.height(), strokeWidth};
    const QSize minimum(qMax(1, qFloor(cellSize.width())), qMax(1, qFloor(cellSize.height())));
    const QSize maximum(qMax(1, qCeil(cellSize.width())), qMax(1, qCeil(cellSize.height())));
    auto sprite = find(key);

    if (!sprite.page) {
        const int columns{minimum.width() != maximum.width() ? 2 : 1};
        const int rows{minimum.height() != maximum.height() ? 2 : 1};
        QImage image(minimum.width() + (columns - 1) * (maximum.width() + 2),
                     minimum.height() + (rows - 1) * (maximum.height() + 2),
                     QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);
        QPainter painter{&image};
        painter.setCompositionMode(QPainter::CompositionMode_Source);

        // The shader chooses a size after snapping the cell's screen-space edges.
        for (int y{0}; y < rows; ++y) {
            for (int x{0}; x < columns; ++x) {
                drawPaddedImage(painter, symbolImage(cp, minimum + QSize(x, y), strokeWidth), QPoint(x * (minimum.width() + 2), y * (minimum.height() + 2)));
            }
        }

        painter.end();
        sprite = insert(key, image, {}, GlyphImage::Format::Grayscale);
    }

    sprite.pixels.setSize(minimum);
    sprite.hasSizeVariants = minimum != maximum;

    return sprite;
}

GlyphImage GlyphAtlas::underline(int style, QSize cellSize, int strokeWidth)
{
    const Key key{{}, quint32(style), Kind::Underline, false, qreal(cellSize.width()), qreal(cellSize.height()), strokeWidth};

    if (auto found = find(key); found.page) {
        return found;
    }

    const int width{cellSize.width()};
    const qreal radius{strokeWidth / std::numbers::sqrt2};
    const int above{qCeil(style == 3 ? strokeWidth / 2.0 : radius - strokeWidth / 2.0)};
    const qreal extent{style == 3 ? width / std::numbers::pi + strokeWidth / 2.0 : radius + strokeWidth / 2.0};
    const int height{qMin(cellSize.height(), qCeil(above + extent))};
    constexpr int scale{4};
    QImage image(width * scale, height * scale, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter{&image};
    painter.scale(scale, scale);
    painter.setRenderHint(QPainter::Antialiasing);

    if (style == 3) {
        const qreal top{qreal(above)};
        const qreal bottom{qMin(top + width / std::numbers::pi, height - strokeWidth / 2.0)};
        QPainterPath wave;
        wave.moveTo(0, bottom);
        wave.cubicTo(width / 6.0, bottom, width / 3.0, top, width / 2.0, top);
        wave.cubicTo(2 * width / 3.0, top, 5 * width / 6.0, bottom, width, bottom);
        painter.setPen(QPen(Qt::white, strokeWidth, Qt::SolidLine, Qt::FlatCap));
        painter.drawPath(wave);
    } else {
        const int count{qMax(1, int(width / (2 * radius + 1)))};
        const qreal spacing{qreal(width) / count};
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::white);

        for (int dot{0}; dot < count; ++dot) {
            painter.drawEllipse(QPointF((dot + 0.5) * spacing, above + strokeWidth / 2.0), radius, radius);
        }
    }

    painter.end();

    return insert(key, image.scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation), QPointF(0, -above), GlyphImage::Format::Grayscale);
}

void GlyphAtlas::beginFrame()
{
    ++m_frame;
}

void GlyphAtlas::flush(quint64 *uploadedBytes)
{
    m_source.reset();

    for (const auto &page : m_pages) {
        if (page->dirty) {
            page->texture.reset(m_window->createTextureFromImage(page->image, QQuickWindow::TextureHasAlphaChannel));

            if (page->texture) {
                page->texture->setFiltering(QSGTexture::Nearest);

                if (uploadedBytes) {
                    *uploadedBytes += page->image.sizeInBytes();
                }

                page->dirty = false;
            }
        }
    }
}

void GlyphAtlas::trim()
{
    qsizetype total{byteSize()};

    while (total > PixelBudget) {
        auto oldest = m_pages.end();

        for (auto it = m_pages.begin(); it != m_pages.end(); ++it) {
            if (it->use_count() == 1 && (oldest == m_pages.end() || (*it)->lastUsed < (*oldest)->lastUsed)) {
                oldest = it;
            }
        }

        if (oldest == m_pages.end()) {
            break;
        }

        total -= (*oldest)->image.sizeInBytes();
        m_pages.erase(oldest);
    }

    m_glyphs.removeIf([](auto it) {
        return it->page.expired();
    });
}

int GlyphAtlas::pageCount() const
{
    return int(m_pages.size());
}

qsizetype GlyphAtlas::byteSize() const
{
    return std::accumulate(m_pages.cbegin(), m_pages.cend(), qsizetype{0}, [](qsizetype total, const auto &page) {
        return total + page->image.sizeInBytes();
    });
}
}
