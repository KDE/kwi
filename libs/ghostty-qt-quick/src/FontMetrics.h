// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include <QFont>
#include <QSizeF>

namespace GhosttyQt::Quick
{
struct FontMetrics {
    QSizeF cellSize;
    qreal baseline{0};
    qreal underlinePosition{0};
    qreal strokeWidth{0};
    QFont pixelFont;
};

[[nodiscard]] FontMetrics fontMetrics(const QFont &font, qreal devicePixelRatio, qreal logicalDpi = 96);
}
