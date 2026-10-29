// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include "kwiterminal_export.h"
#include <QString>
#include <QStringView>

namespace Kwi
{
inline constexpr QStringView DefaultTitleFormat{u"{directory} : {title}"};
inline constexpr QStringView DefaultRemoteTitleFormat{u"{user}@{host} : {title}"};

struct TitleContext {
    QString title{};
    QString path{};
    QString user{};
    QString host{};
    QString command{};
    QString homeDirectory{};
    QString shortHost{};
};

struct TitleFormatResult {
    QString text{};
    QString error{};
};

[[nodiscard]] KWI_TERMINAL_EXPORT TitleFormatResult formatTitle(QStringView format, const TitleContext &context = {});
}
