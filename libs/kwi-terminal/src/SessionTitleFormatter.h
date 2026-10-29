// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include "kwiterminal_export.h"
#include <QObject>
#include <QPointer>
#include <QString>
#include <QtQmlIntegration/qqmlintegration.h>
#include <optional>

Q_MOC_INCLUDE("Session.h")

namespace Kwi
{
class Session;

class KWI_TERMINAL_EXPORT SessionTitleFormatter : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(Kwi::Session *session READ session WRITE setSession NOTIFY sessionChanged)
    Q_PROPERTY(QString format READ format WRITE setFormat RESET resetFormat NOTIFY formatChanged)
    Q_PROPERTY(QString text READ text NOTIFY textChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorChanged)

public:
    explicit SessionTitleFormatter(QObject *parent = nullptr);

    [[nodiscard]] Session *session() const;
    void setSession(Session *newSession);
    [[nodiscard]] QString format() const;
    void setFormat(const QString &newFormat);
    void resetFormat();
    [[nodiscard]] QString text() const;
    [[nodiscard]] QString errorString() const;

Q_SIGNALS:
    void sessionChanged();
    void formatChanged();
    void textChanged();
    void errorChanged();

private:
    void update();

    QPointer<Session> m_session;
    std::optional<QString> m_formatOverride;
    QString m_format;
    QString m_text;
    QString m_error;
};
}
