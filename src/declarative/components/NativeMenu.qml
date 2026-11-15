// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

pragma ComponentBehavior: Bound

import QtQml.Models
import Qt.labs.platform as Platform
import org.kde.kirigami as Kirigami

Platform.Menu {
    id: root

    required property list<Kirigami.Action> actions

    Instantiator {
        model: root.actions

        delegate: NativeMenuItem {
            required property Kirigami.Action modelData

            action: modelData
        }

        onObjectAdded: (index, object) => root.insertItem(index, object)
        onObjectRemoved: (index, object) => root.removeItem(object)
    }
}
