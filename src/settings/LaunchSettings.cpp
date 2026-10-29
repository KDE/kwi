// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "LaunchSettings.h"

#include <KLocalizedString>
#include <QDir>
#include <QStringView>
#include <utility>

namespace Kwi
{
namespace
{
bool fail(QString *error, const QString &message)
{
    if (error) {
        *error = message;
    }

    return false;
}

bool validVariableName(QStringView name)
{
    if (name.isEmpty()) {
        return false;
    }

    const auto letter = [](QChar c) {
        return c == QLatin1Char('_') || (c >= QLatin1Char('A') && c <= QLatin1Char('Z')) || (c >= QLatin1Char('a') && c <= QLatin1Char('z'));
    };

    if (!letter(name.front())) {
        return false;
    }

    for (QChar c : name) {
        if (!letter(c) && !(c >= QLatin1Char('0') && c <= QLatin1Char('9'))) {
            return false;
        }
    }

    return true;
}
}

bool validateEnvironment(const QStringList &entries, QString *error)
{
    qsizetype size{0};

    if (entries.size() > 4096) {
        return fail(error, i18n("Too many environment variables."));
    }

    for (const QString &entry : entries) {
        size += entry.size();
        const qsizetype separator{entry.indexOf(QLatin1Char('='))};

        if (size > 65536 || separator <= 0 || !validVariableName(QStringView(entry).left(separator)) || entry.contains(QChar::Null)
            || entry.contains(QLatin1Char('\n')) || entry.contains(QLatin1Char('\r'))) {
            return fail(error,
                        i18n("Enter one NAME=value assignment per line. Names may contain letters, digits and underscores, and must not start with a digit."));
        }
    }

    if (error) {
        error->clear();
    }

    return true;
}

bool validateLaunchSettings(const LaunchSettings &settings, QString *error)
{
    if (settings.command.size() > 4096 || (!settings.command.isEmpty() && settings.command.first().isEmpty())) {
        return fail(error, i18n("Enter a program followed by its arguments, or leave the command empty to use the system shell."));
    }

    qsizetype size{0};

    for (const QString &argument : settings.command) {
        size += argument.size();

        if (size > 65536 || argument.contains(QChar::Null)) {
            return fail(error, i18n("The command is too long or contains a null character."));
        }
    }

    switch (settings.directoryMode) {
    case LaunchSettings::DirectoryMode::Current:
    case LaunchSettings::DirectoryMode::Home:
        if (!settings.directory.isEmpty()) {
            return fail(error, i18n("Only a custom working directory can contain a path."));
        }

        break;
    case LaunchSettings::DirectoryMode::Custom:
        if (settings.directory.size() > 65536 || settings.directory.contains(QChar::Null) || !QDir::isAbsolutePath(settings.directory)) {
            return fail(error, i18n("Choose an absolute path for the working directory."));
        }

        break;
    default:
        return fail(error, i18n("Choose how the working directory is determined."));
    }

    return validateEnvironment(settings.environment, error);
}

TerminalLaunch resolveLaunch(const LaunchSettings &settings, const LaunchRequest &request)
{
    QStringList command{request.command.value_or(settings.command)};

    TerminalLaunch launch{};

    if (!command.isEmpty()) {
        launch.program = command.takeFirst();
    }

    launch.arguments = std::move(command);
    launch.directory = request.directory;

    if (launch.directory.isEmpty()) {
        switch (settings.directoryMode) {
        case LaunchSettings::DirectoryMode::Current:
            launch.directory = request.contextDirectory;
            break;
        case LaunchSettings::DirectoryMode::Home:
            launch.directory = QDir::homePath();
            break;
        case LaunchSettings::DirectoryMode::Custom:
            launch.directory = settings.directory;
            break;
        }
    }

    if (launch.directory.isEmpty()) {
        launch.directory = QDir::homePath();
    }

    launch.directory = QDir::cleanPath(QDir::current().absoluteFilePath(launch.directory));

    for (const QString &entry : settings.environment) {
        const qsizetype separator{entry.indexOf(QLatin1Char('='))};
        launch.environmentChanges.insert(entry.left(separator), entry.mid(separator + 1));
    }

    return launch;
}

}
