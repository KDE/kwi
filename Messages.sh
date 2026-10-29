#!/bin/sh
# SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
# Match Konsole's extraction of the bundled color scheme descriptions.
trap 'rm -f schemas.cpp' EXIT
sed -n 's/^Description=\(.*\)$/i18n("\1");/p' libs/kwi-terminal/data/color-schemes/*.colorscheme > schemas.cpp
$XGETTEXT $(find src -name '*.cpp' -o -name '*.h' -o -name '*.qml' | sort) schemas.cpp -o "$podir/kwi.pot"

$XGETTEXT $(find libs/kwi-terminal/src -name '*.cpp' -o -name '*.h' -o -name '*.qml' | sort) -o "$podir/kwi-terminal.pot"
