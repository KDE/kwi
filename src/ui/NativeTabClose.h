// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QImage>
#include <QMetaObject>
#include <QPointer>
#include <QQuickItem>
#include <QRectF>
#include <QtQmlIntegration/qqmlintegration.h>

class QEvent;
class QSGNode;
class QStyle;

namespace Kirigami::Platform
{
class PlatformTheme;
}

namespace Kwi
{
class NativeTabClose : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool hovered MEMBER m_hovered NOTIFY hoveredChanged)
    Q_PROPERTY(bool pressed MEMBER m_pressed NOTIFY pressedChanged)
    Q_PROPERTY(bool selected MEMBER m_selected NOTIFY selectedChanged)

public:
    explicit NativeTabClose(QQuickItem *parent = nullptr);

Q_SIGNALS:
    void hoveredChanged();
    void pressedChanged();
    void selectedChanged();

protected:
    bool event(QEvent *incomingEvent) override;
    void itemChange(ItemChange change, const ItemChangeData &data) override;
    void geometryChange(const QRectF &geometry, const QRectF &oldGeometry) override;
    void updatePolish() override;
    QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *data) override;

private:
    void updateStyle();

    Kirigami::Platform::PlatformTheme *m_theme{nullptr};
    QImage m_image{};
    QPointer<QStyle> m_style{};
    QMetaObject::Connection m_styleConnection{};
    bool m_hovered{false};
    bool m_pressed{false};
    bool m_selected{false};
};
}
