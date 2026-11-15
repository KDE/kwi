// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import Qt.labs.platform as Platform
import org.kde.kirigami as Kirigami

Platform.MenuItem {
    id: root

    required property Kirigami.Action action

    text: action?.text ?? ""
    icon.name: action?.icon.name ?? ""
    icon.source: action?.icon.source ?? ""
    shortcut: action?.shortcut ?? ""
    enabled: action?.enabled ?? false
    visible: action?.visible ?? true
    checkable: action?.checkable ?? false
    checked: action?.checked ?? false
    separator: action?.separator ?? false
    role: Platform.MenuItem.NoRole

    onTriggered: action?.trigger()
}
