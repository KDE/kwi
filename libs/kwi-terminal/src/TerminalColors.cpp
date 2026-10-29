// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "TerminalColors.h"
#include <KConfig>
#include <KConfigGroup>
#include <QFileInfo>
#include <QPalette>
#include <QStandardPaths>
#include <QStringList>

namespace Kwi
{
namespace
{

QStringList terminalPalettePaths(QStringView id)
{
    const QString canonicalId{canonicalTerminalPaletteId(id)};
    id = canonicalId;
    const bool imported{id.startsWith(u"imported:")};
    const bool installed{id.startsWith(u"konsole:")};
    const bool user{id.startsWith(u"user:")};

    if (!imported && !installed && !user) {
        return {};
    }

    const QString filename{id.mid(imported ? 9 : user ? 5 : 8).toString()};

    if (filename.isEmpty() || filename.contains(QLatin1Char('/')) || filename.contains(QLatin1Char('\\'))
        || !filename.endsWith(QLatin1String(".colorscheme"))) {
        return {};
    }

    if (imported || user) {
        const QString path{QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/kwi/color-schemes/")
                           + (user ? QStringLiteral("user/") : QString{}) + filename};
        const QFileInfo file{path};

        return file.isFile() ? QStringList{path} : QStringList{};
    }

    auto paths = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("kwi/color-schemes/stock/") + filename);
    paths.append(QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("konsole/") + filename));

    return paths;
}

TerminalColors defaultTerminalColors(QStringView scheme, const QPalette &applicationPalette)
{
    TerminalColors result{};

    if (scheme == u"dark") {
        result.background = QColor(35, 38, 39);
        result.foreground = QColor(252, 252, 252);
    } else if (scheme == u"light") {
        result.background = Qt::white;
        result.foreground = Qt::black;
    } else {
        // The application palette includes Plasma View roles and per-application overrides.
        result.background = applicationPalette.color(QPalette::Active, QPalette::Base);
        result.foreground = applicationPalette.color(QPalette::Active, QPalette::Text);
    }

    result.cursor = result.foreground;
    result.selectionBackground = applicationPalette.color(QPalette::Active, QPalette::Highlight);
    result.selectionForeground = applicationPalette.color(QPalette::Active, QPalette::HighlightedText);

    // Color0..7 and Color0Intense..7Intense from Konsole's Breeze and BlackOnWhite
    // schemes. ANSI colors must not follow desktop accent roles.
    if (result.background.lightnessF() > result.foreground.lightnessF()) {
        result.ansi = {0xff000000,
                       0xffb21818,
                       0xff18b218,
                       0xffb26818,
                       0xff1818b2,
                       0xffb218b2,
                       0xff18b2b2,
                       0xffb2b2b2,
                       0xff686868,
                       0xffff5454,
                       0xff54ff54,
                       0xffffff54,
                       0xff5454ff,
                       0xffff54ff,
                       0xff54ffff,
                       0xffffffff};
    } else {
        result.ansi = {0xff232627,
                       0xffed1515,
                       0xff11d116,
                       0xfff67400,
                       0xff1d99f3,
                       0xff9b59b6,
                       0xff1abc9c,
                       0xfffcfcfc,
                       0xff7f8c8d,
                       0xffc0392b,
                       0xff1cdc9a,
                       0xfffdbc4b,
                       0xff3daee9,
                       0xff8e44ad,
                       0xff16a085,
                       0xffffffff};
    }

    return result;
}
}

TerminalColors terminalColors(QStringView scheme, const QPalette &applicationPalette)
{
    if (const auto palette = loadTerminalPalette(scheme, applicationPalette)) {
        return palette->colors;
    }

    return defaultTerminalColors(u"system", applicationPalette);
}

QString canonicalTerminalPaletteId(QStringView id)
{
    if (id == u"dark") {
        return QStringLiteral("konsole:Breeze.colorscheme");
    }

    if (id == u"light") {
        return QStringLiteral("konsole:BlackOnWhite.colorscheme");
    }

    return id.toString();
}

QString ownedTerminalPalettePath(QStringView id)
{
    return id.startsWith(u"imported:") || id.startsWith(u"user:") ? terminalPalettePaths(id).value(0) : QString{};
}

std::optional<TerminalPalette> loadTerminalPalette(QStringView id, const QPalette &applicationPalette)
{
    if (id == u"system") {
        return TerminalPalette{{}, {}, defaultTerminalColors(u"system", applicationPalette)};
    }

    const auto paths = terminalPalettePaths(id);

    for (const auto &path : paths) {
        // Return the loaded palette; any_of would need separate result storage.
        // cppcheck-suppress useStlAlgorithm
        if (auto palette = loadTerminalPaletteFile(path, applicationPalette)) {
            return palette;
        }
    }

    return std::nullopt;
}

bool isValidTerminalPalette(QStringView id)
{
    return id == u"system" || id == u"dark" || id == u"light" || loadTerminalPalette(id, QPalette{}).has_value();
}

std::optional<TerminalPalette> loadTerminalPaletteFile(const QString &path, const QPalette &applicationPalette)
{
    const QFileInfo file{path};

    if (path.isEmpty() || !file.isFile() || !file.isReadable() || file.size() > 256 * 1024) {
        return std::nullopt;
    }

    const KConfig config{path, KConfig::SimpleConfig};
    const QColor foreground{KConfigGroup(&config, QStringLiteral("Foreground")).readEntry("Color", QColor{})};
    const QColor background{KConfigGroup(&config, QStringLiteral("Background")).readEntry("Color", QColor{})};

    if (!foreground.isValid() || !background.isValid()) {
        return std::nullopt;
    }

    // Konsole's file format uses its fixed default table for omitted normal colors.
    TerminalColors result{defaultTerminalColors(u"light", applicationPalette)};
    result.foreground = foreground;
    result.background = background;
    result.foreground.setAlpha(255);
    result.background.setAlpha(255);
    result.cursor = result.foreground;
    const KConfigGroup intenseForeground{&config, QStringLiteral("ForegroundIntense")};

    if (intenseForeground.hasKey("Color")) {
        result.boldForeground = intenseForeground.readEntry("Color", QColor{});

        if (!result.boldForeground.isValid()) {
            return std::nullopt;
        }

        result.boldForeground.setAlpha(255);
    }

    for (int index{0}; index < 16; ++index) {
        const QString group{QStringLiteral("Color%1%2").arg(index % 8).arg(index >= 8 ? QStringLiteral("Intense") : QString{})};
        const KConfigGroup colorGroup{&config, group};

        if (!colorGroup.hasKey("Color")) {
            // Omitted intense colors inherit their corresponding normal color.
            result.ansi[index] = result.ansi.at(index % 8);

            continue;
        }

        const QColor color{colorGroup.readEntry("Color", QColor{})};

        if (!color.isValid()) {
            return std::nullopt;
        }

        result.ansi[index] = color.rgb();
    }

    return TerminalPalette{path, KConfigGroup(&config, QStringLiteral("General")).readEntry("Description", file.completeBaseName()), result};
}

}
