// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include <QRegion>
#include <QString>
#include <memory>
#include <optional>

namespace GhosttyQt
{
class Terminal;

class Search final
{
public:
    enum class Status {
        Running,
        FeedRequired,
        Complete,
        Error,
    };

    struct Results {
        Status status{Status::Error};
        qsizetype totalMatches{0};
        // Ghostty orders matches newest first. No index means no selection.
        std::optional<qsizetype> selectedIndex;
    };

    ~Search();
    Search(const Search &) = delete;
    Search &operator=(const Search &) = delete;

    [[nodiscard]] bool isValid() const;
    [[nodiscard]] bool setNeedle(const QString &needle);
    [[nodiscard]] Status feed();
    [[nodiscard]] Status tick();
    [[nodiscard]] Results results() const;

    // Copy the last feed's viewport matches into clipped cell coordinates.
    // Read after feed(), before the terminal changes.
    [[nodiscard]] std::optional<QRegion> viewportMatches() const;

    // Follow Ghostty's search order: forward moves toward older content,
    // starting with the newest match; backward moves toward newer content.
    // Scrolls if the selected match is outside the viewport.
    [[nodiscard]] bool select(bool forward);

    // Set the terminal selection to the current match, or clear it if absent.
    // Call after feed()/select(), before the terminal changes. Does not scroll.
    [[nodiscard]] bool applySelection();

private:
    friend class Terminal;
    Search();

    class Private;
    std::unique_ptr<Private> d;
};
}
