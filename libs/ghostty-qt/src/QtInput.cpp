// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "QtInput_p.h"
#include <QGuiApplication>
#include <QKeyEvent>
#ifdef Q_OS_LINUX
#include <linux/input-event-codes.h>
#endif
namespace GhosttyQt
{
namespace
{
constexpr GhosttyKey logicalKey(int key, Qt::KeyboardModifiers mods)
{
    if (mods.testFlag(Qt::KeypadModifier)) {
        if (key >= Qt::Key_0 && key <= Qt::Key_9) {
            return GhosttyKey(GHOSTTY_KEY_NUMPAD_0 + key - Qt::Key_0);
        }

        switch (key) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
            return GHOSTTY_KEY_NUMPAD_ENTER;
        case Qt::Key_Plus:
            return GHOSTTY_KEY_NUMPAD_ADD;
        case Qt::Key_Minus:
            return GHOSTTY_KEY_NUMPAD_SUBTRACT;
        case Qt::Key_Asterisk:
            return GHOSTTY_KEY_NUMPAD_MULTIPLY;
        case Qt::Key_Slash:
            return GHOSTTY_KEY_NUMPAD_DIVIDE;
        case Qt::Key_Period:
        case Qt::Key_Comma:
            return GHOSTTY_KEY_NUMPAD_DECIMAL;
        case Qt::Key_Equal:
            return GHOSTTY_KEY_NUMPAD_EQUAL;
        case Qt::Key_Home:
            return GHOSTTY_KEY_NUMPAD_HOME;
        case Qt::Key_End:
            return GHOSTTY_KEY_NUMPAD_END;
        case Qt::Key_Insert:
            return GHOSTTY_KEY_NUMPAD_INSERT;
        case Qt::Key_Delete:
            return GHOSTTY_KEY_NUMPAD_DELETE;
        case Qt::Key_PageUp:
            return GHOSTTY_KEY_NUMPAD_PAGE_UP;
        case Qt::Key_PageDown:
            return GHOSTTY_KEY_NUMPAD_PAGE_DOWN;
        case Qt::Key_Up:
            return GHOSTTY_KEY_NUMPAD_UP;
        case Qt::Key_Down:
            return GHOSTTY_KEY_NUMPAD_DOWN;
        case Qt::Key_Left:
            return GHOSTTY_KEY_NUMPAD_LEFT;
        case Qt::Key_Right:
            return GHOSTTY_KEY_NUMPAD_RIGHT;
        case Qt::Key_Clear:
            return GHOSTTY_KEY_NUMPAD_BEGIN;
        default:
            break;
        }
    }

    if (key >= Qt::Key_A && key <= Qt::Key_Z) {
        return GhosttyKey(GHOSTTY_KEY_A + key - Qt::Key_A);
    }

    if (key >= Qt::Key_0 && key <= Qt::Key_9) {
        return GhosttyKey(GHOSTTY_KEY_DIGIT_0 + key - Qt::Key_0);
    }

    if (key >= Qt::Key_F1 && key <= Qt::Key_F25) {
        return GhosttyKey(GHOSTTY_KEY_F1 + key - Qt::Key_F1);
    }

    switch (key) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        return GHOSTTY_KEY_ENTER;
    case Qt::Key_Escape:
        return GHOSTTY_KEY_ESCAPE;
    case Qt::Key_Tab:
    case Qt::Key_Backtab:
        return GHOSTTY_KEY_TAB;
    case Qt::Key_Backspace:
        return GHOSTTY_KEY_BACKSPACE;
    case Qt::Key_Insert:
        return GHOSTTY_KEY_INSERT;
    case Qt::Key_Delete:
        return GHOSTTY_KEY_DELETE;
    case Qt::Key_Home:
        return GHOSTTY_KEY_HOME;
    case Qt::Key_End:
        return GHOSTTY_KEY_END;
    case Qt::Key_PageUp:
        return GHOSTTY_KEY_PAGE_UP;
    case Qt::Key_PageDown:
        return GHOSTTY_KEY_PAGE_DOWN;
    case Qt::Key_Up:
        return GHOSTTY_KEY_ARROW_UP;
    case Qt::Key_Down:
        return GHOSTTY_KEY_ARROW_DOWN;
    case Qt::Key_Left:
        return GHOSTTY_KEY_ARROW_LEFT;
    case Qt::Key_Right:
        return GHOSTTY_KEY_ARROW_RIGHT;
    case Qt::Key_Space:
        return GHOSTTY_KEY_SPACE;
    case Qt::Key_Minus:
        return GHOSTTY_KEY_MINUS;
    case Qt::Key_Equal:
    case Qt::Key_Plus:
        return GHOSTTY_KEY_EQUAL;
    case Qt::Key_BracketLeft:
        return GHOSTTY_KEY_BRACKET_LEFT;
    case Qt::Key_BracketRight:
        return GHOSTTY_KEY_BRACKET_RIGHT;
    case Qt::Key_Backslash:
        return GHOSTTY_KEY_BACKSLASH;
    case Qt::Key_Semicolon:
        return GHOSTTY_KEY_SEMICOLON;
    case Qt::Key_Apostrophe:
        return GHOSTTY_KEY_QUOTE;
    case Qt::Key_QuoteLeft:
        return GHOSTTY_KEY_BACKQUOTE;
    case Qt::Key_Comma:
        return GHOSTTY_KEY_COMMA;
    case Qt::Key_Period:
        return GHOSTTY_KEY_PERIOD;
    case Qt::Key_Slash:
        return GHOSTTY_KEY_SLASH;
    case Qt::Key_Shift:
        return GHOSTTY_KEY_SHIFT_LEFT;
    case Qt::Key_Control:
        return GHOSTTY_KEY_CONTROL_LEFT;
    case Qt::Key_Alt:
        return GHOSTTY_KEY_ALT_LEFT;
    case Qt::Key_AltGr:
        return GHOSTTY_KEY_ALT_RIGHT;
    case Qt::Key_Meta:
        return GHOSTTY_KEY_META_LEFT;
    case Qt::Key_CapsLock:
        return GHOSTTY_KEY_CAPS_LOCK;
    case Qt::Key_NumLock:
        return GHOSTTY_KEY_NUM_LOCK;
    case Qt::Key_Menu:
        return GHOSTTY_KEY_CONTEXT_MENU;
    case Qt::Key_Print:
        return GHOSTTY_KEY_PRINT_SCREEN;
    case Qt::Key_ScrollLock:
        return GHOSTTY_KEY_SCROLL_LOCK;
    case Qt::Key_Pause:
        return GHOSTTY_KEY_PAUSE;
    default:
        return GHOSTTY_KEY_UNIDENTIFIED;
    }
}

GhosttyKey physicalKey(const QKeyEvent &event)
{
    // TODO: Verify FreeBSD Wayland events and resolve FreeBSD X11 keys through XKB names.
    // Its X11 keycode tables can differ from Linux's evdev map.
#ifdef Q_OS_LINUX
    // Qt Wayland adds 8 to evdev keycodes; Qt XCB passes XKB keycodes through.
    // Linux's evdev XKB map uses the same offset. Keep synthetic events and
    // other platform plugins on the logical fallback.
    static const bool usesEvdev =
        QGuiApplication::platformName() == QLatin1String("xcb") || QGuiApplication::platformName().startsWith(QLatin1String("wayland"));

    if (!usesEvdev || event.nativeScanCode() < 8 || event.modifiers().testFlag(Qt::KeypadModifier)) {
        return logicalKey(event.key(), event.modifiers());
    }

    switch (event.nativeScanCode() - 8) {
    case KEY_A:
        return GHOSTTY_KEY_A;
    case KEY_B:
        return GHOSTTY_KEY_B;
    case KEY_C:
        return GHOSTTY_KEY_C;
    case KEY_D:
        return GHOSTTY_KEY_D;
    case KEY_E:
        return GHOSTTY_KEY_E;
    case KEY_F:
        return GHOSTTY_KEY_F;
    case KEY_G:
        return GHOSTTY_KEY_G;
    case KEY_H:
        return GHOSTTY_KEY_H;
    case KEY_I:
        return GHOSTTY_KEY_I;
    case KEY_J:
        return GHOSTTY_KEY_J;
    case KEY_K:
        return GHOSTTY_KEY_K;
    case KEY_L:
        return GHOSTTY_KEY_L;
    case KEY_M:
        return GHOSTTY_KEY_M;
    case KEY_N:
        return GHOSTTY_KEY_N;
    case KEY_O:
        return GHOSTTY_KEY_O;
    case KEY_P:
        return GHOSTTY_KEY_P;
    case KEY_Q:
        return GHOSTTY_KEY_Q;
    case KEY_R:
        return GHOSTTY_KEY_R;
    case KEY_S:
        return GHOSTTY_KEY_S;
    case KEY_T:
        return GHOSTTY_KEY_T;
    case KEY_U:
        return GHOSTTY_KEY_U;
    case KEY_V:
        return GHOSTTY_KEY_V;
    case KEY_W:
        return GHOSTTY_KEY_W;
    case KEY_X:
        return GHOSTTY_KEY_X;
    case KEY_Y:
        return GHOSTTY_KEY_Y;
    case KEY_Z:
        return GHOSTTY_KEY_Z;
    case KEY_0:
        return GHOSTTY_KEY_DIGIT_0;
    case KEY_1:
        return GHOSTTY_KEY_DIGIT_1;
    case KEY_2:
        return GHOSTTY_KEY_DIGIT_2;
    case KEY_3:
        return GHOSTTY_KEY_DIGIT_3;
    case KEY_4:
        return GHOSTTY_KEY_DIGIT_4;
    case KEY_5:
        return GHOSTTY_KEY_DIGIT_5;
    case KEY_6:
        return GHOSTTY_KEY_DIGIT_6;
    case KEY_7:
        return GHOSTTY_KEY_DIGIT_7;
    case KEY_8:
        return GHOSTTY_KEY_DIGIT_8;
    case KEY_9:
        return GHOSTTY_KEY_DIGIT_9;
    case KEY_MINUS:
        return GHOSTTY_KEY_MINUS;
    case KEY_EQUAL:
        return GHOSTTY_KEY_EQUAL;
    case KEY_LEFTBRACE:
        return GHOSTTY_KEY_BRACKET_LEFT;
    case KEY_RIGHTBRACE:
        return GHOSTTY_KEY_BRACKET_RIGHT;
    case KEY_BACKSLASH:
        return GHOSTTY_KEY_BACKSLASH;
    case KEY_SEMICOLON:
        return GHOSTTY_KEY_SEMICOLON;
    case KEY_APOSTROPHE:
        return GHOSTTY_KEY_QUOTE;
    case KEY_GRAVE:
        return GHOSTTY_KEY_BACKQUOTE;
    case KEY_COMMA:
        return GHOSTTY_KEY_COMMA;
    case KEY_DOT:
        return GHOSTTY_KEY_PERIOD;
    case KEY_SLASH:
        return GHOSTTY_KEY_SLASH;
    case KEY_102ND:
        return GHOSTTY_KEY_INTL_BACKSLASH;
    case KEY_RO:
        return GHOSTTY_KEY_INTL_RO;
    case KEY_YEN:
        return GHOSTTY_KEY_INTL_YEN;
    case KEY_LEFTSHIFT:
        return GHOSTTY_KEY_SHIFT_LEFT;
    case KEY_RIGHTSHIFT:
        return GHOSTTY_KEY_SHIFT_RIGHT;
    case KEY_LEFTCTRL:
        return GHOSTTY_KEY_CONTROL_LEFT;
    case KEY_RIGHTCTRL:
        return GHOSTTY_KEY_CONTROL_RIGHT;
    case KEY_LEFTALT:
        return GHOSTTY_KEY_ALT_LEFT;
    case KEY_RIGHTALT:
        return GHOSTTY_KEY_ALT_RIGHT;
    case KEY_LEFTMETA:
        return GHOSTTY_KEY_META_LEFT;
    case KEY_RIGHTMETA:
        return GHOSTTY_KEY_META_RIGHT;
    default:
        return logicalKey(event.key(), event.modifiers());
    }
#else
    return logicalKey(event.key(), event.modifiers());
#endif
}
}

GhosttyMods ghosttyModifiers(Qt::KeyboardModifiers mods)
{
    return (mods.testFlag(Qt::ShiftModifier) ? GHOSTTY_MODS_SHIFT : 0) | (mods.testFlag(Qt::ControlModifier) ? GHOSTTY_MODS_CTRL : 0)
        | (mods.testFlag(Qt::AltModifier) ? GHOSTTY_MODS_ALT : 0) | (mods.testFlag(Qt::MetaModifier) ? GHOSTTY_MODS_SUPER : 0);
}

KeyInput keyInput(const QKeyEvent &event)
{
    KeyInput result;
    result.key = physicalKey(event);
    result.action = event.type() == QEvent::KeyRelease ? GHOSTTY_KEY_ACTION_RELEASE
        : event.isAutoRepeat()                         ? GHOSTTY_KEY_ACTION_REPEAT
                                                       : GHOSTTY_KEY_ACTION_PRESS;
    auto modifiers = event.modifiers();

    if (event.key() == Qt::Key_Backtab)
        modifiers |= Qt::ShiftModifier;

    // Qt already applies the keyboard layout and AltGr to the event text.
    if (modifiers.testFlag(Qt::GroupSwitchModifier))
        modifiers &= ~(Qt::ControlModifier | Qt::AltModifier);

    result.modifiers = ghosttyModifiers(modifiers);
    result.text = event.text().toUtf8();

    if (event.key() >= 0x20 && event.key() <= 0x10ffff)
        result.codepoint = QChar::toLower(char32_t(event.key()));
    else if (!event.text().isEmpty())
        result.codepoint = event.text().toUcs4().first();
    const bool printable{!event.text().isEmpty() && event.text().front().isPrint()};

    if (printable && !modifiers.testFlag(Qt::ControlModifier))
        result.consumedModifiers = ghosttyModifiers(modifiers & Qt::ShiftModifier);

    // Ghostty needs printable key text rather than Qt's C0 text for Ctrl+letter.
    if (modifiers.testFlag(Qt::ControlModifier) && result.text.size() == 1 && quint8(result.text.at(0)) < 0x20 && result.codepoint >= 0x20
        && result.codepoint <= 0x10ffff) {
        char32_t printableCodepoint{result.codepoint};

        if (modifiers.testFlag(Qt::ShiftModifier) && printableCodepoint >= U'a' && printableCodepoint <= U'z')
            printableCodepoint -= U'a' - U'A';

        result.text = QString::fromUcs4(&printableCodepoint, 1).toUtf8();
    }

    return result;
}
}
