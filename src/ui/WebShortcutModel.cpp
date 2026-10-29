// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "WebShortcutModel.h"
#include "WebShortcuts_p.h"

#include <KIO/CommandLauncherJob>
#include <KLocalizedString>
#include <KStringHandler>
#include <KUriFilter>
#include <QStandardPaths>

namespace Kwi
{
namespace
{
constexpr int iconNameRole{Qt::UserRole + 1};
}

WebShortcutModel::WebShortcutModel(QObject *parent)
    : QAbstractListModel{parent}
{
}

int WebShortcutModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : count();
}

QVariant WebShortcutModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid)) {
        return {};
    }

    const auto &provider = m_providers.at(index.row());

    if (role == Qt::DisplayRole) {
        return provider.name;
    }

    if (role == iconNameRole) {
        return provider.iconName;
    }

    return {};
}

QHash<int, QByteArray> WebShortcutModel::roleNames() const
{
    return {{Qt::DisplayRole, "providerName"}, {iconNameRole, "iconName"}};
}

int WebShortcutModel::count() const
{
    return m_providers.size();
}

QString WebShortcutModel::title() const
{
    return m_title;
}

bool WebShortcutModel::canConfigure() const
{
    return !QStandardPaths::findExecutable(QStringLiteral("kcmshell6")).isEmpty();
}

void WebShortcutModel::refresh(const QString &selectedText)
{
    beginResetModel();
    m_providers.clear();
    m_title.clear();

    const QString text = selectedText.simplified();

    if (!text.isEmpty() && webShortcutsEnabled()) {
        KUriFilterData filter{text};
        filter.setSearchFilteringOptions(KUriFilterData::RetrievePreferredSearchProvidersOnly);

        if (KUriFilter::self()->filterSearchUri(filter, KUriFilter::NormalTextFilter)) {
            const auto providers = filter.preferredSearchProviders();
            m_providers.reserve(providers.size());

            for (const auto &provider : providers) {
                // Append provider records directly without transform's lambda and output iterator.
                // cppcheck-suppress useStlAlgorithm
                m_providers.append({provider, filter.iconNameForPreferredSearchProvider(provider), filter.queryForPreferredSearchProvider(provider)});
            }

            // BUG: qqc2-desktop-style can render selected "<b>...</b>" as markup.
            // Fix upstream in Kirigami's literal-text-to-mnemonic rendering path.
            // Escaping this title alone is not reliable with style auto-detection.
            m_title = i18nc("@title:menu", "Search for '%1' with", KStringHandler::rsqueeze(text, 21).replace(u'&', QStringLiteral("&&")));
        }
    }

    endResetModel();
    Q_EMIT refreshed();
}

QUrl WebShortcutModel::searchUrl(int row) const
{
    if (row < 0 || row >= count()) {
        return {};
    }

    KUriFilterData filter{m_providers.at(row).query};

    // Restrict filtering to search providers to avoid interpreting text as a command or path.
    if (KUriFilter::self()->filterSearchUri(filter, KUriFilter::WebShortcutFilter)) {
        return filter.uri();
    }

    return {};
}

void WebShortcutModel::configure()
{
    auto *job = new KIO::CommandLauncherJob{QStringLiteral("kcmshell6"), {QStringLiteral("webshortcuts")}, this};
    job->setUiDelegate(nullptr);

    connect(job, &KJob::result, this, [this](KJob *finished) {
        if (finished->error()) {
            Q_EMIT configurationError(finished->errorString());
        }
    });

    job->start();
}
}
