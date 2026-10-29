// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "Search.h"
#include "Search_p.h"
#include "Terminal.h"

#include <QByteArray>
#include <ghostty/vt/search.h>

#include <iterator>

namespace GhosttyQt
{
namespace
{
Search::Status status(GhosttySearchStatus value)
{
    switch (value) {
    case GHOSTTY_SEARCH_STATUS_RUNNING:
        return Search::Status::Running;
    case GHOSTTY_SEARCH_STATUS_FEED_REQUIRED:
        return Search::Status::FeedRequired;
    case GHOSTTY_SEARCH_STATUS_COMPLETE:
        return Search::Status::Complete;
    default:
        return Search::Status::Error;
    }
}
}

Search::Search()
    : d{std::make_unique<Private>()}
{
}

Search::~Search()
{
    ghostty_search_free(d->handle);
}

bool Search::isValid() const
{
    return d->handle;
}

bool Search::setNeedle(const QString &needle)
{
    const QByteArray bytes{needle.toUtf8()};
    const GhosttyString value{reinterpret_cast<const uint8_t *>(bytes.constData()), size_t(bytes.size())};

    return ghostty_search_set(d->handle, GHOSTTY_SEARCH_OPT_NEEDLE, &value) == GHOSTTY_SUCCESS;
}

Search::Status Search::feed()
{
    if (ghostty_search_feed(d->handle) != GHOSTTY_SUCCESS) {
        return Status::Error;
    }

    return results().status;
}

Search::Status Search::tick()
{
    GhosttySearchStatus value;

    if (ghostty_search_tick(d->handle, &value) != GHOSTTY_SUCCESS) {
        return Status::Error;
    }

    return status(value);
}

Search::Results Search::results() const
{
    Results result;
    GhosttySearchStatus currentStatus;
    size_t count{0};
    const GhosttySearchData keys[] = {GHOSTTY_SEARCH_DATA_STATUS, GHOSTTY_SEARCH_DATA_TOTAL_MATCHES};
    void *values[] = {&currentStatus, &count};

    if (ghostty_search_get_multi(d->handle, std::size(keys), keys, values, nullptr) != GHOSTTY_SUCCESS) {
        return result;
    }

    result.status = status(currentStatus);
    result.totalMatches = qsizetype(count);
    size_t index{0};
    const auto selected = ghostty_search_get(d->handle, GHOSTTY_SEARCH_DATA_SELECTED_INDEX, &index);

    if (selected == GHOSTTY_SUCCESS) {
        result.selectedIndex = qsizetype(index);
    } else if (selected != GHOSTTY_NO_VALUE) {
        result.status = Status::Error;
    }

    return result;
}

bool Search::select(bool forward)
{
    return d->terminal && d->terminal->selectSearchMatch(*this, forward);
}

std::optional<QRegion> Search::viewportMatches() const
{
    return d->terminal ? d->terminal->searchViewportMatches(*this) : std::nullopt;
}

bool Search::applySelection()
{
    return d->terminal && d->terminal->applySearchSelection(*this);
}
}
