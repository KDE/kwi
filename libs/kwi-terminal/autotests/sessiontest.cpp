// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Session.h"
#include "ProcessTestSupport.h"

#include <QByteArray>
#include <QChar>
#include <QDir>
#include <QLatin1Char>
#include <QObject>
#include <QSignalSpy>
#include <QString>
#include <QStringLiteral>
#include <QTemporaryDir>
#include <QTest>
#include <QUrl>
#include <csignal>

using namespace Kwi;

class SessionTest : public QObject
{
    Q_OBJECT

public:
    using QObject::QObject;

private Q_SLOTS:
    void alternateScreenTracksOutputAndReset();
    void outputReceivedReflectsTerminalContents();
    void outputHandlerCanCloseSession();
    void sanitizesApplicationTitles();
    void repeatedStartPreservesRunningSession();
    void processTermination_data();
    void processTermination();
    void restartClearsProcessMetadataAndRetainsHistory_data();
    void restartClearsProcessMetadataAndRetainsHistory();
};

// Sessions report screen switches and return to the main screen after a reset.
void SessionTest::alternateScreenTracksOutputAndReset()
{
    Session session;
    QSignalSpy changed{&session, &Session::alternateScreenChanged};
    QVERIFY(!session.alternateScreen());
    session.feed("plain text");
    QCOMPARE(changed.count(), 0);

    for (const auto mode : {47, 1047, 1049}) {
        const QByteArray prefix{QByteArray("\x1b[?") + QByteArray::number(mode)};
        session.feed(prefix + 'h');
        QVERIFY(session.alternateScreen());
        QCOMPARE(changed.count(), 1);
        session.feed(prefix + 'h');
        QCOMPARE(changed.count(), 1);
        session.feed(prefix + 'l');
        QVERIFY(!session.alternateScreen());
        QCOMPARE(changed.count(), 2);
        changed.clear();
    }

    session.feed("\x1b[?1049h");
    session.resetTerminal();
    QVERIFY(!session.alternateScreen());
    QCOMPARE(changed.count(), 2);
    session.resetTerminal();
    QCOMPARE(changed.count(), 2);
    session.feed("\x1b[?1049h");
    session.feed("\033c");
    QVERIFY(!session.alternateScreen());
    QCOMPARE(changed.count(), 4);
}

// Output handlers see updated terminal contents without enabling activity monitoring.
void SessionTest::outputReceivedReflectsTerminalContents()
{
    Session session;
    QSignalSpy output{&session, &Session::outputReceived};
    QSignalSpy activity{&session, &Session::activityDetected};
    QString lastOutput;
    int progress{-1};
    connect(&session, &Session::outputReceived, this, [&] {
        lastOutput = session.historyText();
        progress = session.progressPercent();
    });

    session.feed(QByteArrayLiteral("\x1b]9;4;1;42\afirst\r\n"));
    QCOMPARE(output.count(), 1);
    QVERIFY(lastOutput.contains(QStringLiteral("first")));
    QCOMPARE(progress, 42);
    QCOMPARE(activity.count(), 0);

    session.feed({});
    session.resize(40, 10);
    session.setZoomOffset(1);
    session.scrollPrompt(-1);
    session.clearScrollback();
    session.resetTerminal();
    QCOMPARE(output.count(), 1);

    session.feed(QByteArrayLiteral("second\r\n"));
    QCOMPARE(output.count(), 2);
    QVERIFY(lastOutput.contains(QStringLiteral("second")));
    session.close();
    session.feed(QByteArrayLiteral("after close\r\n"));
    QCOMPARE(output.count(), 2);
}

// Closing from an output handler leaves the session finished with monitoring disabled.
void SessionTest::outputHandlerCanCloseSession()
{
    Session session;
    session.setMonitorActivity(true);
    session.setMonitorSilence(true);
    connect(&session, &Session::outputReceived, &session, &Session::close);
    session.feed(QByteArrayLiteral("finished\r\n"));
    QVERIFY(session.isFinished());
    QVERIFY(!session.monitorActivity());
    QVERIFY(!session.monitorSilence());
}

// Application titles remove control characters and remain bounded without splitting a surrogate pair.
void SessionTest::sanitizesApplicationTitles()
{
    Session session;
    const QString unsafe = QStringLiteral("unsafe") + QChar{0x85} + QChar{0xfdd0} + QChar{0x7f} + QString(300, QLatin1Char('a'));
    session.feed(QByteArray("\x1b]2;") + unsafe.toUtf8() + QByteArray("\x1b\\"));
    QVERIFY(session.applicationTitle().startsWith(QStringLiteral("unsafe")));
    QCOMPARE(session.applicationTitle().size(), 256);
    const QString title{session.applicationTitle()};

    for (const QChar character : title) {
        QVERIFY(character.category() != QChar::Other_Control);
        QVERIFY(!character.isNonCharacter());
    }

    const QString longTitle{QString(255, QLatin1Char('a')) + QStringLiteral("😀")};
    session.feed(QByteArray("\x1b]2;") + longTitle.toUtf8() + QByteArray("\x07"));
    QCOMPARE(session.applicationTitle(), QString(255, QLatin1Char('a')));
}

// A second start request leaves the running child and its input channel intact.
void SessionTest::repeatedStartPreservesRunningSession()
{
    Session session;
    QVERIFY(session.start(
        testLaunch(QStringLiteral("/bin/sh"),
                   {QStringLiteral("-c"), QStringLiteral("stty -echo; printf 'ready\\n'; read -r line; printf 'received:%s\\n' \"$line\"; read -r ack")},
                   {},
                   {QStringLiteral("ENV="), QStringLiteral("BASH_ENV=")})));
    QTRY_VERIFY(session.historyText().contains(QStringLiteral("ready")));
    const auto pid{session.processId()};
    QVERIFY(pid > 0);

    QVERIFY(!session.start(
        testLaunch(QStringLiteral("/bin/sh"), {QStringLiteral("-c"), QStringLiteral("exit 99")}, {}, {QStringLiteral("ENV="), QStringLiteral("BASH_ENV=")})));
    QCOMPARE(session.processId(), pid);
    QVERIFY(session.isActive());
    QVERIFY(session.sendInput(QStringLiteral("still-running\n")));
    QTRY_VERIFY(session.historyText().contains(QStringLiteral("received:still-running")));
    QVERIFY(session.sendInput(QStringLiteral("finish\n")));
    QTRY_VERIFY(session.isFinished());
    QCOMPARE(session.exitCode(), 0);
}

void SessionTest::processTermination_data()
{
    QTest::addColumn<QString>("command");
    QTest::addColumn<int>("exitCode");
    QTest::addColumn<bool>("crashed");

    QTest::newRow("success") << QStringLiteral("exit 0") << 0 << false;
    QTest::newRow("failure") << QStringLiteral("exit 1") << 1 << false;
    QTest::newRow("exit-134") << QStringLiteral("exit 134") << 134 << false;
    QTest::newRow("exit-137") << QStringLiteral("exit 137") << 137 << false;
    QTest::newRow("abort") << QStringLiteral("kill -ABRT $$") << 128 + SIGABRT << true;
    QTest::newRow("kill") << QStringLiteral("kill -KILL $$") << 128 + SIGKILL << true;
    QTest::newRow("terminate") << QStringLiteral("kill -TERM $$") << 128 + SIGTERM << true;
}

// Process termination retains output and distinguishes crashes from normal exits.
void SessionTest::processTermination()
{
    QFETCH(QString, command);
    QFETCH(int, exitCode);
    QFETCH(bool, crashed);
    Session session;
    QSignalSpy exited{&session, &Session::exited};
    connect(&session, &Session::exited, this, [&](int code) {
        QCOMPARE(code, exitCode);
        QCOMPARE(session.exitCode(), exitCode);
        QCOMPARE(session.state(), crashed ? Session::State::Failed : Session::State::Exited);
        QCOMPARE(session.errorString().isEmpty(), !crashed);
        QVERIFY(session.historyText().contains(QStringLiteral("final-output")));
        QVERIFY(!session.isActive());
        QVERIFY(!session.monitorActivity());
        QVERIFY(!session.monitorSilence());
    });
    QVERIFY(session.start(testLaunch(QStringLiteral("/bin/sh"),
                                     {QStringLiteral("-c"), QStringLiteral("ulimit -c 0; printf 'final-output\\n'; read -r ack; ") + command},
                                     {},
                                     {QStringLiteral("ENV="), QStringLiteral("BASH_ENV=")})));
    // Receive the output before the child revokes its FreeBSD controlling TTY.
    QTRY_VERIFY(session.historyText().contains(QStringLiteral("final-output")));
    session.setMonitorActivity(true);
    session.setMonitorSilence(true);
    QVERIFY(session.sendInput(QStringLiteral("\n")));
    QTRY_COMPARE(exited.size(), 1);
}

void SessionTest::restartClearsProcessMetadataAndRetainsHistory_data()
{
    QTest::addColumn<bool>("crash");
    QTest::newRow("normal-exit") << false;
    QTest::newRow("crash") << true;
}

// Restarting a session clears errors and process metadata while preserving terminal history.
void SessionTest::restartClearsProcessMetadataAndRetainsHistory()
{
    QFETCH(bool, crash);
    Session session;
    QTemporaryDir firstDirectory{QDir{QDir::tempPath()}.canonicalPath() + QStringLiteral("/sessiontest-XXXXXX")};
    QTemporaryDir secondDirectory{QDir{QDir::tempPath()}.canonicalPath() + QStringLiteral("/sessiontest-XXXXXX")};
    QVERIFY(firstDirectory.isValid());
    QVERIFY(secondDirectory.isValid());

    const QString firstUrl{QString::fromUtf8(QUrl::fromLocalFile(firstDirectory.path()).toEncoded())};
    const QString firstCommand{
        QStringLiteral("ulimit -c 0; printf '\\033]7;%s\\007\\033]2;First process\\007\\033]9;4;1;42\\007old-process\\n' \"$1\"; IFS= read -r line; ")
        + (crash ? QStringLiteral("kill -ABRT $$") : QStringLiteral("exit 0"))};
    QVERIFY(session.start(testLaunch(QStringLiteral("/bin/sh"),
                                     {QStringLiteral("-c"), firstCommand, QStringLiteral("test"), firstUrl},
                                     firstDirectory.path(),
                                     {QStringLiteral("ENV="), QStringLiteral("BASH_ENV=")})));
    QTRY_VERIFY(session.historyText().contains(QStringLiteral("old-process")));
    QCOMPARE(session.workingDirectory(), firstDirectory.path());
    QCOMPARE(session.applicationTitle(), QStringLiteral("First process"));
    QCOMPARE(session.progressPercent(), 42);
    QVERIFY(session.sendInput(QStringLiteral("\n")));
    QTRY_VERIFY(session.isFinished());
    QCOMPARE(session.exitCode(), crash ? 128 + SIGABRT : 0);
    QCOMPARE(session.state(), crash ? Session::State::Failed : Session::State::Exited);
    QCOMPARE(session.errorString().isEmpty(), !crash);
    QCOMPARE(session.progressState(), Session::ProgressState::None);
    QCOMPARE(session.progressPercent(), -1);

    QVERIFY(session.start(testLaunch(QStringLiteral("cat"), {}, secondDirectory.path())));
    QCOMPARE(session.progressState(), Session::ProgressState::None);
    QCOMPARE(session.progressPercent(), -1);
    QVERIFY(!session.hasApplicationTitle());
    QVERIFY(session.applicationTitle().isEmpty());
    QTRY_COMPARE(session.state(), Session::State::Running);
    QVERIFY(session.errorString().isEmpty());
    QCOMPARE(session.launchedProgramName(), QStringLiteral("cat"));
    QCOMPARE(session.workingDirectory(), secondDirectory.path());
    QVERIFY(session.sendInput(QStringLiteral("second-process\n")));
    QTRY_VERIFY(session.historyText().contains(QStringLiteral("second-process")));
    QVERIFY(session.historyText().contains(QStringLiteral("old-process")));
    QCOMPARE(session.workingDirectory(), secondDirectory.path());
    QCOMPARE(session.progressState(), Session::ProgressState::None);
    QCOMPARE(session.progressPercent(), -1);
    QVERIFY(!session.hasApplicationTitle());
    QVERIFY(session.applicationTitle().isEmpty());
    session.close();
    QTRY_VERIFY(session.isFinished());
    QCOMPARE(session.state(), Session::State::Exited);
    QVERIFY(session.errorString().isEmpty());
}

QTEST_MAIN(SessionTest)

#include "sessiontest.moc"
