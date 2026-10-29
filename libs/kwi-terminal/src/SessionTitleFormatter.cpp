// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "SessionTitleFormatter.h"
#include "Session.h"
#include "TitleFormat_p.h"

#include <QAbstractSocket>
#include <QHostAddress>
#include <utility>

namespace Kwi
{
namespace
{
TitleContext titleContext(const Session &session)
{
    const QString foreground{session.foregroundCommandName()};
    TitleContext context{session.applicationTitle(),
                         session.workingDirectory(),
                         session.localUserName(),
                         session.localHostName(),
                         foreground.isEmpty() ? session.launchedProgramName() : foreground,
                         session.homeDirectory()};

    if (session.hasSshInfo()) {
        context.user = session.sshUserName();
        context.host = session.sshHostName();
        const QString remoteCommand{session.sshRemoteCommand()};

        if (!remoteCommand.isEmpty()) {
            context.command = remoteCommand;
        }

        const QHostAddress address{context.host};
        context.shortHost = address.isNull() ? context.host.section(QLatin1Char('.'), 0, 0) : context.host;

        if (session.sshPort() != 22) {
            if (address.protocol() == QAbstractSocket::IPv6Protocol) {
                context.host = QLatin1Char('[') + context.host + QLatin1Char(']');
            }

            context.host += QLatin1Char(':') + QString::number(session.sshPort());
        }
    }

    return context;
}
}

SessionTitleFormatter::SessionTitleFormatter(QObject *parent)
    : QObject{parent}
    , m_format{DefaultTitleFormat.toString()}
{
}

Session *SessionTitleFormatter::session() const
{
    return m_session;
}

void SessionTitleFormatter::setSession(Session *newSession)
{
    if (m_session == newSession) {
        return;
    }

    if (m_session) {
        disconnect(m_session, nullptr, this, nullptr);
    }

    m_session = newSession;

    if (newSession) {
        connect(newSession, &QObject::destroyed, this, [this] {
            m_session.clear();
            update();

            Q_EMIT sessionChanged();
        });
        connect(newSession, &Session::applicationTitleChanged, this, &SessionTitleFormatter::update);
        connect(newSession, &Session::launchedProgramNameChanged, this, &SessionTitleFormatter::update);
        connect(newSession, &Session::workingDirectoryChanged, this, &SessionTitleFormatter::update);
        connect(newSession, &Session::foregroundCommandChanged, this, &SessionTitleFormatter::update);
        connect(newSession, &Session::sshInfoChanged, this, &SessionTitleFormatter::update);
    }

    update();

    Q_EMIT sessionChanged();
}

QString SessionTitleFormatter::format() const
{
    return m_format;
}

void SessionTitleFormatter::setFormat(const QString &newFormat)
{
    if (m_formatOverride && *m_formatOverride == newFormat) {
        return;
    }

    m_formatOverride = newFormat;
    update();
}

void SessionTitleFormatter::resetFormat()
{
    if (!m_formatOverride) {
        return;
    }

    m_formatOverride.reset();
    update();
}

QString SessionTitleFormatter::text() const
{
    return m_text;
}

QString SessionTitleFormatter::errorString() const
{
    return m_error;
}

void SessionTitleFormatter::update()
{
    const QString effectiveFormat{m_formatOverride.value_or((m_session && m_session->hasSshInfo() ? DefaultRemoteTitleFormat : DefaultTitleFormat).toString())};
    auto result = formatTitle(effectiveFormat, m_session ? titleContext(*m_session) : TitleContext{});

    if (!m_session || !result.error.isEmpty()) {
        result.text.clear();
    }

    const bool changedFormat{m_format != effectiveFormat};
    const bool changedText{m_text != result.text};
    const bool changedError{m_error != result.error};
    m_format = effectiveFormat;
    m_text = std::move(result.text);
    m_error = std::move(result.error);

    if (changedFormat) {
        Q_EMIT formatChanged();
    }

    if (changedText) {
        Q_EMIT textChanged();
    }

    if (changedError) {
        Q_EMIT errorChanged();
    }
}
}
