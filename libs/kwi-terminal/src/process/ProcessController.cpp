// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "ProcessController_p.h"

#include <KLocalizedString>
#include <KPtyDevice>
#include <QStringView>
#include <algorithm>
#include <cerrno>
#include <cstring>

namespace Kwi
{
bool ProcessController::prepareMaster(int masterFd)
{
    m_master = {};

    if (masterFd >= 0) {
        m_master = QDBusUnixFileDescriptor(masterFd);

        if (!m_master.isValid()) {
            Q_EMIT errorOccurred(i18n("Could not attach to the terminal: %1", QString::fromLocal8Bit(strerror(errno))));

            return false;
        }
    }

    return true;
}

void ProcessController::closeTransport()
{
    if (device()) {
        device()->close();
    }

    m_master = {};

    Q_EMIT transportClosed();
}

bool interactiveShellArguments(const QStringList &arguments)
{
    return std::ranges::all_of(arguments, [](const QString &argument) {
        if (argument == QLatin1String("--login") || argument == QLatin1String("--interactive") || argument == QLatin1String("--noprofile")
            || argument == QLatin1String("--norc") || argument == QLatin1String("--no-config") || argument == QLatin1String("--no-rcs")) {
            return true;
        }

        // Commands and script filenames are active work even in the shell's group.
        if (!argument.startsWith(QLatin1Char('-')) || argument.size() < 2) {
            return false;
        }

        return std::ranges::all_of(QStringView(argument).mid(1), [](QChar flag) {
            return flag == QLatin1Char('i') || flag == QLatin1Char('l');
        });
    });
}
}
