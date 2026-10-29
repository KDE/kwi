// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <KConfigGroup>
#include <KSharedConfig>
#include <QString>

namespace Kwi
{
inline bool webShortcutsEnabled()
{
    // Provider-only lookups succeed even when web shortcuts are disabled.
    const auto config = KSharedConfig::openConfig(QStringLiteral("kuriikwsfilterrc"));

    return config->group(QStringLiteral("General")).readEntry("EnableWebShortcuts", true);
}
}
