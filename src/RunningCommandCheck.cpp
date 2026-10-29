// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "RunningCommandCheck.h"
#include "TerminalSession.h"

#include <algorithm>
#include <memory>
#include <utility>

namespace Kwi
{
void checkRunningCommands(const QList<QPointer<TerminalSession>> &sessions, QObject *context, std::function<void(bool)> completed)
{
    if (sessions.isEmpty()) {
        completed(false);

        return;
    }

    struct State {
        qsizetype remaining;
        QList<QPointer<TerminalSession>> sessions{};
        std::function<void(bool)> completed;
    };

    const auto state = std::make_shared<State>(sessions.size(), sessions, std::move(completed));

    const QPointer<QObject> request{new QObject{context}};

    // Cancel before the owner destroys the sessions this request observes.
    const QPointer<QObject> owner{context};

    for (const auto &session : sessions) {
        const auto handled = std::make_shared<bool>(false);
        const auto done = [owner, request, state, handled] {
            if (!owner || !request || std::exchange(*handled, true)) {
                return;
            }

            if (--state->remaining == 0) {
                request->deleteLater();

                // Earlier replies may be stale by the time the last session responds.
                const bool running = std::ranges::any_of(state->sessions, [](const auto &session) {
                    return session && session->isBusy();
                });

                state->completed(running);
            }
        };

        if (!session) {
            done();
            continue;
        }

        const auto destroyed = QObject::connect(session, &QObject::destroyed, request, done);
        session->refreshProcessInfo([destroyed, done] {
            QObject::disconnect(destroyed);
            done();
        });
    }
}
}
