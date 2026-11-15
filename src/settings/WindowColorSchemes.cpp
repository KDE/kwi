// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "WindowColorSchemes.h"

#include <KColorSchemeManager>
#include <QModelIndex>

namespace Kwi
{
QAbstractItemModel *WindowColorSchemes::model() const
{
    return KColorSchemeManager::instance()->model();
}

int WindowColorSchemes::currentIndex() const
{
    const auto *manager = KColorSchemeManager::instance();

    return manager->indexForSchemeId(manager->activeSchemeId()).row();
}

void WindowColorSchemes::setCurrentIndex(int index)
{
    if (index == currentIndex()) {
        return;
    }

    KColorSchemeManager::instance()->activateScheme(model()->index(index, 0));
    Q_EMIT currentIndexChanged();
}
}
