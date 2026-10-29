// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include <QtTypes>

namespace Kwi::ProcessQueryLimits
{
inline constexpr qsizetype MaximumCommandLineBytes{32768};
inline constexpr qsizetype MaximumOutputBytes{128 * 1024};
inline constexpr int TimeoutMs{5000};
}
