// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include "kwiterminal_export.h"
#include <QObject>

namespace Kwi
{
class KWI_TERMINAL_EXPORT SessionEvents final : public QObject // clazy:exclude=ctor-missing-parent-argument
{
    Q_OBJECT

Q_SIGNALS:
    void frameChanged();
    void historyChanged();
    void zoomChanged();
};
}
