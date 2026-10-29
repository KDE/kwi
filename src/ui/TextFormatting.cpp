// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "TextFormatting.h"

#include <KTextToHTML>

namespace Kwi
{
// TODO upstream: add qmltypes for KCoreAddons' dynamically registered KTextToHTML
// singleton. This downstream wrapper avoids qmllint's unresolved-type and
// missing-property errors. Call the public QML API directly once it has metadata.
QString TextFormatting::plainTextToHtml(const QString &text) const
{
    if (text.isEmpty()) {
        return {};
    }

    // Preserve literal spacing and force AutoText to decode escaped entities.
    return QStringLiteral("<span>") + KTextToHTML::convertToHtml(text, KTextToHTML::PreserveSpaces | KTextToHTML::IgnoreUrls) + QStringLiteral("</span>");
}
}
