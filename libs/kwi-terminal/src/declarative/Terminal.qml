// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

import QtQuick
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import org.kde.ki18n
import org.kde.kwi.terminal

/*!
    \qmltype Terminal
    \inqmlmodule org.kde.kwi.terminal
    \inherits FocusScope
    \brief A terminal view with a scrollbar and visual feedback.

    Set \l session to a \l Session to display its contents. The session must be
    started explicitly. Configure fonts, colors and input behavior through \l view.

    Terminal provides an always-visible scrollbar, a visual bell and a keyboard
    selection cursor. It forwards context menu, link activation and paste
    confirmation requests to the embedding application.
*/
FocusScope {
    id: root

    /*!
        \qmlproperty Session Terminal::session
        The session displayed by \l view. The default is null. Terminal does not own,
        start or close the session. A session can be attached to only one view at a time.

        \sa TerminalView::session
    */
    property alias session: terminal.session
    /*!
        \qmlproperty TerminalView Terminal::view
        \readonly
        The contained terminal view. Use it to configure appearance and input
        behavior, and to invoke terminal actions such as \l {TerminalView::copy()}{copy},
        \l {TerminalView::paste()}{paste} and \l {TerminalView::find()}{search}.
    */
    readonly property alias view: terminal

    /*!
        \qmlsignal Terminal::multilinePasteRequested(Session origin, string text)
        Emitted when pasting \a text into \a origin needs confirmation. Forwarded
        from \l view without changing the session or text.

        \sa TerminalView::multilinePasteRequested()
    */
    signal multilinePasteRequested(Session origin, string text)
    /*!
        \qmlsignal Terminal::contextMenuRequested(point position)
        Emitted when a context menu is requested at \a position in \l view coordinates.
        The embedding application creates and opens the menu.

        \sa TerminalView::contextMenuRequested()
    */
    signal contextMenuRequested(point position)
    /*!
        \qmlsignal Terminal::linkActivated(url url)
        Emitted when the user activates a link with target \a url. The embedding
        application decides how to open it.

        \sa TerminalView::linkActivated()
    */
    signal linkActivated(url url)

    KI18nContext {
        id: translations
        translationDomain: "kwi-terminal"
    }

    TerminalView {
        id: terminal

        objectName: "terminalView"
        anchors.fill: parent
        anchors.rightMargin: scrollbar.width
        focus: true
        Accessible.role: Accessible.EditableText
        Accessible.name: session ? translations.i18nc("@info:accessibility", "Terminal: %1", (session.applicationTitle || session.launchedProgramName)) : translations.i18nc("@info:accessibility", "Terminal")
        Accessible.description: translations.i18nc("@info:accessibility", "Terminal output and command input.")

        onMultilinePasteRequested: (origin, text) => root.multilinePasteRequested(origin, text)
        onContextMenuRequested: position => root.contextMenuRequested(position)
        onLinkActivated: url => root.linkActivated(url)
    }

    Rectangle {
        x: terminal.x + terminal.selectionCursorRectangle.x
        y: terminal.y + terminal.selectionCursorRectangle.y
        width: terminal.selectionCursorRectangle.width
        height: terminal.selectionCursorRectangle.height
        visible: terminal.keyboardSelectionMode
        color: "transparent"
        border.color: Kirigami.Theme.highlightColor
        border.width: 2
        Accessible.ignored: true
    }

    Rectangle {
        objectName: "visualBell"
        anchors.fill: terminal
        color: "transparent"
        border.color: Kirigami.Theme.highlightColor
        border.width: 2
        visible: bellPulse.running
        Accessible.ignored: true

        Timer {
            id: bellPulse

            interval: 150
        }

        Connections {
            target: root.session

            function onBell(): void {
                if (!bellPulse.running) {
                    terminal.Accessible.announce(translations.i18nc("@info:accessibility", "Terminal bell"), Accessible.Polite);
                }

                bellPulse.restart();
            }
        }
    }

    Rectangle {
        // Keep the scrollbar opaque even when its style paints a translucent background.
        anchors.fill: scrollbar
        color: scrollbar.Kirigami.Theme.backgroundColor
        Accessible.ignored: true
    }

    Controls.ScrollBar {
        id: scrollbar

        objectName: "terminalScrollBar"
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        orientation: Qt.Vertical

        Kirigami.Theme.inherit: false
        Kirigami.Theme.backgroundColor: terminal.colorScheme === "system" ? root.Kirigami.Theme.backgroundColor : terminal.backgroundColor
        Kirigami.Theme.textColor: terminal.colorScheme === "system" ? root.Kirigami.Theme.textColor : terminal.foregroundColor

        // HACK: The desktop style hides size == 1 even with AlwaysOn. One representable
        // step below 1 still rounds to zero scroll range. Remove when the style honors AlwaysOn.
        size: Math.min(terminal.scrollSize, 1 - Number.EPSILON / 2)
        stepSize: terminal.scrollStepSize
        policy: Controls.ScrollBar.AlwaysOn
        interactive: true

        Accessible.ignored: terminal.scrollSize === 1

        // Desktop styles also move the bar through arrow/page buttons and
        // middle clicks, without setting ScrollBar.pressed.
        onPositionChanged: terminal.scrollTo(position)

        Binding {
            target: scrollbar
            property: "position"
            value: terminal.scrollPosition
        }

        WheelHandler {
            target: null
            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad

            onWheel: event => terminal.scrollHistory(event.pixelDelta, event.angleDelta, event.modifiers)
        }
    }
}
