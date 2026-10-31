// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "FontMetrics.h"

#include <QFontMetricsF>
#include <cmath>

namespace GhosttyQt::Quick
{
FontMetrics fontMetrics(const QFont &font, qreal devicePixelRatio, qreal logicalDpi)
{
    const qreal dpr{qMax(qreal(0.25), devicePixelRatio)};
    FontMetrics result;
    result.pixelFont = font;
    const qreal pixels{font.pixelSize() > 0 ? font.pixelSize() * dpr : qMax(qreal(1), font.pointSizeF()) * logicalDpi * dpr / 72};
    result.pixelFont.setPixelSize(qMax(1, qRound(pixels)));
    const QFontMetricsF fm{result.pixelFont};
    // Include leading and center the font in a whole physical-pixel row.
    const qreal lineHeight{fm.lineSpacing()};
    const int cellHeight{qMax(1, qRound(lineHeight))};
    const qreal baselineFromBottom{fm.descent() + fm.leading() / 2 - (cellHeight - lineHeight) / 2};
    result.cellSize = QSizeF(qMax(1, qRound(fm.horizontalAdvance(QLatin1Char('M')))), cellHeight) / dpr;
    // Nerd Font single-cell icons balance cap height with the full font height.
    result.iconHeight = qMin(qreal(cellHeight), (2 * fm.capHeight() + lineHeight) / 3) / dpr;
    result.baseline = (cellHeight - qRound(baselineFromBottom)) / dpr;
    result.strokeWidth = qMax(qreal(1), std::round(fm.lineWidth())) / dpr;
    result.underlinePosition = qMin(result.cellSize.height() - result.strokeWidth, std::round(result.baseline * dpr + fm.underlinePos()) / dpr);
    result.strikePosition = std::round(result.baseline * dpr - fm.strikeOutPos()) / dpr;

    return result;
}
}
