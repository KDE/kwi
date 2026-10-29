// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "HostProcessProtocol_p.h"
#include "ProcessQueryLimits_p.h"

#include <QByteArrayList>
#include <QDBusMetaType>
#include <QDBusUnixFileDescriptor>
#include <QFile>
#include <QResource>
#include <QVariant>
#include <algorithm>
#include <limits>

namespace Kwi::HostProcessProtocol
{
namespace
{
using Environment = QMap<QString, QString>;

QString script(const QString &path)
{
    const QResource resource{path};
    Q_ASSERT(resource.isValid());

    return QString::fromUtf8(resource.uncompressedData());
}

std::optional<quint64> positiveNumber(const QByteArray &text)
{
    if (text.isEmpty() || std::ranges::any_of(text, [](char character) {
            return character < '0' || character > '9';
        })) {
        return std::nullopt;
    }

    bool valid{false};
    const auto value = text.toULongLong(&valid);

    return valid && value > 0 ? std::optional(value) : std::nullopt;
}

QDBusMessage method(const QString &name)
{
    return QDBusMessage::createMethodCall(QString::fromLatin1(Service), QString::fromLatin1(Object), QString::fromLatin1(Interface), name);
}
}

QString resolveCommandScript()
{
    return script(QStringLiteral(":/kwi/process/resolve-command.sh"));
}

QString inspectProcessScript()
{
    return script(QStringLiteral(":/kwi/process/inspect-process.sh"));
}

QString cleanupProcessScript()
{
    return script(QStringLiteral(":/kwi/process/cleanup-process.sh"));
}

QStringList boundedQueryCommand(const QStringList &arguments)
{
    // Leave time for RPC delivery/cleanup before the local five-second deadline.
    return QStringList{QStringLiteral("/usr/bin/timeout"), QStringLiteral("--signal=KILL"), QStringLiteral("3")} + arguments;
}

std::optional<ResolvedCommand> decodeCommand(const QByteArray &reply)
{
    if (reply.size() > ProcessQueryLimits::MaximumOutputBytes) {
        return std::nullopt;
    }

    const auto fields = reply.split('\0');

    if (fields.size() != 4 || !fields.last().isEmpty() || fields[0].isEmpty() || fields[1].isEmpty() || (fields[2] != "0" && fields[2] != "1")) {
        return std::nullopt;
    }

    return ResolvedCommand{QString::fromLocal8Bit(fields[0]), QString::fromLocal8Bit(fields[1]), fields[2] == "1"};
}

std::optional<Observation> decodeObservation(const QByteArray &reply, uint pid, quint64 expectedStart)
{
    if (!pid || pid > uint(std::numeric_limits<int>::max()) || reply.size() > ProcessQueryLimits::MaximumOutputBytes) {
        return std::nullopt;
    }

    const auto fields = reply.split('\0');

    if (fields.size() < 8) {
        return std::nullopt;
    }

    const auto start = positiveNumber(fields[0]);
    const auto foreground = positiveNumber(fields[1]);

    if (!start || (expectedStart && *start != expectedStart) || !foreground || *foreground > std::numeric_limits<int>::max()) {
        return std::nullopt;
    }

    Observation observation;
    observation.leader = {pid, *start};
    observation.foreground = uint(*foreground);
    observation.directory = QString::fromLocal8Bit(fields[2]);
    observation.foregroundExecutable = QString::fromLocal8Bit(fields[3]);

    if (!fields[5].isEmpty()) {
        const auto sshPid = positiveNumber(fields[5]);
        const auto sshStart = positiveNumber(fields[6]);

        if (!sshPid || *sshPid > std::numeric_limits<int>::max() || !sshStart) {
            return std::nullopt;
        }

        observation.ssh = {uint(*sshPid), *sshStart};
    }

    qsizetype commandOffset{0};

    for (qsizetype index{0}; index < 7; ++index) {
        commandOffset += fields[index].size() + 1;
    }

    if (reply.size() - commandOffset <= ProcessQueryLimits::MaximumCommandLineBytes && reply.endsWith('\0') && !fields[7].isEmpty()) {
        for (qsizetype index{7}; index + 1 < fields.size(); ++index) {
            observation.arguments.append(QString::fromLocal8Bit(fields[index]));
        }
    }

    const auto members = fields[4].split(' ');

    for (const auto &entry : members) {
        if (entry.isEmpty()) {
            continue;
        }

        const auto identity = entry.split(':');

        if (identity.size() != 2) {
            return std::nullopt;
        }

        const auto member = positiveNumber(identity[0]);
        const auto memberStart = positiveNumber(identity[1]);

        if (!member || *member > std::numeric_limits<int>::max() || !memberStart) {
            return std::nullopt;
        }

        observation.members.append({uint(*member), *memberStart});
    }

    return observation;
}

QString encodeMembers(const QList<ProcessIdentity> &members)
{
    QStringList encoded;
    encoded.reserve(members.size());

    for (const auto &member : members) {
        // Append formatted entries directly without transform's lambda and output iterator.
        // cppcheck-suppress useStlAlgorithm
        encoded.append(QString::number(member.pid) + QLatin1Char(':') + QString::number(member.startTime));
    }

    return encoded.join(QLatin1Char(' '));
}

QDBusMessage command(const QString &directory, const QStringList &arguments, const QProcessEnvironment &environment, int slaveFd, CommandFlags flags)
{
    FileDescriptors descriptors;

    if (slaveFd >= 0) {
        for (uint fd = 0; fd < 3; ++fd) {
            descriptors.insert(fd, QDBusUnixFileDescriptor(slaveFd));
        }
    }

    return command(directory, arguments, environment, descriptors, flags);
}

QDBusMessage
command(const QString &directory, const QStringList &arguments, const QProcessEnvironment &environment, const FileDescriptors &descriptors, CommandFlags flags)
{
    qDBusRegisterMetaType<FileDescriptors>();
    qDBusRegisterMetaType<Environment>();
    qDBusRegisterMetaType<QByteArrayList>();
    QByteArrayList encoded;
    encoded.reserve(arguments.size());

    for (const auto &argument : arguments) {
        encoded.append(QFile::encodeName(argument) + '\0');
    }

    Environment variables;

    const auto environmentNames = environment.keys();

    for (const auto &name : environmentNames) {
        variables.insert(name, environment.value(name));
    }

    auto message = method(QStringLiteral("HostCommand"));
    const QByteArray encodedDirectory{QFile::encodeName(directory) + '\0'};
    message.setArguments({encodedDirectory, QVariant::fromValue(encoded), QVariant::fromValue(descriptors), QVariant::fromValue(variables), uint(flags)});

    return message;
}

QDBusMessage signal(uint pid, int signal, bool processGroup)
{
    auto message = method(QStringLiteral("HostCommandSignal"));
    message.setArguments({pid, uint(signal), processGroup});

    return message;
}
}
