// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QColor>
#include <QObject>
#include <QString>
#include <QStringView>
#include <QUrl>
#include <QVariantList>
#include <QVariantMap>
#include <QtQmlIntegration/qqmlintegration.h>

namespace Kwi
{
class TerminalSettings;

class TerminalPaletteModel final : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ColorSchemeModel)
    QML_UNCREATABLE("Color schemes are shared by the application.")
    Q_PROPERTY(QVariantList entries READ entries NOTIFY changed)
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY changed)
    Q_PROPERTY(bool canRemoveCurrent READ canRemoveCurrent NOTIFY changed)
    Q_PROPERTY(bool canEditCurrent READ canEditCurrent NOTIFY changed)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorChanged)

public:
    explicit TerminalPaletteModel(TerminalSettings *settings, QObject *parent = nullptr);

    [[nodiscard]] QVariantList entries() const;
    [[nodiscard]] int currentIndex() const;
    [[nodiscard]] bool canRemoveCurrent() const;
    [[nodiscard]] bool canEditCurrent() const;
    [[nodiscard]] QString errorString() const;

    void refresh();
    Q_INVOKABLE bool importScheme(const QUrl &url);
    Q_INVOKABLE bool removeScheme(const QString &id);

    [[nodiscard]] Q_INVOKABLE QVariantMap schemeData(const QString &id) const;
    [[nodiscard]] Q_INVOKABLE QString editingName(const QString &id, bool createCopy) const;
    Q_INVOKABLE bool saveScheme(const QString &id,
                                const QString &name,
                                const QColor &foreground,
                                const QColor &background,
                                const QVariantList &ansi,
                                const QColor &boldForeground = {});

    Q_INVOKABLE void clearError();

Q_SIGNALS:
    void changed();
    void errorChanged();

private:
    [[nodiscard]] int indexOfScheme(QStringView id) const;
    [[nodiscard]] bool validateName(const QString &id, const QString &name);
    void setError(const QString &error);

    TerminalSettings *m_settings;
    QVariantList m_entries{};
    QString m_error{};
};
}
