// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QPointer>
#include <QQuickWidget>
#include <QString>
#include <QUrl>

class QEvent;
class QFocusEvent;
class QKeyEvent;
class QPoint;

namespace Kwi
{
class TerminalSession;
class Session;
class TerminalView;

class TerminalWidget final : public QQuickWidget
{
    Q_OBJECT

public:
    explicit TerminalWidget(QWidget *parent = nullptr);

    [[nodiscard]] TerminalView *terminalView() const;
    [[nodiscard]] QString loadError() const;

    void setSession(TerminalSession *session);
    void setHostError(const QString &message);
    void showFind();
    void findNext(bool forward = true);

Q_SIGNALS:
    void contextMenuRequested(const QPoint &globalPosition, const QUrl &link);
    void multilinePasteRequested(Kwi::Session *origin, const QString &text);
    void linkActivated(const QUrl &url);

    /** NOTE: Direct, synchronous connection only! */
    void overrideShortcut(QKeyEvent *event, bool &override); // clazy:exclude=signal-with-return-value

protected:
    bool event(QEvent *incomingEvent) override;
    bool eventFilter(QObject *object, QEvent *incomingEvent) override;
    void focusInEvent(QFocusEvent *focusEvent) override;

private:
    bool handleShortcutOverride(QEvent *incomingEvent);
    void showLoadError(const QString &message);

    QPointer<TerminalView> m_terminal{};
    QString m_loadError{};
};
}
