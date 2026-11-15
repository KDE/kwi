// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Controls.ToolBar {
    id: root

    required property Controls.Action action
    property int actionRole: Controls.DialogButtonBox.AcceptRole
    property Component leadingComponent

    signal rejected()

    position: Controls.ToolBar.Footer
    padding: Kirigami.Units.largeSpacing
    background: null

    function focusCancel(): void {
        buttonBox.standardButton(Controls.DialogButtonBox.Cancel).forceActiveFocus(Qt.PopupFocusReason);
    }

    contentItem: RowLayout {
        spacing: Kirigami.Units.largeSpacing

        Loader {
            visible: item !== null
            sourceComponent: root.leadingComponent
        }

        Controls.DialogButtonBox {
            id: buttonBox

            Layout.fillWidth: true
            padding: 0
            background: null
            standardButtons: Controls.DialogButtonBox.Cancel

            onRejected: root.rejected()

            Controls.Button {
                objectName: "promptActionButton"
                action: root.action
                Controls.DialogButtonBox.buttonRole: root.actionRole
            }
        }
    }
}
