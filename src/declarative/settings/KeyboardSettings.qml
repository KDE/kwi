// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Window
import org.kde.kirigami.actioncollection as ActionCollection

ActionCollection.ShortcutsEditor {
    id: root

    objectName: "keyboardSettings"

    // The shortcut editor calls this PageRow hook even when hosted in ConfigurationView.
    function closeDialog(): void {
        root.Window.window.close();
    }
}
