// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "TerminalWidget.h"
#include "TerminalSession.h"
#include "TerminalView.h"

#include <KLocalizedQmlContext>
#include <KLocalizedString>
#include <QDebug>
#include <QEvent>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QMetaObject>
#include <QPointF>
#include <QQmlError>
#include <QQmlFileSelector>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSurfaceFormat>
#include <QVBoxLayout>
#include <QVariant>
#include <QtAlgorithms>
#include <bit>

namespace Kwi
{
TerminalWidget::TerminalWidget(QWidget *parent)
    : QQuickWidget{parent}
{
    setObjectName(QStringLiteral("kwiTerminalWidget"));
    setResizeMode(QQuickWidget::SizeRootObjectToView);

    QSurfaceFormat format{QSurfaceFormat::defaultFormat()};
    format.setAlphaBufferSize(8);
    setFormat(format);

    // Blend with the host's backing store. The host controls window translucency.
    setClearColor(Qt::transparent);

    KLocalization::setupLocalizedContext(engine());
    new QQmlFileSelector{engine(), engine()};

    connect(this, &QQuickWidget::sceneGraphError, this, [this](QQuickWindow::SceneGraphError, const QString &message) {
        showLoadError(message);
    });

    connect(this, &QQuickWidget::statusChanged, this, [this](QQuickWidget::Status status) {
        if (status == QQuickWidget::Error) {
            const auto qmlErrors = errors();
            showLoadError(qJoin(qmlErrors.cbegin(), qmlErrors.cend(), QString{}, QLatin1Char('\n'), &QQmlError::toString));
        }
    });

    loadFromModule(QStringLiteral("org.kde.kwi"), QStringLiteral("EmbeddedTerminal"));

    if (status() != QQuickWidget::Ready || !rootObject()) {
        if (m_loadError.isEmpty()) {
            showLoadError({});
        }

        return;
    }

    m_terminal = rootObject()->property("terminalView").value<TerminalView *>();

    if (!m_terminal) {
        showLoadError(i18n("The terminal view is missing from the interface."));

        return;
    }

    m_terminal->installEventFilter(this);

    connect(m_terminal, &TerminalView::contextMenuRequested, this, [this](QPointF position) {
        Q_EMIT contextMenuRequested(m_terminal->mapToGlobal(position).toPoint(), m_terminal->hoveredLink());
    });

    connect(m_terminal, &TerminalView::multilinePasteRequested, this, &TerminalWidget::multilinePasteRequested);
    connect(m_terminal, &TerminalView::linkActivated, this, &TerminalWidget::linkActivated);
    connect(m_terminal, &TerminalView::focusRequested, this, [this] {
        setFocus(Qt::OtherFocusReason);
    });
}

TerminalView *TerminalWidget::terminalView() const
{
    return m_terminal;
}

QString TerminalWidget::loadError() const
{
    return m_loadError;
}

void TerminalWidget::showLoadError(const QString &message)
{
    m_loadError = i18n("The terminal interface could not be loaded.");

    if (!message.isEmpty()) {
        m_loadError += QLatin1Char('\n') + message;
    }

    qWarning().noquote() << m_loadError;
    auto *label = findChild<QLabel *>(QStringLiteral("kwiTerminalLoadError"));

    if (!label) {
        auto *layout = new QVBoxLayout(this);
        label = new QLabel(this);
        label->setObjectName(QStringLiteral("kwiTerminalLoadError"));
        label->setTextFormat(Qt::PlainText);
        label->setWordWrap(true);
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);
    }

    label->setText(m_loadError);
}

void TerminalWidget::setSession(TerminalSession *session)
{
    if (rootObject()) {
        rootObject()->setProperty("session", QVariant::fromValue(session));
    }
}

void TerminalWidget::setHostError(const QString &message)
{
    if (rootObject()) {
        rootObject()->setProperty("hostError", message);
    }
}

void TerminalWidget::showFind()
{
    setFocus(Qt::ShortcutFocusReason);

    if (rootObject()) {
        QMetaObject::invokeMethod(rootObject(), "showFind");
    }
}

void TerminalWidget::findNext(bool forward)
{
    if (rootObject()) {
        QMetaObject::invokeMethod(rootObject(), "findNext", Q_ARG(bool, forward));
    }
}

void TerminalWidget::focusInEvent(QFocusEvent *focusEvent)
{
    QQuickWidget::focusInEvent(focusEvent);

    if (m_terminal && rootObject() && !rootObject()->property("findVisible").toBool()) {
        m_terminal->forceActiveFocus(focusEvent->reason());
    }
}

bool TerminalWidget::event(QEvent *incomingEvent)
{
    if (handleShortcutOverride(incomingEvent)) {
        return true;
    }

    return QQuickWidget::event(incomingEvent);
}

bool TerminalWidget::eventFilter(QObject *object, QEvent *incomingEvent)
{
    // Quick forwards presses directly to its focus item, bypassing QWidget filters.
    // Check host policy here too to prevent terminal keys from triggering host actions.
    if (object == m_terminal && handleShortcutOverride(incomingEvent)) {
        return true;
    }

    return QQuickWidget::eventFilter(object, incomingEvent);
}

bool TerminalWidget::handleShortcutOverride(QEvent *incomingEvent)
{
    if (incomingEvent->type() == QEvent::ShortcutOverride && m_terminal && m_terminal->hasActiveFocus()) {
        auto *key = static_cast<QKeyEvent *>(incomingEvent);
        const auto modifiers = key->modifiers();

        if (modifiers != Qt::NoModifier && std::popcount(static_cast<unsigned>(modifiers.toInt())) == 1
            && !(modifiers == Qt::ShiftModifier && key->key() == Qt::Key_Insert)) {
            // Negotiate terminal keys with the host, excluding application shortcuts such as Copy.
            bool override{true};

            Q_EMIT overrideShortcut(key, override);

            key->setAccepted(override);

            return true;
        }

        const bool terminalKey{(modifiers == Qt::NoModifier
                                && (key->key() == Qt::Key_Tab || key->key() == Qt::Key_Backspace || key->key() == Qt::Key_Delete || key->key() == Qt::Key_Slash
                                    || key->key() == Qt::Key_Period || key->key() == Qt::Key_Space || key->key() == Qt::Key_Left || key->key() == Qt::Key_Right
                                    || key->key() == Qt::Key_Up || key->key() == Qt::Key_Down || key->key() == Qt::Key_Home || key->key() == Qt::Key_End))};

        if (terminalKey) {
            key->accept();

            return true;
        }
    }

    return false;
}
}
