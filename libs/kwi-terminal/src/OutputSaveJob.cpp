// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "OutputSaveJob.h"

#include <HistoryReader.h>
#include <KLocalizedString>
#include <QByteArray>
#include <QMetaObject>
#include <QSaveFile>
#include <QThread>
#include <Terminal.h>

namespace Kwi
{
class OutputWriter final : public QObject
{
    Q_OBJECT

public:
    explicit OutputWriter(const QString &path, QObject *parent = nullptr)
        : QObject{parent}
        , m_file{path, this}
    {
    }

    void open()
    {
        if (QThread::currentThread()->isInterruptionRequested()) {
            return;
        }

        if (!m_file.open(QIODevice::WriteOnly)) {
            fail(m_file.errorString());
            return;
        }

        Q_EMIT ready();
    }

    void write(const QByteArray &bytes, bool last)
    {
        if (QThread::currentThread()->isInterruptionRequested()) {
            return;
        }

        if (m_file.write(bytes) != bytes.size()) {
            fail(m_file.errorString());
            return;
        }

        if (QThread::currentThread()->isInterruptionRequested()) {
            return;
        }

        if (last) {
            Q_EMIT finished(m_file.commit() ? QString{} : m_file.errorString());
        } else {
            Q_EMIT ready();
        }
    }

    void fail(const QString &error)
    {
        m_file.cancelWriting();

        Q_EMIT finished(error);
    }

Q_SIGNALS:
    void ready();
    void finished(const QString &error);

private:
    QSaveFile m_file;
};

OutputSaveJob::OutputSaveJob(GhosttyQt::Terminal *terminal, const QString &path, QObject *parent)
    : KJob{parent}
    , m_reader{terminal->createHistoryReader()}
    , m_path{path}
{
    setCapabilities(Killable);
}

OutputSaveJob::~OutputSaveJob()
{
    OutputSaveJob::doKill();
}

bool OutputSaveJob::doKill()
{
    if (m_thread) {
        m_thread->requestInterruption();
        m_thread->quit();
    }

    return true;
}

void OutputSaveJob::start()
{
    if (!m_reader) {
        setError(UserDefinedError);
        setErrorText(i18n("Could not read terminal output."));
        emitResult();
        return;
    }

    auto *thread = new QThread;
    m_thread = thread;
    auto *writer = new OutputWriter{m_path};
    writer->moveToThread(thread);
    connect(thread, &QThread::started, writer, &OutputWriter::open);
    connect(thread, &QThread::finished, writer, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    connect(writer, &OutputWriter::finished, thread, &QThread::quit, Qt::DirectConnection);

    // Wait for each write before reading another chunk to bound queued data.
    connect(writer, &OutputWriter::ready, this, [this, writer] {
        if (isFinished()) {
            return;
        }

        const auto chunk = m_reader->read();

        if (!chunk) {
            QString error;

            switch (chunk.error()) {
            case GhosttyQt::HistoryReader::Error::TerminalUnavailable:
                error = i18n("This terminal is no longer available.");
                break;
            case GhosttyQt::HistoryReader::Error::HistoryChanged:
                error = i18n("The terminal history changed while saving output.");
                break;
            case GhosttyQt::HistoryReader::Error::FormattingFailed:
                error = i18n("Could not read terminal output.");
                break;
            }

            QMetaObject::invokeMethod(
                writer,
                [writer, error] {
                    writer->fail(error);
                },
                Qt::QueuedConnection);
            return;
        }

        QMetaObject::invokeMethod(
            writer,
            [writer, chunk = *chunk] {
                writer->write(chunk.bytes, chunk.finished);
            },
            Qt::QueuedConnection);
        setProcessedAmount(Bytes, processedAmount(Bytes) + chunk->bytes.size());
    });

    connect(writer, &OutputWriter::finished, this, [this](const QString &error) {
        if (isFinished()) {
            return;
        }

        if (!error.isEmpty()) {
            setError(UserDefinedError);
            setErrorText(error);
        }

        emitResult();
    });

    thread->start();
}
}

#include "OutputSaveJob.moc"
