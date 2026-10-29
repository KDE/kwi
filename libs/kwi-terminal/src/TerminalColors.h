// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include "kwiterminal_export.h"
#include <QColor>
#include <QString>
#include <QStringView>
#include <QVector>
#include <optional>

class QPalette;

namespace Kwi
{

struct TerminalColors {
    QColor foreground{};
    QColor boldForeground{}; // Unset uses the default foreground, including OSC 10.
    QColor background{};
    QColor cursor{};
    QColor selectionForeground{};
    QColor selectionBackground{};
    QVector<QRgb> ansi{};
};

struct TerminalPalette {
    QString path{};
    QString description{};
    TerminalColors colors{};
};

[[nodiscard]] KWI_TERMINAL_EXPORT TerminalColors terminalColors(QStringView scheme, const QPalette &applicationPalette);

/** Resolve legacy dark/light settings IDs. */
[[nodiscard]] KWI_TERMINAL_EXPORT QString canonicalTerminalPaletteId(QStringView id);
[[nodiscard]] KWI_TERMINAL_EXPORT QString ownedTerminalPalettePath(QStringView id);
[[nodiscard]] KWI_TERMINAL_EXPORT bool isValidTerminalPalette(QStringView id);
[[nodiscard]] KWI_TERMINAL_EXPORT std::optional<TerminalPalette> loadTerminalPalette(QStringView id, const QPalette &applicationPalette);
[[nodiscard]] KWI_TERMINAL_EXPORT std::optional<TerminalPalette> loadTerminalPaletteFile(const QString &path, const QPalette &applicationPalette);

}
