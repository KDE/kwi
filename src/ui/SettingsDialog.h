// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <KPageDialog>
#include <QString>

class KPageWidgetItem;
class QCloseEvent;

namespace Kwi
{
class PresetEditor;
class TerminalPaletteModel;
class TerminalPresets;
class TerminalSettings;

class SettingsDialog final : public KPageDialog
{
    Q_OBJECT

public:
    enum class Page {
        Preferences,
        Presets
    };

    static void
    showDialog(QWidget *parent, TerminalSettings *settings, TerminalPresets *presets, const QString &presetName = {}, Page page = Page::Preferences);
    ~SettingsDialog() override;

    void reject() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    SettingsDialog(QWidget *parent, TerminalSettings *settings, TerminalPresets *presets);

    void selectPage(Page page, const QString &presetName);
    bool confirmClose();
    void watchLifetime(QObject *dependency);

    TerminalSettings *m_settings;
    TerminalPresets *m_presets;
    TerminalPaletteModel *m_palettes;
    PresetEditor *m_presetEditor;
    KPageWidgetItem *m_preferencesPage{nullptr};
    KPageWidgetItem *m_presetsPage{nullptr};
    bool m_closing{false};
};
}
