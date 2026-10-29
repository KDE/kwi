// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.ki18n
import org.kde.kirigami as Kirigami
import org.kde.kwi.terminal

Controls.ToolBar {
    id: root

    required property TerminalView terminal
    property alias text: findField.text

    signal closeRequested()

    Keys.onEscapePressed: root.closeRequested()

    function focusSearch(): void {
        findField.forceActiveFocus();
        findField.selectAll();
    }

    function cancelPendingSearch(): void { findTimer.stop(); }

    function search(forward: bool): void {
        findTimer.stop();

        if (findField.text.length > 0)
            terminal.find(findField.text, forward);
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.smallSpacing
        spacing: Kirigami.Units.smallSpacing

        Controls.TextField {
            id: findField

            objectName: "findField"
            // The status padding depends on width, so exclude it from implicitWidth.
            implicitWidth: Kirigami.Units.gridUnit * 24
            Layout.fillWidth: true
            placeholderText: KI18n.i18nc("@info:placeholder", "Find in terminal…")
            Accessible.name: KI18n.i18nc("@label:textbox", "Find in terminal")
            Accessible.description: root.terminal.searchStatus
            Accessible.searchEdit: true

            // Reserve status space so updates don't move the text or buttons.
            leftPadding: root.mirrored ? feedback.width + padding + Kirigami.Units.smallSpacing : padding
            rightPadding: root.mirrored ? padding : feedback.width + padding + Kirigami.Units.smallSpacing

            onTextEdited: {
                if (text.length > 0) {
                    findTimer.restart();
                } else {
                    findTimer.stop();
                    root.terminal.clearSearch();
                }
            }

            onAccepted: root.search(true)
            Keys.onReturnPressed: event => root.search(!(event.modifiers & Qt.ShiftModifier))
            Keys.onEnterPressed: event => root.search(!(event.modifiers & Qt.ShiftModifier))

            RowLayout {
                id: feedback

                anchors.right: parent.right
                anchors.rightMargin: findField.padding
                anchors.verticalCenter: parent.verticalCenter
                width: Math.min(findField.width * 0.4, Kirigami.Units.gridUnit * 12)
                spacing: Kirigami.Units.smallSpacing
                LayoutMirroring.enabled: root.mirrored
                LayoutMirroring.childrenInherit: true

                Controls.BusyIndicator {
                    running: root.terminal.searchBusy
                    visible: running
                    implicitWidth: Kirigami.Units.iconSizes.smallMedium
                    implicitHeight: implicitWidth
                    Accessible.name: KI18n.i18nc("@info:accessibility", "Searching terminal output")
                    Accessible.ignored: !running
                }

                Controls.Label {
                    id: feedbackLabel

                    Layout.fillWidth: true
                    Layout.minimumWidth: 0
                    text: root.terminal.searchStatus
                    textFormat: Text.PlainText
                    font: findField.font
                    color: findField.color
                    horizontalAlignment: Text.AlignRight
                    elide: Text.ElideRight
                    Accessible.role: Accessible.StaticText

                    HoverHandler { id: statusHover }

                    Controls.ToolTip {
                        objectName: "searchStatusToolTip"
                        text: feedbackLabel.text
                        visible: feedbackLabel.truncated && (statusHover.hovered || findField.activeFocus)
                        delay: Kirigami.Units.toolTipDelay
                    }
                }
            }

            Timer {
                id: findTimer

                interval: 120

                onTriggered: root.search(true)
            }
        }

        Controls.ToolButton {
            icon.name: "go-up"
            Kirigami.MnemonicData.enabled: false
            text: KI18n.i18nc("@action:button", "Next Match")
            display: Controls.AbstractButton.IconOnly
            enabled: findField.text.length > 0

            onClicked: root.search(true)

            Controls.ToolTip.text: text
            Controls.ToolTip.visible: hovered
            Controls.ToolTip.delay: Kirigami.Units.toolTipDelay
        }

        Controls.ToolButton {
            icon.name: "go-down"
            Kirigami.MnemonicData.enabled: false
            text: KI18n.i18nc("@action:button", "Previous Match")
            display: Controls.AbstractButton.IconOnly
            enabled: findField.text.length > 0

            onClicked: root.search(false)

            Controls.ToolTip.text: text
            Controls.ToolTip.visible: hovered
            Controls.ToolTip.delay: Kirigami.Units.toolTipDelay
        }

        Controls.ToolButton {
            icon.name: "dialog-close"
            Kirigami.MnemonicData.enabled: false
            text: KI18n.i18nc("@action:button", "Close Find Bar")
            display: Controls.AbstractButton.IconOnly

            onClicked: root.closeRequested()

            Controls.ToolTip.text: text
            Controls.ToolTip.visible: hovered
            Controls.ToolTip.delay: Kirigami.Units.toolTipDelay
        }
    }
}
