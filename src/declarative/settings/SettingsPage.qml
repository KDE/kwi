// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: root

    property bool defaultsVisible: false
    property bool defaultsEnabled: true

    signal restoreDefaultsRequested

    footer: Controls.ToolBar {
        visible: root.defaultsVisible
        padding: 0

        contentItem: Controls.DialogButtonBox {
            objectName: "settingsDefaultsButtons"
            padding: Kirigami.Units.largeSpacing
            standardButtons: Controls.DialogButtonBox.RestoreDefaults
            enabled: root.defaultsEnabled

            onReset: root.restoreDefaultsRequested()
            Component.onCompleted: standardButton(Controls.DialogButtonBox.RestoreDefaults).objectName = "restoreDefaultsButton"
        }
    }
}
