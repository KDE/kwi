// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include "settings/LaunchSettings.h"
#include "settings/TerminalPresets.h"

#include <QString>
#include <QtQmlIntegration/qqmlintegration.h>
#include <Session.h>
#include <SessionTitleFormatter.h>

Q_MOC_INCLUDE("settings/TerminalSettings.h")

namespace Kwi
{
class TerminalSettings;

class TerminalSession final : public Session
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Sessions are owned by their session model.")
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QString iconName READ iconName NOTIFY iconNameChanged)
    Q_PROPERTY(Kwi::TerminalSettings *settings READ settings CONSTANT)
    Q_PROPERTY(QString currentPresetName READ currentPresetName WRITE setCurrentPreset NOTIFY currentPresetChanged)
    Q_PROPERTY(QString customTitle READ customTitle WRITE setCustomTitle NOTIFY titleChanged)
    Q_PROPERTY(bool attention READ attention WRITE setAttention NOTIFY attentionChanged)

public:
    explicit TerminalSession(TerminalSettings *settings, TerminalPresets *presets, QObject *parent = nullptr);

    [[nodiscard]] TerminalSettings *settings() const;
    [[nodiscard]] QString currentPresetName() const;
    [[nodiscard]] QString presetPath() const;
    [[nodiscard]] LaunchSettings launchSettings() const;
    bool setCurrentPreset(const QString &name);
    [[nodiscard]] QString title() const;
    [[nodiscard]] QString iconName() const;
    [[nodiscard]] QString customTitle() const;
    void setCustomTitle(const QString &newTitle);
    [[nodiscard]] bool attention() const;
    void setAttention(bool newAttention);
    [[nodiscard]] bool holdOpen() const;
    void setHoldOpen(bool hold);

Q_SIGNALS:
    void titleChanged();
    void iconNameChanged();
    void attentionChanged();
    void currentPresetChanged();

private:
    void refreshPreset();
    void updatePreset(TerminalPreset preset);
    void updateTitleFormat();

    TerminalSettings *m_settings;
    TerminalPresets *m_presets;
    TerminalPreset m_preset{};
    SessionTitleFormatter m_titleFormatter;
    QString m_customTitle;
    bool m_attention{false};
    bool m_holdOpen{false};
};
}
