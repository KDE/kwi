// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include <QByteArray>
#include <expected>
#include <memory>

namespace GhosttyQt
{
class Terminal;

class HistoryReader final
{
public:
    enum class Error {
        TerminalUnavailable,
        HistoryChanged,
        FormattingFailed,
    };

    struct Chunk {
        QByteArray bytes;
        bool finished{false};
    };

    ~HistoryReader();

    [[nodiscard]] std::expected<Chunk, Error> read();

private:
    friend class Terminal;
    HistoryReader();

    class Private;
    std::unique_ptr<Private> d;
};
}
