// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "PresetEditor.h"
#include "TerminalPresets.h"

#include <KLocalizedString>
#include <KShell>
#include <QVariantMap>

namespace Kwi
{
PresetEditor::PresetEditor(TerminalPresets *presets, QObject *parent)
    : QObject{parent}
    , m_presets{presets}
{
    Q_ASSERT(presets);
    loadValues(presets->defaultPresetName());

    connect(presets, &TerminalPresets::changed, this, [this] {
        if (!m_saving && !isDirty()) {
            const auto selected = m_presets->presetById(m_selectedId);
            loadValues(selected ? selected->name : m_presets->defaultPresetName());
        }
    });
}

QString PresetEditor::selectedName() const
{
    return m_selectedName;
}

QString PresetEditor::name() const
{
    return m_name;
}

QString PresetEditor::command() const
{
    return m_command;
}

int PresetEditor::directoryMode() const
{
    return m_directoryMode;
}

QString PresetEditor::directory() const
{
    return m_directory;
}

QVariantList PresetEditor::environmentEntries() const
{
    QVariantList entries{};

    for (const QString &line : m_environment) {
        const qsizetype separator{line.indexOf(QLatin1Char('='))};
        entries.append(QVariantMap{{QStringLiteral("name"), line.left(separator)}, {QStringLiteral("value"), line.mid(separator + 1)}});
    }

    return entries;
}

QString PresetEditor::errorString() const
{
    return m_error;
}

bool PresetEditor::canRemove() const
{
    const auto preset = m_presets->presetById(m_selectedId);

    return preset && preset->editable;
}

bool PresetEditor::isBuiltin() const
{
    return m_selectedName == TerminalPresets::builtinPresetName();
}

bool PresetEditor::isDirty() const
{
    return m_selectedId.isEmpty() || m_name != m_selectedName || m_command != m_savedCommand || m_directoryMode != m_savedDirectoryMode
        || m_directory != m_savedDirectory || m_environment != m_savedEnvironment;
}

bool PresetEditor::setError(const QString &error)
{
    if (m_error != error) {
        m_error = error;

        Q_EMIT errorChanged();
    }

    return false;
}

void PresetEditor::setName(const QString &newName)
{
    if (m_name != newName) {
        m_name = newName;

        Q_EMIT changed();
    }
}

void PresetEditor::setCommand(const QString &newCommand)
{
    if (m_command != newCommand) {
        m_command = newCommand;

        Q_EMIT changed();
    }
}

void PresetEditor::setDirectoryMode(int mode)
{
    if (mode >= int(LaunchSettings::DirectoryMode::Current) && mode <= int(LaunchSettings::DirectoryMode::Custom) && m_directoryMode != mode) {
        m_directoryMode = mode;

        Q_EMIT changed();
    }
}

void PresetEditor::setDirectory(const QString &newDirectory)
{
    if (m_directory != newDirectory) {
        m_directory = newDirectory;

        Q_EMIT changed();
    }
}

void PresetEditor::setDirectoryUrl(const QUrl &url)
{
    if (url.isLocalFile()) {
        setDirectoryMode(int(LaunchSettings::DirectoryMode::Custom));
        setDirectory(url.toLocalFile());
    }
}

QString PresetEditor::environmentVariableError(int index, const QString &variableName, const QString &value) const
{
    if (variableName.contains(QLatin1Char('=')) || !validateEnvironment({variableName + QLatin1Char('=')})) {
        return i18n("Use letters, numbers and underscores. The name must not start with a number.");
    }

    if (value.contains(QChar::Null) || value.contains(QLatin1Char('\n')) || value.contains(QLatin1Char('\r'))) {
        return i18n("Enter the value on a single line.");
    }

    QStringList assignments{m_environment};

    if (index < -1 || index >= assignments.size()) {
        return i18n("This variable is no longer available.");
    }

    for (qsizetype row{0}; row < assignments.size(); ++row) {
        const QString &entry = assignments.at(row);

        if (row != index && entry.left(entry.indexOf(QLatin1Char('='))) == variableName) {
            return i18n("A variable with this name already exists.");
        }
    }

    if (index == -1) {
        assignments.append(variableName + QLatin1Char('=') + value);
    } else {
        assignments[index] = variableName + QLatin1Char('=') + value;
    }

    QString error{};
    validateEnvironment(assignments, &error);

    return error;
}

bool PresetEditor::setEnvironmentVariable(int index, const QString &variableName, const QString &value)
{
    const QString error{environmentVariableError(index, variableName, value)};

    if (!error.isEmpty()) {
        return setError(error);
    }

    setError({});
    const QString assignment{variableName + QLatin1Char('=') + value};

    if (index == -1) {
        m_environment.append(assignment);
    } else if (m_environment[index] != assignment) {
        m_environment[index] = assignment;
    } else {
        return true;
    }

    Q_EMIT changed();

    return true;
}

void PresetEditor::removeEnvironmentVariable(int index)
{
    if (index < 0 || index >= m_environment.size()) {
        return;
    }

    m_environment.removeAt(index);
    setError({});

    Q_EMIT changed();
}

void PresetEditor::loadValues(const QString &presetName)
{
    const auto preset = m_presets->preset(presetName);

    if (!preset) {
        return;
    }

    m_selectedId = preset->id;
    m_loadedId = preset->id;
    m_selectedName = preset->name;
    m_name = preset->name;
    m_command = KShell::joinArgs(preset->launch.command);
    m_savedCommand = m_command;
    m_directoryMode = int(preset->launch.directoryMode);
    m_savedDirectoryMode = m_directoryMode;
    m_directory = preset->launch.directory;
    m_savedDirectory = m_directory;
    m_environment = preset->launch.environment;
    m_savedEnvironment = m_environment;
    setError({});

    Q_EMIT changed();
}

bool PresetEditor::load(const QString &presetName)
{
    if (isDirty()) {
        return setError(i18n("Save or discard the current preset changes first."));
    }

    if (!m_presets->preset(presetName)) {
        return setError(i18n("The terminal preset is not available."));
    }

    loadValues(presetName);

    return true;
}

QString PresetEditor::uniqueName(const QString &base) const
{
    QString candidateName{base.left(80).trimmed()};
    int suffix{2};

    while (m_presets->preset(candidateName)) {
        const QString number{QStringLiteral(" %1").arg(suffix++)};
        candidateName = base.left(80 - number.size()).trimmed() + number;
    }

    return candidateName;
}

void PresetEditor::beginEdit()
{
    const auto current = m_presets->presetById(m_selectedId);

    if (current && !current->editable && m_name == m_selectedName) {
        const QString label{isBuiltin() ? i18nc("@item terminal preset", "System Shell") : current->name};
        m_name = uniqueName(i18nc("@item terminal preset", "%1 (Modified)", label));

        Q_EMIT changed();
    }
}

void PresetEditor::newCopy()
{
    const QString label{isBuiltin() ? i18nc("@item terminal preset", "System Shell") : m_name};
    m_name = uniqueName(i18nc("@item terminal preset", "Copy of %1", label));
    m_selectedId.clear();
    m_selectedName.clear();
    setError({});

    Q_EMIT changed();
}

bool PresetEditor::save()
{
    LaunchSettings launch{};

    if (!m_command.trimmed().isEmpty()) {
        KShell::Errors error{};
        launch.command = KShell::splitArgs(m_command, KShell::AbortOnMeta, &error);

        if (error != KShell::NoError || launch.command.isEmpty()) {
            return setError(i18n("Enter a program and its arguments. Quote spaces; shell operators and expansions are not supported."));
        }
    }

    launch.directoryMode = static_cast<LaunchSettings::DirectoryMode>(m_directoryMode);
    launch.directory = launch.directoryMode == LaunchSettings::DirectoryMode::Custom ? m_directory : QString{};
    launch.environment = m_environment;
    QString error{};

    if (!validateLaunchSettings(launch, &error)) {
        return setError(error);
    }

    const auto current = m_presets->presetById(m_selectedId);
    const bool detach{current && !current->editable};

    if (detach && m_name == current->name) {
        beginEdit();
    }

    if ((!current || detach) && m_presets->preset(m_name)) {
        return setError(i18n("A terminal preset named %1 already exists.", m_name));
    }

    const bool makeDefault{detach && m_presets->defaultPresetName() == current->name};
    m_saving = true;
    const bool result{current && !detach ? m_presets->updatePreset(current->name, m_name, launch) : m_presets->savePreset(m_name, launch)};
    const bool defaultSaved{result && makeDefault ? m_presets->setDefaultPreset(m_name) : true};
    m_saving = false;

    if (!result) {
        return setError(m_presets->errorString());
    }

    const QString defaultError{m_presets->errorString()};
    loadValues(m_name);

    if (!defaultSaved) {
        return setError(i18n("The preset was saved, but could not be made the default: %1", defaultError));
    }

    return true;
}

bool PresetEditor::removeCurrent()
{
    const auto current = m_presets->presetById(m_selectedId);

    if (!canRemove() || !current) {
        return setError(i18n("Only user-owned terminal presets can be removed."));
    }

    m_saving = true;
    const bool result{m_presets->removePreset(current->name)};
    m_saving = false;

    if (!result) {
        return setError(m_presets->errorString());
    }

    loadValues(m_presets->defaultPresetName());

    return true;
}

void PresetEditor::discard()
{
    // Unsaved copies have no ID, so discarding restores the source preset.
    const auto current = m_presets->presetById(m_selectedId.isEmpty() ? m_loadedId : m_selectedId);
    loadValues(current ? current->name : m_presets->defaultPresetName());
}
}
