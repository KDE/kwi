// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Templates as T
import org.kde.kwi

T.ToolButton {
    id: control

    property bool selected: false

    implicitWidth: implicitBackgroundWidth
    implicitHeight: implicitBackgroundHeight
    hoverEnabled: Application.styleHints.useHoverEffects

    background: NativeTabClose {
        objectName: "nativeTabCloseVisual"
        hovered: control.hovered
        pressed: control.down
        selected: control.selected
    }
}
