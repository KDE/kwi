// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractItemModel>
#include <QObject>
#include <QtQmlIntegration/qqmlintegration.h>

namespace Kwi
{
class WindowColorSchemes final : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QAbstractItemModel *model READ model CONSTANT)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)

public:
    using QObject::QObject;

    [[nodiscard]] QAbstractItemModel *model() const;
    [[nodiscard]] int currentIndex() const;
    void setCurrentIndex(int index);

Q_SIGNALS:
    void currentIndexChanged();
};
}
