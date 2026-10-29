// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick.Controls as Controls
import org.kde.union.impl as Union

Controls.ToolButton {
    property bool selected: false

    Union.Element.type: "CloseButton"
    display: Controls.AbstractButton.IconOnly
}
