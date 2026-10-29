// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QString>
#include <QtQmlIntegration/qqmlintegration.h>
#include <memory>

class QQmlApplicationEngine;

namespace Kwi
{
class AppShortcuts : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Shortcut definitions are shared by the application.")

public:
    explicit AppShortcuts(QObject *parent = nullptr);
    ~AppShortcuts() override;

    [[nodiscard]] Q_INVOKABLE QString collectionFor(const QString &id) const;

private:
    std::unique_ptr<QQmlApplicationEngine> m_engine;
};
}
