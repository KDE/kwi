// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QList>
#include <QPointer>
#include <functional>

class QObject;

namespace Kwi
{
class TerminalSession;

void checkRunningCommands(const QList<QPointer<TerminalSession>> &sessions, QObject *context, std::function<void(bool)> completed);
}
