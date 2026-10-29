// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include "kwiterminal_export.h"
#include <QByteArray>
#include <QDBusMessage>
#include <QDBusUnixFileDescriptor>
#include <QList>
#include <QMap>
#include <QProcessEnvironment>
#include <QString>
#include <QStringList>
#include <optional>

namespace Kwi::HostProcessProtocol
{
using FileDescriptors = QMap<uint, QDBusUnixFileDescriptor>;

inline constexpr auto Service = "org.freedesktop.Flatpak";
inline constexpr auto Object = "/org/freedesktop/Flatpak/Development";
inline constexpr auto Interface = "org.freedesktop.Flatpak.Development";
inline constexpr int CallTimeoutMs = 5000;

enum class CommandFlags : uint {
    None = 0,
    WatchBus = 2
};

struct ProcessIdentity {
    uint pid{0};
    quint64 startTime{0};

    bool operator==(const ProcessIdentity &) const = default;
};

struct ResolvedCommand {
    QString program;
    QString executable;
    bool shell{false};
};

struct Observation {
    ProcessIdentity leader;
    uint foreground{0};
    QString directory;
    QString foregroundExecutable;
    ProcessIdentity ssh;
    // Complete foreground argv, including argv[0]. Empty if not requested or incomplete.
    QStringList arguments;
    QList<ProcessIdentity> members;
};

[[nodiscard]] KWI_TERMINAL_EXPORT QString resolveCommandScript();
[[nodiscard]] KWI_TERMINAL_EXPORT QString inspectProcessScript();
[[nodiscard]] KWI_TERMINAL_EXPORT QString cleanupProcessScript();

/** Add a timeout enforced by the host. */
[[nodiscard]] KWI_TERMINAL_EXPORT QStringList boundedQueryCommand(const QStringList &arguments);

[[nodiscard]] KWI_TERMINAL_EXPORT std::optional<ResolvedCommand> decodeCommand(const QByteArray &reply);
[[nodiscard]] KWI_TERMINAL_EXPORT std::optional<Observation> decodeObservation(const QByteArray &reply, uint pid, quint64 expectedStart = 0);
[[nodiscard]] KWI_TERMINAL_EXPORT QString encodeMembers(const QList<ProcessIdentity> &members);

[[nodiscard]] KWI_TERMINAL_EXPORT QDBusMessage command(const QString &directory,
                                                       const QStringList &arguments,
                                                       const QProcessEnvironment &environment = {},
                                                       int slaveFd = -1,
                                                       CommandFlags flags = CommandFlags::None);
[[nodiscard]] KWI_TERMINAL_EXPORT QDBusMessage
command(const QString &directory, const QStringList &arguments, const QProcessEnvironment &environment, const FileDescriptors &descriptors, CommandFlags flags);
[[nodiscard]] KWI_TERMINAL_EXPORT QDBusMessage signal(uint pid, int signal, bool processGroup = false);
}
