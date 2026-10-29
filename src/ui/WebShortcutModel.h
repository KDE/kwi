// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QByteArray>
#include <QHash>
#include <QList>
#include <QString>
#include <QUrl>
#include <QVariant>
#include <QtQmlIntegration/qqmlintegration.h>

namespace Kwi
{
class WebShortcutModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(int count READ count NOTIFY refreshed)
    Q_PROPERTY(QString title READ title NOTIFY refreshed)
    Q_PROPERTY(bool canConfigure READ canConfigure NOTIFY refreshed)

public:
    explicit WebShortcutModel(QObject *parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
    [[nodiscard]] int count() const;
    [[nodiscard]] QString title() const;
    [[nodiscard]] bool canConfigure() const;

    Q_INVOKABLE void refresh(const QString &selectedText);
    [[nodiscard]] Q_INVOKABLE QUrl searchUrl(int row) const;
    Q_INVOKABLE void configure();

Q_SIGNALS:
    void refreshed();
    void configurationError(const QString &message);

private:
    struct Provider {
        QString name;
        QString iconName;
        QString query;
    };

    QList<Provider> m_providers;
    QString m_title;
};
}
