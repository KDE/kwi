// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include "kwiterminal_export.h"

#include <KJob>
#include <QPointer>
#include <QString>
#include <memory>

class QThread;

namespace GhosttyQt
{
class HistoryReader;
class Terminal;
}

namespace Kwi
{
class KWI_TERMINAL_EXPORT OutputSaveJob final : public KJob
{
    Q_OBJECT

public:
    OutputSaveJob(GhosttyQt::Terminal *terminal, const QString &path, QObject *parent = nullptr);
    ~OutputSaveJob() override;

    void start() override;

protected:
    bool doKill() override;

private:
    std::unique_ptr<GhosttyQt::HistoryReader> m_reader;
    QString m_path;
    QPointer<QThread> m_thread;
};
}
