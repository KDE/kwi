// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "ProcessLaunch.h"

#include <KLocalizedString>
#include <KUser>
#include <QDir>
#include <algorithm>

namespace Kwi
{
QString systemShell(const QProcessEnvironment &environment)
{
    QString shell{environment.value(QStringLiteral("SHELL"))};

    if (shell.isEmpty()) {
        shell = KUser{}.shell();
    }

    return shell.isEmpty() ? QStringLiteral("/bin/sh") : shell;
}

QMap<QString, std::optional<QString>> TerminalLaunch::resolvedEnvironmentChanges() const
{
    QMap<QString, std::optional<QString>> result{{QStringLiteral("TERM"), QStringLiteral("xterm-256color")},
                                                 {QStringLiteral("COLORTERM"), QStringLiteral("truecolor")},
                                                 {QStringLiteral("TERM_PROGRAM"), QStringLiteral("kwi")},
                                                 {QStringLiteral("COLUMNS"), std::nullopt},
                                                 {QStringLiteral("LINES"), std::nullopt},
                                                 {QStringLiteral("TERMCAP"), std::nullopt}};
    result.insert(environmentChanges);

    return result;
}

QString TerminalLaunch::validationError() const
{
    if ((!directory.isEmpty() && !QDir::isAbsolutePath(directory)) || directory.contains(QChar::Null)) {
        return i18nd("kwi-terminal", "A terminal's initial directory must be an absolute path.");
    }

    if (program.contains(QChar::Null) || std::ranges::any_of(arguments, [](const QString &argument) {
            return argument.contains(QChar::Null);
        })) {
        return i18nd("kwi-terminal", "The command contains a null character.");
    }

    const auto validAssignment = [](const QString &name, const QString &value) {
        return !name.isEmpty() && !name.contains(QChar::Null) && !name.contains(QLatin1Char('=')) && !value.contains(QChar::Null);
    };

    const auto inheritedNames = inheritedEnvironment.keys();

    if (std::ranges::any_of(inheritedNames, [&](const QString &name) {
            return !validAssignment(name, inheritedEnvironment.value(name));
        })) {
        return i18nd("kwi-terminal", "The terminal environment contains an invalid assignment.");
    }

    for (auto it = environmentChanges.cbegin(); it != environmentChanges.cend(); ++it) {
        if (!validAssignment(it.key(), it.value().value_or(QString{}))) {
            return i18nd("kwi-terminal", "The terminal environment contains an invalid assignment.");
        }
    }

    return {};
}

QProcessEnvironment TerminalLaunch::environment() const
{
    auto result = inheritedEnvironment;
    const auto changes{resolvedEnvironmentChanges()};

    for (auto it = changes.cbegin(); it != changes.cend(); ++it) {
        if (it.value()) {
            result.insert(it.key(), *it.value());
        } else {
            result.remove(it.key());
        }
    }

    return result;
}
}
