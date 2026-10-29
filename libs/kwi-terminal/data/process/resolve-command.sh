#!/bin/sh
# SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
# SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

program=$1

account=$(/usr/bin/getent passwd "$(/usr/bin/id -u)") || account=
account_shell=${account##*:}
default_shell=${SHELL:-$account_shell}
[ -n "$default_shell" ] || default_shell=/bin/sh

if [ "$2" = 1 ]; then
    program=$default_shell
fi

[ -n "$program" ] || exit 1

case "$program" in
    */*) resolved=$program ;;
    *)
        search=${PATH-/usr/bin:/bin}

        # /bin/sh may initialize PATH itself after env -u PATH. Preserve the
        # caller's removal by searching the host's standard executable path.
        [ "$3" != 1 ] || search=$(/usr/bin/getconf PATH)
        resolved=

        while :; do
            directory=${search%%:*}
            [ -n "$directory" ] || directory=.
            candidate=$directory/$program

            if [ -x "$candidate" ] && [ ! -d "$candidate" ]; then
                resolved=$candidate
                break
            fi

            case "$search" in *:*) search=${search#*:} ;; *) break ;; esac
        done
        ;;
esac

[ -x "$resolved" ] && [ ! -d "$resolved" ] || exit 1

canonical=$(/usr/bin/readlink -f -- "$resolved") || exit 1

matches_shell()
{
    [ -n "$1" ] && { [ "$1" = "$resolved" ] || [ "$(/usr/bin/readlink -f -- "$1")" = "$canonical" ]; }
}

is_shell()
{
    for candidate in "$default_shell" "$account_shell" /bin/sh; do
        matches_shell "$candidate" && return 0
    done

    while IFS= read -r candidate; do
        candidate=${candidate%%#*}
        candidate=${candidate#"${candidate%%[![:space:]]*}"}
        candidate=${candidate%"${candidate##*[![:space:]]}"}
        matches_shell "$candidate" && return 0
    done < /etc/shells

    return 1
}

if is_shell; then shell=1; else shell=0; fi

printf '%s\000' "$resolved" "$canonical" "$shell"
