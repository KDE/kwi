// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "TerminalSession.h"
#include "settings/TerminalSettings.h"

#include <utility>

namespace Kwi
{
TerminalSession::TerminalSession(TerminalSettings *settings, TerminalPresets *presets, QObject *parent)
    : Session{parent}
    , m_settings{settings}
    , m_presets{presets}
{
    Q_ASSERT(settings && presets);
    refreshPreset();
    setScrollbackLines(settings->scrollbackLines());
    setSilenceSeconds(settings->silenceSeconds());

    connect(settings, &TerminalSettings::scrollbackLinesChanged, this, [this] {
        setScrollbackLines(m_settings->scrollbackLines());
    });
    connect(settings, &TerminalSettings::silenceSecondsChanged, this, [this] {
        setSilenceSeconds(m_settings->silenceSeconds());
    });
    connect(settings, &TerminalSettings::titleFormatChanged, this, &TerminalSession::updateTitleFormat);
    connect(settings, &TerminalSettings::remoteTitleFormatChanged, this, &TerminalSession::updateTitleFormat);
    connect(presets, &TerminalPresets::changed, this, &TerminalSession::refreshPreset);
    connect(this, &Session::sshInfoChanged, this, &TerminalSession::updateTitleFormat);
    connect(this, &Session::exitStatusChanged, this, &TerminalSession::iconNameChanged);
    connect(this, &Session::progressChanged, this, &TerminalSession::iconNameChanged);
    connect(this, &Session::readOnlyChanged, this, &TerminalSession::iconNameChanged);
    connect(this, &Session::monitoringChanged, this, &TerminalSession::iconNameChanged);
    connect(this, &TerminalSession::attentionChanged, this, &TerminalSession::iconNameChanged);
    connect(&m_titleFormatter, &SessionTitleFormatter::textChanged, this, [this] {
        if (m_customTitle.isEmpty()) {
            Q_EMIT titleChanged();
        }
    });

    updateTitleFormat();
    m_titleFormatter.setSession(this);
}

TerminalSettings *TerminalSession::settings() const
{
    return m_settings;
}

QString TerminalSession::currentPresetName() const
{
    return m_preset.name;
}

QString TerminalSession::presetPath() const
{
    return m_preset.path;
}

LaunchSettings TerminalSession::launchSettings() const
{
    return m_preset.launch;
}

bool TerminalSession::setCurrentPreset(const QString &name)
{
    auto preset = m_presets->preset(name);

    if (!preset) {
        return false;
    }

    updatePreset(std::move(*preset));

    return true;
}

void TerminalSession::refreshPreset()
{
    auto preset = m_presets->presetById(m_preset.id);

    if (!preset) {
        preset = m_presets->preset(m_presets->defaultPresetName());
    }

    Q_ASSERT(preset);
    updatePreset(std::move(*preset));
}

void TerminalSession::updatePreset(TerminalPreset preset)
{
    const bool nameChanged{m_preset.name != preset.name};
    m_preset = std::move(preset);

    if (nameChanged) {
        Q_EMIT currentPresetChanged();
    }
}

QString TerminalSession::title() const
{
    return m_customTitle.isEmpty() ? m_titleFormatter.text() : m_customTitle;
}

QString TerminalSession::iconName() const
{
    if (m_attention) {
        return QStringLiteral("notifications");
    }

    if (state() == State::Failed || (!isFinished() && progressState() == ProgressState::Error)) {
        return QStringLiteral("dialog-error");
    }

    if (!isFinished() && progressState() == ProgressState::Paused) {
        return QStringLiteral("media-playback-pause");
    }

    if (isReadOnly()) {
        return QStringLiteral("object-locked");
    }

    if (monitorActivity() || monitorSilence()) {
        return QStringLiteral("notifications-symbolic");
    }

    return isFinished() ? QStringLiteral("dialog-ok-apply") : QStringLiteral("utilities-terminal");
}

void TerminalSession::updateTitleFormat()
{
    m_titleFormatter.setFormat(hasSshInfo() ? m_settings->remoteTitleFormat() : m_settings->titleFormat());
}

QString TerminalSession::customTitle() const
{
    return m_customTitle;
}

void TerminalSession::setCustomTitle(const QString &newTitle)
{
    QString cleaned{newTitle.simplified().left(128)};

    for (QChar &character : cleaned) {
        if (character.category() == QChar::Other_Control || character.isNonCharacter()) {
            character = QLatin1Char(' ');
        }
    }

    if (m_customTitle == cleaned) {
        return;
    }

    m_customTitle = cleaned;

    Q_EMIT titleChanged();
}

bool TerminalSession::attention() const
{
    return m_attention;
}

void TerminalSession::setAttention(bool newAttention)
{
    if (m_attention == newAttention) {
        return;
    }

    m_attention = newAttention;

    Q_EMIT attentionChanged();
}

bool TerminalSession::holdOpen() const
{
    return m_holdOpen;
}

void TerminalSession::setHoldOpen(bool hold)
{
    m_holdOpen = hold;
}
}
