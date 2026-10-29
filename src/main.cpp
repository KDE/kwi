// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "DesktopIntegration.h"
#include "WindowManager.h"
#include "kwi-config.h"
#include "kwi-version.h"
#include "settings/TerminalPaletteModel.h"
#include "settings/TerminalPresets.h"
#include "settings/TerminalSettings.h"
#include "ui/AppShortcuts.h"

#include <KAboutData>
#include <KDBusService>
#include <KLocalizedString>
#include <KirigamiAppDefaults>
#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include <QQuickWindow>
#include <QString>
#include <QStringList>
#include <utility>

namespace
{
[[nodiscard]] QString commandLineError(const QCommandLineParser &parser)
{
    const auto command = parser.positionalArguments();

    if (parser.isSet(QStringLiteral("execute")) && (command.isEmpty() || command.constFirst().isEmpty())) {
        return i18n("-e requires a program.");
    }

    if (parser.isSet(QStringLiteral("preset")) && parser.value(QStringLiteral("preset")).isEmpty()) {
        return i18n("--preset requires a preset name.");
    }

    return {};
}

[[nodiscard]] Kwi::LaunchOptions launchOptions(const QCommandLineParser &parser, const QString &callerDirectory)
{
    Kwi::LaunchOptions options{};

    auto command = parser.positionalArguments();

    if (!command.isEmpty()) {
        options.launch.command = std::move(command);
    }

    const QString directory{parser.value(QStringLiteral("workdir"))};

    if (!directory.isEmpty()) {
        options.launch.directory = QDir(callerDirectory).absoluteFilePath(directory);
    }

    options.launch.contextDirectory = callerDirectory;
    options.presetName = parser.value(QStringLiteral("preset"));
    options.title = parser.value(QStringLiteral("tab-title"));
    options.hold = parser.isSet(QStringLiteral("hold"));
    options.newTab = parser.isSet(QStringLiteral("new-tab"));
    options.fullscreen = parser.isSet(QStringLiteral("fullscreen"));

    return options;
}
}

int main(int argc, char **argv)
{
    QQuickWindow::setDefaultAlphaBuffer(true);
    QApplication app{argc, argv};
    KirigamiAppDefaults::apply(&app);
    QCoreApplication::setApplicationName(QStringLiteral(KWI_EXECUTABLE));
    QCoreApplication::setOrganizationDomain(QStringLiteral(KWI_ORGANIZATION_DOMAIN));
    QGuiApplication::setDesktopFileName(QStringLiteral(KWI_APP_ID));

    Kwi::DesktopIntegration::initializeApplicationIcon();
    KLocalizedString::setApplicationDomain("kwi");

    KAboutData about{QStringLiteral(KWI_EXECUTABLE),
                     i18n("Kwi"),
                     QStringLiteral(KWI_VERSION_STRING),
                     i18n("A modern and fast terminal"),
                     KAboutLicense::GPL_V2,
                     i18n("© 2026 Kwi contributors")};
    about.addLicense(KAboutLicense::GPL_V3);
    about.addLicenseTextFile(QStringLiteral(":/kwi/licenses/LicenseRef-KDE-Accepted-GPL.txt"));
    about.setTranslator(i18nc("NAME OF TRANSLATORS", "Your names"), i18nc("EMAIL OF TRANSLATORS", "Your emails"));
    about.setBugAddress({});
    KAboutData::setApplicationData(about);

    QCommandLineParser parser{};
    about.setupCommandLine(&parser);
    parser.setApplicationDescription(about.shortDescription());
    parser.setOptionsAfterPositionalArgumentsMode(QCommandLineParser::ParseAsPositionalArguments);
    parser.addOption({QStringLiteral("workdir"), i18n("Start in this directory."), i18nc("@info:shell command-line argument", "directory")});
    parser.addOption({QStringLiteral("preset"), i18n("Start with this terminal preset."), i18nc("@info:shell command-line argument", "name")});
    parser.addOption({{QStringLiteral("hold"), QStringLiteral("noclose")}, i18n("Keep the terminal open after its command exits.")});
    parser.addOption({QStringLiteral("new-tab"), i18n("Open a tab in the most recently active window.")});
    parser.addOption({QStringLiteral("fullscreen"), i18n("Open the window in full screen.")});
    parser.addOption(
        {{QStringLiteral("T"), QStringLiteral("tab-title")}, i18n("Set the initial tab title."), i18nc("@info:shell command-line argument", "title")});
    parser.addOption({{QStringLiteral("e"), QStringLiteral("execute")}, i18n("Run the specified program.")});
    parser.addPositionalArgument(i18nc("@info:shell command-line argument", "program"),
                                 i18n("Optional program and its arguments."),
                                 i18nc("@info:shell command-line syntax", "[program [arguments…]]"));

    // Handle help/version before forwarding to a running instance.
    parser.process(app);
    about.processCommandLine(&parser);
    const QString error{commandLineError(parser)};

    if (!error.isEmpty()) {
        qCritical().noquote() << error;

        return 2;
    }

    const auto initialOptions = launchOptions(parser, QDir::currentPath());
    KDBusService service{KDBusService::Unique | KDBusService::NoExitOnFailure};

    Kwi::TerminalSettings settings{};
    Kwi::TerminalPresets presets{&settings};
    Kwi::AppShortcuts shortcuts{};
    Kwi::TerminalPaletteModel palettes{&settings};
    Kwi::WindowManager windows{&settings, &presets, &shortcuts, &palettes};

    QObject::connect(
        &service,
        &KDBusService::activateRequested,
        &windows,
        [&](const QStringList &arguments, const QString &directory) {
            if (!parser.parse(arguments) || !commandLineError(parser).isEmpty()) {
                service.setExitValue(2);

                return;
            }

            const QString cwd{directory.isEmpty() ? QDir::homePath() : directory};
            service.setExitValue(windows.launch(launchOptions(parser, cwd)) ? 0 : 1);
        },
        Qt::DirectConnection);

    if (!windows.launch(initialOptions)) {
        return 1;
    }

    return app.exec();
}
