// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include <QImage>
#include <QSize>

namespace GhosttyQt::Quick
{
[[nodiscard]] bool isProceduralSymbol(char32_t codepoint);
[[nodiscard]] QImage symbolImage(char32_t codepoint, QSize cellSize, int strokeWidth);
}
