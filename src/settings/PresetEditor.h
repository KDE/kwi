// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include "LaunchSettings.h"

#include <QObject>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QVariantList>
#include <QtQmlIntegration/qqmlintegration.h>

namespace Kwi
{
class TerminalPresets;

class PresetEditor final : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Preset drafts are owned by their window or settings dialog.")
    Q_PROPERTY(QString selectedName READ selectedName NOTIFY changed)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY changed)
    Q_PROPERTY(QString command READ command WRITE setCommand NOTIFY changed)
    Q_PROPERTY(int directoryMode READ directoryMode WRITE setDirectoryMode NOTIFY changed)
    Q_PROPERTY(QString directory READ directory WRITE setDirectory NOTIFY changed)
    Q_PROPERTY(QVariantList environmentEntries READ environmentEntries NOTIFY changed)
    Q_PROPERTY(bool canRemove READ canRemove NOTIFY changed)
    Q_PROPERTY(bool dirty READ isDirty NOTIFY changed)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorChanged)

public:
    explicit PresetEditor(TerminalPresets *presets, QObject *parent = nullptr);

    [[nodiscard]] QString selectedName() const;
    [[nodiscard]] QString name() const;
    [[nodiscard]] QString command() const;
    [[nodiscard]] int directoryMode() const;
    [[nodiscard]] QString directory() const;
    [[nodiscard]] QVariantList environmentEntries() const;
    [[nodiscard]] bool canRemove() const;
    [[nodiscard]] bool isDirty() const;
    [[nodiscard]] QString errorString() const;

    void setName(const QString &newName);
    void setCommand(const QString &newCommand);
    void setDirectoryMode(int mode);
    void setDirectory(const QString &newDirectory);
    Q_INVOKABLE void setDirectoryUrl(const QUrl &url);

    [[nodiscard]] Q_INVOKABLE QString environmentVariableError(int index, const QString &variableName, const QString &value) const;
    Q_INVOKABLE bool setEnvironmentVariable(int index, const QString &variableName, const QString &value);
    Q_INVOKABLE void removeEnvironmentVariable(int index);

    Q_INVOKABLE bool load(const QString &presetName);
    Q_INVOKABLE void newCopy();
    Q_INVOKABLE bool save();
    Q_INVOKABLE bool removeCurrent();
    Q_INVOKABLE void discard();

Q_SIGNALS:
    void changed();
    void errorChanged();

private:
    void beginEdit();
    [[nodiscard]] bool isBuiltin() const;
    [[nodiscard]] QString uniqueName(const QString &base) const;
    bool setError(const QString &error);
    void loadValues(const QString &presetName);

    TerminalPresets *m_presets;
    QString m_selectedId{};
    QString m_loadedId{};
    QString m_selectedName{};

    QString m_name{};
    QString m_command{};
    QString m_savedCommand{};
    int m_directoryMode{int(LaunchSettings::DirectoryMode::Current)};
    int m_savedDirectoryMode{int(LaunchSettings::DirectoryMode::Current)};
    QString m_directory{};
    QString m_savedDirectory{};
    QStringList m_environment{};
    QStringList m_savedEnvironment{};

    QString m_error{};
    bool m_saving{false};
};
}
