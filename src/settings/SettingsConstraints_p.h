// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QStringView>
#include <QtTypes>

namespace Kwi::SettingsConstraints
{
inline constexpr qreal MinimumFontPointSize{6.0};
inline constexpr qreal MaximumFontPointSize{48.0};
inline constexpr int MaximumScrollbackLines{100000};
inline constexpr int MinimumSilenceSeconds{1};
inline constexpr int MaximumSilenceSeconds{3600};

[[nodiscard]] inline bool isValidCursorStyle(QStringView style)
{
    return style == u"block" || style == u"beam" || style == u"underline";
}
}
