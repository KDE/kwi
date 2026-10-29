// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Dialogs as Dialogs
import QtQuick.Layouts
import org.kde.ki18n
import org.kde.kirigami as Kirigami
import org.kde.kwi

Controls.Dialog {
    id: root

    required property AppSettings appSettings
    required property ColorSchemeModel terminalPalettes

    objectName: "colorSchemeEditor"

    property string schemeId: ""
    property bool creatingCopy: false
    property color foregroundColor: "white"
    property var boldForegroundColor
    property color backgroundColor: "black"
    property var ansi: []
    readonly property var colorNames: [KI18n.i18nc("@label:color", "Black"), KI18n.i18nc("@label:color", "Red"), KI18n.i18nc("@label:color", "Green"), KI18n.i18nc("@label:color", "Yellow"), KI18n.i18nc("@label:color", "Blue"), KI18n.i18nc("@label:color", "Magenta"), KI18n.i18nc("@label:color", "Cyan"), KI18n.i18nc("@label:color", "White")]
    readonly property var brightColorNames: [KI18n.i18nc("@label:color", "Bright Black"), KI18n.i18nc("@label:color", "Bright Red"), KI18n.i18nc("@label:color", "Bright Green"), KI18n.i18nc("@label:color", "Bright Yellow"), KI18n.i18nc("@label:color", "Bright Blue"), KI18n.i18nc("@label:color", "Bright Magenta"), KI18n.i18nc("@label:color", "Bright Cyan"), KI18n.i18nc("@label:color", "Bright White")]

    component PaletteColorButton: Controls.Button {
        id: button

        required property color color
        property string dialogTitle

        signal accepted(color color)

        Accessible.description: KI18n.i18nc("@info:accessibility", "Current color: %1. Opens a color chooser.", color)

        onActiveFocusChanged: if (activeFocus) root.revealControl(this)

        implicitWidth: Kirigami.Units.gridUnit * 3

        Rectangle {
            anchors.centerIn: parent
            width: Math.max(0, button.width - Kirigami.Units.smallSpacing * 2)
            height: Math.max(0, button.height - Kirigami.Units.smallSpacing * 2)
            color: button.color
            border.color: Kirigami.Theme.disabledTextColor
        }

        onClicked: {
            picker.selectedColor = color;
            picker.open();
        }

        Dialogs.ColorDialog {
            id: picker

            title: button.dialogTitle

            onAccepted: button.accepted(selectedColor)
        }
    }

    component ColorPairRow: RowLayout {
        id: colorEntry

        required property int colorIndex
        readonly property string colorName: root.colorNames[colorIndex]

        Kirigami.FormData.label: KI18n.i18nc("@label:color", "%1:", colorName)
        Layout.maximumWidth: implicitWidth
        spacing: Kirigami.Units.largeSpacing

        PaletteColorButton {
            objectName: "schemeAnsiButton" + colorEntry.colorIndex
            Layout.preferredWidth: colorForm.colorColumnWidth
            color: root.ansi.length === 16 ? root.ansi[colorEntry.colorIndex] : "black"
            dialogTitle: colorEntry.colorName
            Accessible.name: colorEntry.colorName

            onAccepted: color => root.setAnsiColor(colorEntry.colorIndex, color)
        }

        PaletteColorButton {
            objectName: "schemeAnsiButton" + (colorEntry.colorIndex + 8)
            Layout.preferredWidth: colorForm.colorColumnWidth
            color: root.ansi.length === 16 ? root.ansi[colorEntry.colorIndex + 8] : "black"
            dialogTitle: root.brightColorNames[colorEntry.colorIndex]
            Accessible.name: dialogTitle

            onAccepted: color => root.setAnsiColor(colorEntry.colorIndex + 8, color)
        }
    }

    function edit(id: string, createCopy: bool): void {
        const scheme = root.terminalPalettes.schemeData(id);

        if (!scheme.id)
            return;

        schemeId = !createCopy && scheme.editable ? scheme.id : "";
        creatingCopy = createCopy;
        nameField.text = root.terminalPalettes.editingName(id, createCopy);
        foregroundColor = scheme.foreground;
        boldForegroundColor = scheme.boldForeground;
        backgroundColor = scheme.background;
        ansi = scheme.ansi;
        root.terminalPalettes.clearError();
        open();
    }

    function setAnsiColor(index: int, color: color): void {
        const updated = ansi.slice();
        updated[index] = color;
        ansi = updated;
    }

    function revealControl(item: Item): void {
        const position = item.mapToItem(editorContent, 0, 0);
        editorPage.ensureVisible(item,
                               position.x - item.x + editorPage.leftPadding,
                               position.y - item.y + editorPage.topPadding);
    }

    function save(): void {
        if (root.terminalPalettes.saveScheme(schemeId, nameField.text, foregroundColor, backgroundColor, ansi, boldForegroundColor))
            accept();
    }

    parent: Controls.Overlay.overlay
    anchors.centerIn: parent
    width: Math.min(Kirigami.Units.gridUnit * 34, parent.width - 2 * margins)
    height: Math.min(implicitHeight, parent.height - 2 * margins)
    padding: 0

    footer: Controls.ToolBar {
        position: Controls.ToolBar.Footer
        padding: 0

        background: Item {
            Kirigami.Separator {
                width: parent.width
                anchors.top: parent.top
            }
        }

        contentItem: Controls.DialogButtonBox {
            background: null
            standardButtons: Controls.DialogButtonBox.Cancel

            onAccepted: root.save()
            onRejected: root.reject()

            Controls.Button {
                objectName: "schemeSaveButton"
                text: KI18n.i18nc("@action:button", "Save")
                icon.name: "document-save"
                enabled: nameField.text.trim().length > 0
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.AcceptRole
            }
        }
    }

    title: creatingCopy ? KI18n.i18nc("@title:window", "Duplicate Color Scheme") : KI18n.i18nc("@title:window", "Edit Color Scheme")
    modal: true
    focus: true
    closePolicy: Controls.Popup.CloseOnEscape

    onOpened: nameField.forceActiveFocus(Qt.PopupFocusReason)
    onRejected: root.terminalPalettes.clearError()

    contentItem: Kirigami.ScrollablePage {
        id: editorPage

        padding: Kirigami.Units.largeSpacing

        ColumnLayout {
            id: editorContent

            spacing: Kirigami.Units.largeSpacing

            Kirigami.InlineMessage {
                Layout.fillWidth: true
                visible: text.length > 0

                text: TextFormatting.plainTextToHtml(root.terminalPalettes.errorString)
                type: Kirigami.MessageType.Error
            }

            Kirigami.Heading {
                objectName: "schemePreviewHeading"
                text: KI18n.i18nc("@title:group", "Preview")
                level: 2
            }

            Rectangle {
                objectName: "schemePreview"
                Layout.fillWidth: true
                implicitHeight: preview.implicitHeight + Kirigami.Units.largeSpacing * 2
                color: root.backgroundColor
                border.color: Kirigami.Theme.disabledTextColor
                Accessible.role: Accessible.StaticText
                Accessible.name: KI18n.i18nc("@info:accessibility", "Color scheme preview")

                ColumnLayout {
                    id: preview

                    anchors.fill: parent
                    anchors.margins: Kirigami.Units.largeSpacing
                    spacing: Kirigami.Units.smallSpacing

                    Controls.Label {
                        Layout.fillWidth: true
                        text: KI18n.i18nc("@info color scheme preview. Keep the shell command syntax intact.", "user@host:~/Documents $ echo \"Hello, world\"")
                        color: root.foregroundColor
                        font.family: root.appSettings.fontFamily
                        elide: Text.ElideRight
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        columns: 8

                        Repeater {
                            model: root.ansi

                            Controls.Label {
                                required property int index
                                required property color modelData

                                Layout.fillWidth: true
                                text: KI18n.i18nc("@info short color sample", "Aa")
                                color: modelData
                                font.family: root.appSettings.fontFamily
                                font.bold: index >= 8
                                Accessible.name: index < 8 ? root.colorNames[index] : root.brightColorNames[index - 8]
                            }
                        }
                    }
                }
            }

            Kirigami.FormLayout {
                id: colorForm

                Layout.fillWidth: true

                readonly property real colorColumnWidth: Math.max(Kirigami.Units.gridUnit * 3,
                                                                   normalHeading.implicitWidth,
                                                                   brightHeading.implicitWidth)

                Controls.TextField {
                    id: nameField

                    objectName: "schemeNameField"
                    Kirigami.FormData.label: KI18n.i18nc("@label:textbox", "Name:")
                    Layout.fillWidth: true
                    maximumLength: 80

                    onActiveFocusChanged: if (activeFocus) root.revealControl(this)
                    onAccepted: root.save()
                }

                PaletteColorButton {
                    objectName: "schemeForegroundButton"
                    Kirigami.FormData.label: KI18n.i18nc("@label:color", "Text:")
                    Layout.preferredWidth: colorForm.colorColumnWidth
                    color: root.foregroundColor
                    dialogTitle: KI18n.i18nc("@title:window", "Text Color")
                    Accessible.name: KI18n.i18nc("@label:color", "Text")

                    onAccepted: color => root.foregroundColor = color
                }

                PaletteColorButton {
                    objectName: "schemeBackgroundButton"
                    Kirigami.FormData.label: KI18n.i18nc("@label:color", "Background:")
                    Layout.preferredWidth: colorForm.colorColumnWidth
                    color: root.backgroundColor
                    dialogTitle: KI18n.i18nc("@title:window", "Background Color")
                    Accessible.name: KI18n.i18nc("@label:color", "Background")

                    onAccepted: color => root.backgroundColor = color
                }

                Kirigami.Separator {
                    Kirigami.FormData.label: KI18n.i18nc("@title:group", "Terminal Colors")
                    Kirigami.FormData.isSection: true
                    Layout.fillWidth: true
                }

                RowLayout {
                    Layout.maximumWidth: implicitWidth
                    spacing: Kirigami.Units.largeSpacing

                    Controls.Label {
                        id: normalHeading

                        Layout.preferredWidth: colorForm.colorColumnWidth
                        horizontalAlignment: Text.AlignHCenter
                        text: KI18n.i18nc("@label:column", "Normal")
                    }

                    Controls.Label {
                        id: brightHeading

                        Layout.preferredWidth: colorForm.colorColumnWidth
                        horizontalAlignment: Text.AlignHCenter
                        text: KI18n.i18nc("@label:column", "Bright")
                    }
                }

                Repeater {
                    model: 8

                    ColorPairRow {
                        required property int index

                        colorIndex: index
                    }
                }
            }
        }
    }
}
