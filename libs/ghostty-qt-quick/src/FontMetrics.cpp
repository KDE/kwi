// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "FontMetrics.h"

#include <QFontMetricsF>
#include <cmath>

namespace GhosttyQt::Quick
{
FontMetrics fontMetrics(const QFont &font, qreal devicePixelRatio)
{
    const qreal dpr{qMax(qreal(0.25), devicePixelRatio)};
    const QFontMetricsF logicalMetrics{font};
    const qreal pixels{font.pixelSize() > 0 ? font.pixelSize() : qMax(qreal(1), font.pointSizeF()) * logicalMetrics.fontDpi() / 72};
    FontMetrics result;
    // Keep the grid independent of screen scale. Rasterize text at the target resolution.
    result.cellSize = QSize(qMax(1, qRound(logicalMetrics.horizontalAdvance(QLatin1Char('M')))), qMax(1, qRound(logicalMetrics.lineSpacing())));
    result.pixelFont = font;
    result.pixelFont.setPixelSize(qMax(1, qRound(pixels * dpr)));
    const QFontMetricsF fm{result.pixelFont};
    const qreal lineHeight{fm.lineSpacing()};
    const qreal cellHeight{result.cellSize.height() * dpr};
    // Nerd Font single-cell icons balance cap height with the full font height.
    result.iconHeight = qMin(cellHeight, (2 * fm.capHeight() + lineHeight) / 3) / dpr;
    // Center the font in the row. Leave pixel alignment to glyph placement.
    result.baseline = (fm.ascent() + (cellHeight - fm.height()) / 2) / dpr;
    result.strokeWidth = qMax(qreal(1), std::round(fm.lineWidth())) / dpr;
    result.underlinePosition = qMin(std::floor(cellHeight) / dpr - result.strokeWidth, std::round(result.baseline * dpr + fm.underlinePos()) / dpr);
    result.strikePosition = std::round(result.baseline * dpr - fm.strikeOutPos()) / dpr;

    return result;
}
}
