// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include "ProcessLaunch.h"
#include <QString>
#include <QStringList>
#include <optional>

namespace Kwi
{
struct LaunchSettings {
    enum class DirectoryMode {
        Current,
        Home,
        Custom
    };

    QStringList command{};
    DirectoryMode directoryMode{DirectoryMode::Current};
    QString directory{};
    QStringList environment{};

    bool operator==(const LaunchSettings &) const = default;
};

struct LaunchRequest {
    std::optional<QStringList> command{};
    QString directory{};
    QString contextDirectory{};
};

[[nodiscard]] bool validateLaunchSettings(const LaunchSettings &settings, QString *error = nullptr);
bool validateEnvironment(const QStringList &entries, QString *error = nullptr);
[[nodiscard]] TerminalLaunch resolveLaunch(const LaunchSettings &settings, const LaunchRequest &request = {});
}
