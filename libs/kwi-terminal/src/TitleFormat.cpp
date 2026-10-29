// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "TitleFormat_p.h"

#include <KLocalizedString>
#include <QDir>
#include <QFileInfo>

namespace Kwi
{
namespace
{
constexpr qsizetype maximumTitleLength{256};
}

TitleFormatResult formatTitle(QStringView format, const TitleContext &context)
{
    TitleFormatResult result{};

    if (format.trimmed().isEmpty()) {
        result.error = i18nd("kwi-terminal", "Enter a title format.");

        return result;
    }

    if (format.size() > maximumTitleLength) {
        result.error = i18nd("kwi-terminal", "Title formats can contain at most 256 characters.");

        return result;
    }

    const auto append = [&result](QStringView text) {
        result.text += text.left(maximumTitleLength - result.text.size());
    };

    for (qsizetype i{0}; i < format.size(); ++i) {
        const QChar character{format[i]};

        if (character.isNonCharacter() || character.category() == QChar::Other_Control) {
            result.error = i18nd("kwi-terminal", "Title formats cannot contain control characters.");

            return result;
        }

        if (character == QLatin1Char('{')) {
            if (i + 1 < format.size() && format[i + 1] == QLatin1Char('{')) {
                append(u"{");
                ++i;
                continue;
            }

            const auto end = format.indexOf(QLatin1Char('}'), i + 1);

            if (end < 0) {
                result.error = i18nd("kwi-terminal", "Close each placeholder with }. Use {{ and }} for literal braces.");

                return result;
            }

            const auto token = format.mid(i + 1, end - i - 1);

            if (token == u"title") {
                append(context.title.isEmpty() ? context.command : context.title);
            } else if (token == u"path" || token == u"directory") {
                QString path{context.path.isEmpty() ? QString{} : QDir::cleanPath(context.path)};

                if (!context.homeDirectory.isEmpty()) {
                    const QString home{QDir::cleanPath(context.homeDirectory)};
                    const QString prefix{home.endsWith(QLatin1Char('/')) ? home : home + QLatin1Char('/')};

                    if (path == home) {
                        path = QStringLiteral("~");
                    } else if (path.startsWith(prefix)) {
                        path.replace(0, prefix.size(), QStringLiteral("~/"));
                    }
                }

                if (token == u"path" || path.isEmpty() || path == QDir::rootPath()) {
                    append(path);
                } else {
                    append(QFileInfo(path).fileName());
                }
            } else if (token == u"user") {
                append(context.user);
            } else if (token == u"host") {
                append(context.host);
            } else if (token == u"shortHost") {
                append(context.shortHost.isEmpty() ? context.host.section(QLatin1Char('.'), 0, 0) : context.shortHost);
            } else if (token == u"command") {
                append(context.command);
            } else {
                result.error = i18nd("kwi-terminal", "Unknown title placeholder: {%1}", token.toString());

                return result;
            }

            i = end;
        } else if (character == QLatin1Char('}')) {
            if (i + 1 >= format.size() || format[i + 1] != QLatin1Char('}')) {
                result.error = i18nd("kwi-terminal", "Use }} for a literal closing brace.");

                return result;
            }

            append(u"}");
            ++i;
        } else {
            append(format.mid(i, 1));
        }
    }

    for (QChar &character : result.text) {
        if (character.isNonCharacter() || character.category() == QChar::Other_Control) {
            character = QLatin1Char(' ');
        }
    }

    if (!result.text.isEmpty() && result.text.back().isHighSurrogate()) {
        result.text.chop(1);
    }

    return result;
}
}
