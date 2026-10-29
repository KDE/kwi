// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include "Search.h"

#include <QPointer>
#include <ghostty/vt/types.h>

namespace GhosttyQt
{
class Search::Private
{
public:
    GhosttySearch handle{nullptr};
    QPointer<Terminal> terminal;
};
}
