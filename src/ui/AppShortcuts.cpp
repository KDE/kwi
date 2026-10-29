// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "AppShortcuts.h"

#include <ActionCollection/ActionCollection>
#include <KLocalizedQmlContext>
#include <QQmlApplicationEngine>

namespace Kwi
{
AppShortcuts::AppShortcuts(QObject *parent)
    : QObject{parent}
    , m_engine{std::make_unique<QQmlApplicationEngine>()}
{
    KLocalization::setupLocalizedContext(m_engine.get());
    m_engine->loadFromModule(QStringLiteral("org.kde.kwi"), QStringLiteral("ActionDefinitions"));

    if (m_engine->rootObjects().isEmpty()) {
        qFatal("Could not initialize application shortcut definitions.");
    }
}

AppShortcuts::~AppShortcuts() = default;

QString AppShortcuts::collectionFor(const QString &id) const
{
    const auto collections = m_engine->rootObjects().constFirst()->findChildren<KirigamiActions::ActionCollection *>();

    for (auto *collection : collections) {
        if (collection->action(id)) {
            return collection->name();
        }
    }

    return {};
}
}
