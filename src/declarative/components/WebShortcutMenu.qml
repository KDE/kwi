// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

pragma ComponentBehavior: Bound

import QtQml.Models
import QtQuick.Controls as Controls
import org.kde.ki18n
import org.kde.kwi

Controls.Menu {
    id: root

    required property WebShortcutModel providers

    signal urlRequested(url url)

    title: providers.title
    icon.name: "preferences-web-browser-shortcuts"

    Instantiator {
        model: root.providers

        delegate: Controls.MenuItem {
            required property int index
            required property string providerName
            required property string iconName

            text: providerName.replace(/&/g, "&&")
            icon.name: iconName

            onTriggered: root.urlRequested(root.providers.searchUrl(index))
        }

        onObjectAdded: (index, object) => root.insertItem(index, object)
        onObjectRemoved: (index, object) => root.removeItem(object)
    }

    Controls.MenuSeparator {
        visible: root.providers.canConfigure
    }

    Controls.MenuItem {
        text: KI18n.i18nc("@action:inmenu", "Configure Web Shortcuts…")
        icon.name: "configure"
        visible: root.providers.canConfigure

        onTriggered: root.providers.configure()
    }
}
