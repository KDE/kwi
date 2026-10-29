#!/bin/sh
# SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
# SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

# Check session and start time before each signal to avoid signaling a reused PID.
PATH=/usr/bin:/bin
leader=$1
members=$2

for signal in HUP TERM KILL; do
    for member in $members; do
        pid=${member%:*}
        start=${member#*:}

        IFS= read -r stat 2>/dev/null < "/proc/$pid/stat" || continue
        set -- ${stat##*) }

        [ "$4" = "$leader" ] && [ "${20}" = "$start" ] || continue

        kill -s "$signal" "$pid" 2>/dev/null || :
    done

    case "$signal" in HUP) sleep 0.5 ;; TERM) sleep 1 ;; esac
done
