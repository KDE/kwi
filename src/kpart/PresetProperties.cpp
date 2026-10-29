// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "PresetProperties.h"

#include "TerminalSession.h"
#include "settings/LaunchSettings.h"
#include "settings/TerminalSettings.h"
#include <QDir>
#include <QFont>
#include <QStringList>
#include <QStringView>
#include <array>
#include <utility>

namespace Kwi::PresetProperties
{
namespace
{
constexpr std::array<std::pair<QStringView, QStringView>, 7> titleTokens{{
    {u"title", u"%w"},
    {u"command", u"%n"},
    {u"directory", u"%d"},
    {u"path", u"%D"},
    {u"user", u"%u"},
    {u"host", u"%H"},
    {u"shortHost", u"%h"},
}};

QString titleToKonsole(const QString &format, bool remote = false)
{
    QString result{};

    for (qsizetype i{0}; i < format.size(); ++i) {
        if (format.mid(i, 2) == u"{{" || format.mid(i, 2) == u"}}") {
            result += format[i++];
        } else if (format[i] == u'{') {
            const auto end = format.indexOf(u'}', i + 1);
            const auto name = format.mid(i + 1, end - i - 1);

            for (const auto &[token, replacement] : titleTokens) {
                if (name == token) {
                    result += remote && token == u"command" ? u"%c" : replacement;
                    break;
                }
            }

            i = end;
        } else if (format[i] == u'%') {
            result += QStringLiteral("%%");
        } else {
            result += format[i];
        }
    }

    return result;
}
}

QVariant property(const TerminalSession *session, const QString &name)
{
    if (!session) {
        return {};
    }

    const auto *settings = session->settings();

    if (name == u"Name" || name == u"UntranslatedName") {
        return session->currentPresetName();
    }

    if (name == u"Path") {
        return session->presetPath();
    }

    if (name == u"Icon") {
        return QStringLiteral("utilities-terminal");
    }

    const auto launch = session->launchSettings();

    if (name == u"Command") {
        return launch.command.isEmpty() ? systemShell() : launch.command.first();
    }

    if (name == u"Arguments") {
        return launch.command.isEmpty() ? QStringList{systemShell()} : launch.command;
    }

    if (name == u"Environment") {
        return launch.environment;
    }

    if (name == u"Directory") {
        return launch.directoryMode == LaunchSettings::DirectoryMode::Home  ? QDir::homePath()
            : launch.directoryMode == LaunchSettings::DirectoryMode::Custom ? launch.directory
                                                                            : QString{};
    }

    if (name == u"StartInCurrentSessionDir") {
        return launch.directoryMode == LaunchSettings::DirectoryMode::Current;
    }

    if (name == u"Font") {
        QFont font{settings->fontFamily()};
        font.setPointSizeF(settings->fontPointSize());

        return font;
    }

    if (name == u"ColorScheme") {
        const auto scheme = settings->colorScheme();

        if (scheme == u"dark") {
            return QStringLiteral("Breeze");
        }

        if (scheme == u"light") {
            return QStringLiteral("BlackOnWhite");
        }

        if (scheme.startsWith(u"konsole:") && scheme.endsWith(u".colorscheme")) {
            return scheme.mid(8, scheme.size() - 20);
        }

        return scheme;
    }

    if (name == u"BackgroundOpacity") {
        return settings->backgroundOpacity();
    }

    if (name == u"BackgroundBlur") {
        return settings->backgroundBlur();
    }

    if (name == u"HistorySize") {
        return settings->scrollbackLines();
    }

    if (name == u"HistoryMode") {
        return settings->scrollbackLines() == -1 ? 2 : (settings->scrollbackLines() == 0 ? 0 : 1);
    }

    if (name == u"SilenceSeconds") {
        return settings->silenceSeconds();
    }

    if (name == u"LocalTabTitleFormat") {
        return titleToKonsole(settings->titleFormat());
    }

    if (name == u"RemoteTabTitleFormat") {
        return titleToKonsole(settings->remoteTitleFormat(), true);
    }

    if (name == u"CursorShape") {
        return settings->cursorStyle() == u"beam" ? 1 : settings->cursorStyle() == u"underline" ? 2 : 0;
    }

    if (name == u"BlinkingCursorEnabled") {
        return settings->cursorBlinking();
    }

    if (name == u"AutoCopySelectedText") {
        return settings->copyOnSelect();
    }

    // Report Kwi's fixed behavior for these KonsolePart properties.
    if (name == u"BellMode") {
        return 1;
    }

    if (name == u"ScrollBarPosition") {
        return 1;
    }

    if (name == u"DefaultEncoding") {
        return QStringLiteral("UTF-8");
    }

    if (name == u"ReflowLines" || name == u"AntiAliasFonts" || name == u"KittyKeyboardEnabled") {
        return true;
    }

    if (name == u"ShowTerminalSizeHint" || name == u"BadgeEnabled" || name == u"InheritContainerContext" || name == u"VerticalLine") {
        return false;
    }

    return {};
}

}
