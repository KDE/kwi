// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Session.h"
#include "TerminalView.h"

#include <KLocalizedString>
#include <QByteArray>
#include <QCoreApplication>
#include <QEvent>
#include <QFile>
#include <QGuiApplication>
#include <QHoverEvent>
#include <QIODevice>
#include <QLatin1Char>
#include <QMouseEvent>
#include <QObject>
#include <QPointF>
#include <QQuickWindow>
#include <QRectF>
#include <QSignalSpy>
#include <QSizeF>
#include <QString>
#include <QStringLiteral>
#include <QTemporaryDir>
#include <QTest>
#include <QUrl>
#include <QVariant>
#include <QVariantMap>
#include <QtQuickTest/quicktest.h>

using namespace Kwi;

class TerminalLinksTest : public QObject
{
    Q_OBJECT

public:
    using QObject::QObject;

private Q_SLOTS:
    void fileLinks_data();
    void fileLinks();
};

// File links resolve to the expected paths.
void TerminalLinksTest::fileLinks_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<bool>("exists");
    QTest::addColumn<bool>("absolute");
    QTest::addColumn<bool>("hasDirectory");

    QTest::newRow("relative-file") << QStringLiteral("example.cpp") << QStringLiteral("%1") << true << false << true;
    QTest::newRow("relative-location") << QStringLiteral("example.cpp") << QStringLiteral("%1:12:3") << true << false << true;
    QTest::newRow("missing-relative-file") << QStringLiteral("missing.cpp") << QStringLiteral("%1") << false << false << true;
    QTest::newRow("missing-relative-location") << QStringLiteral("missing.cpp") << QStringLiteral("%1:12:3") << false << false << true;
    QTest::newRow("colon-file") << QStringLiteral(":example.cpp") << QStringLiteral("%1") << true << false << true;
    QTest::newRow("colon-location") << QStringLiteral(":example.cpp") << QStringLiteral("%1:12:3") << true << false << true;
    QTest::newRow("quoted-colon-file") << QStringLiteral(":example file.cpp") << QStringLiteral("\"%1\"") << true << false << true;
    QTest::newRow("missing-colon-file") << QStringLiteral(":missing.cpp") << QStringLiteral("%1") << false << false << true;
    QTest::newRow("quoted-missing-colon-file") << QStringLiteral(":missing file.cpp") << QStringLiteral("'%1'") << false << false << true;
    QTest::newRow("resource-path") << QStringLiteral(":/qt/qml/missing.qml") << QStringLiteral("%1") << false << false << true;
    QTest::newRow("colon-without-cwd") << QStringLiteral(":missing.cpp") << QStringLiteral("%1") << false << false << false;
    QTest::newRow("missing-absolute-file") << QStringLiteral("missing.cpp") << QStringLiteral("%1") << false << true << true;
    QTest::newRow("missing-absolute-location") << QStringLiteral("missing.cpp") << QStringLiteral("%1:12:3") << false << true << true;
    QTest::newRow("quoted-absolute-file") << QStringLiteral("missing file.cpp") << QStringLiteral("\"%1\"") << false << true << true;
    QTest::newRow("absolute-without-cwd") << QStringLiteral("missing.cpp") << QStringLiteral("%1") << false << true << false;
    QTest::newRow("ascii-art") << QStringLiteral(":cccccccccccccc;MMM.;cccccccccccccccc:") << QStringLiteral("%1") << false << false << true;
}

void TerminalLinksTest::fileLinks()
{
    QFETCH(QString, name);
    QFETCH(QString, pattern);
    QFETCH(bool, exists);
    QFETCH(bool, absolute);
    QFETCH(bool, hasDirectory);

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path{directory.path() + QLatin1Char('/') + name};

    if (exists) {
        QFile file{path};
        QVERIFY(file.open(QIODevice::WriteOnly));
    }

    Session session;
    QQuickWindow window;
    TerminalView view{window.contentItem()};
    window.resize(1280, 400);
    view.setSize(QSizeF(window.size()));
    view.setSession(&session);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    QVERIFY(QQuickTest::qWaitForPolish(&view));
    const QRectF cell{view.inputMethodQuery(Qt::ImCursorRectangle).toRectF()};

    if (hasDirectory) {
        session.feed(QByteArray("\x1b]7;") + QUrl::fromLocalFile(directory.path()).toEncoded() + QByteArray("\x1b\\"));
        QCOMPARE(session.workingDirectory(), directory.path());
    } else {
        QVERIFY(session.workingDirectory().isEmpty());
    }

    session.feed(pattern.arg(absolute ? path : name).toUtf8());
    QVERIFY(QQuickTest::qWaitForPolish(&view));
    const QUrl expected{absolute || (exists && hasDirectory) ? QUrl::fromLocalFile(path) : QUrl()};
    const auto links = view.links();
    QCOMPARE(links.size(), expected.isEmpty() ? 0 : 1);

    if (!expected.isEmpty()) {
        QCOMPARE(links.first().toMap().value(QStringLiteral("url")).toUrl(), expected);
    }

    const QPointF position{cell.topLeft() + QPointF(2.5 * cell.width(), 0.5 * cell.height())};
    QHoverEvent hover{QEvent::HoverMove, position, position, position, Qt::ControlModifier};
    QCoreApplication::sendEvent(&view, &hover);
    QCOMPARE(view.hoveredLink(), expected);
    QSignalSpy activated{&view, &TerminalView::linkActivated};
    QMouseEvent press{QEvent::MouseButtonPress, position, position, Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier};
    QCoreApplication::sendEvent(&view, &press);
    QMouseEvent release{QEvent::MouseButtonRelease, position, position, Qt::LeftButton, Qt::NoButton, Qt::ControlModifier};
    QCoreApplication::sendEvent(&view, &release);
    QCOMPARE(activated.size(), expected.isEmpty() ? 0 : 1);

    if (!expected.isEmpty()) {
        QCOMPARE(activated.first().first().toUrl(), expected);
    }
}

int main(int argc, char **argv)
{
    QTemporaryDir configuration;

    if (!configuration.isValid()) {
        return 1;
    }

    qputenv("XDG_CONFIG_HOME", QFile::encodeName(configuration.path()));
    QGuiApplication application(argc, argv);
    KLocalizedString::setApplicationDomain("kwi");
    TerminalLinksTest test;

    return QTest::qExec(&test, argc, argv);
}

#include "terminallinkstest.moc"
