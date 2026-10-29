// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "Unicode.h"

#include <QChar>
#include <QString>
#include <cstddef>
#include <cstdint>
#include <ghostty/vt.h>

namespace GhosttyQt
{
QVector<Grapheme> graphemes(QStringView text)
{
    const auto codepoints = text.toString().toUcs4();
    size_t offset{0};
    QVector<Grapheme> result;

    while (offset < size_t(codepoints.size())) {
        uint8_t width{0};
        const auto count =
            ghostty_unicode_grapheme_width(reinterpret_cast<const uint32_t *>(codepoints.constData()) + offset, size_t(codepoints.size()) - offset, &width);
        qsizetype length{0};

        for (size_t i{0}; i < count; ++i) {
            length += QChar::requiresSurrogates(codepoints[qsizetype(offset + i)]) ? 2 : 1;
        }

        result.append({length, width});
        offset += count;
    }

    return result;
}
}
