// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include <QStringView>
#include <QVector>

namespace GhosttyQt
{
struct Grapheme {
    qsizetype length{0}; // UTF-16 code units.
    quint8 width{0}; // Terminal columns.
};

// Stateless segmentation and widths using the same Unicode rules as the terminal.
[[nodiscard]] QVector<Grapheme> graphemes(QStringView text);
}
