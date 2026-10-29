// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami

Controls.ToolButton {
    property bool selected: false

    icon.name: "tab-close-other"
    icon.width: Kirigami.Units.iconSizes.small
    icon.height: Kirigami.Units.iconSizes.small
    display: Controls.AbstractButton.IconOnly
}
