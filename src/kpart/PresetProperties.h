// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QString>
#include <QVariant>

namespace Kwi
{
class TerminalSession;

/** Read-only KonsolePart property compatibility for shared settings and launch presets. */
namespace PresetProperties
{
[[nodiscard]] QVariant property(const TerminalSession *session, const QString &name);
}
}
