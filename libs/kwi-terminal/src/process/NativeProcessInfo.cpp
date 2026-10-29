// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "NativeProcessInfo_p.h"
#include "ProcessQueryLimits_p.h"

#ifdef Q_OS_FREEBSD
#include <cstddef>
#include <limits.h>
#include <span>
#include <sys/sysctl.h>
#include <sys/user.h>
#include <vector>
#else
#include <QDirListing>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <unistd.h>
#endif

namespace Kwi::NativeProcessInfo
{
#ifdef Q_OS_FREEBSD
namespace
{
bool query(pid_t pid, int field, void *data, size_t &size)
{
    const int mib[]{CTL_KERN, KERN_PROC, field, pid};

    return sysctl(mib, 4, data, &size, nullptr, 0) == 0;
}
}

QString directory(pid_t pid)
{
    // KERN_PROC_CWD returns a packed kinfo_file.
    kinfo_file file{};
    size_t size{sizeof(file)};

    if (!query(pid, KERN_PROC_CWD, &file, size) || size <= offsetof(kinfo_file, kf_path)) {
        return {};
    }

    return QString::fromLocal8Bit(file.kf_path);
}

QString executable(pid_t pid)
{
    char path[PATH_MAX]{};
    size_t size{sizeof(path)};

    return query(pid, KERN_PROC_PATHNAME, path, size) ? QString::fromLocal8Bit(path) : QString{};
}

QByteArray commandLine(pid_t pid)
{
    QByteArray bytes(ProcessQueryLimits::MaximumCommandLineBytes + 1, Qt::Uninitialized);
    size_t size{size_t(bytes.size())};

    if (!query(pid, KERN_PROC_ARGS, bytes.data(), size)) {
        return {};
    }

    bytes.truncate(qsizetype(size));

    return bytes;
}

quint64 startTime(pid_t pid)
{
    kinfo_proc process{};
    size_t size{sizeof(process)};

    if (!query(pid, KERN_PROC_PID, &process, size) || size != sizeof(process)) {
        return 0;
    }

    return quint64(process.ki_start.tv_sec) * 1000000 + quint64(process.ki_start.tv_usec);
}

pid_t groupMember(pid_t group, pid_t session)
{
    size_t size{0};

    if (!query(group, KERN_PROC_PGRP, nullptr, size) || size == 0) {
        return -1;
    }

    std::vector<kinfo_proc> processes(size / sizeof(kinfo_proc));
    size = processes.size() * sizeof(kinfo_proc);

    if (!query(group, KERN_PROC_PGRP, processes.data(), size)) {
        return -1;
    }

    for (const auto &process : std::span{processes.data(), size / sizeof(kinfo_proc)}) {
        if (process.ki_pgid == group && process.ki_sid == session) {
            return process.ki_pid;
        }
    }

    return -1;
}
#else
QString directory(pid_t pid)
{
    return QFileInfo(QStringLiteral("/proc/%1/cwd").arg(pid)).symLinkTarget();
}

QString executable(pid_t pid)
{
    return QFileInfo(QStringLiteral("/proc/%1/exe").arg(pid)).symLinkTarget();
}

QByteArray commandLine(pid_t pid)
{
    QFile file{QStringLiteral("/proc/%1/cmdline").arg(pid)};

    return file.open(QIODevice::ReadOnly) ? file.read(ProcessQueryLimits::MaximumCommandLineBytes + 1) : QByteArray{};
}

quint64 startTime(pid_t pid)
{
    QFile file{QStringLiteral("/proc/%1/stat").arg(pid)};
    const QByteArray bytes{file.open(QIODevice::ReadOnly) ? file.read(4096) : QByteArray{}};
    const auto fields = bytes.mid(bytes.lastIndexOf(')') + 2).split(' ');

    return fields.size() > 19 ? fields[19].toULongLong() : 0;
}

pid_t groupMember(pid_t group, pid_t session)
{
    for (const auto &entry : QDirListing{QStringLiteral("/proc"), QDirListing::IteratorFlag::DirsOnly}) {
        bool valid{false};
        const int pid{entry.fileName().toInt(&valid)};

        if (valid && getpgid(pid) == group && getsid(pid) == session) {
            return pid;
        }
    }

    return -1;
}
#endif
}
