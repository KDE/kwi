// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include <QString>
#include <QStringList>
#include <QtTypes>

namespace Kwi
{
struct SshProcessInfo {
    qint64 pid{-1};
    quint64 startTime{0};
    QString executable;
    QString directory;
    QStringList arguments;

    bool operator==(const SshProcessInfo &) const = default;
};

struct ProcessInfo {
    QString directory;
    qint64 foregroundPid{-1};
    QString foregroundName;
    SshProcessInfo ssh;
    bool busy{false};

    bool operator==(const ProcessInfo &) const = default;
};
}
