// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.ki18n
import org.kde.kirigami.forms as Forms
import org.kde.kwi

SettingsPage {
    id: root

    required property AppSettings appSettings
    property bool showWindowSettings: true

    objectName: "behaviorSettings"
    title: KI18n.i18nc("@title:window", "Behavior")
    defaultsVisible: true
    defaultsEnabled: root.appSettings.canResetBehavior

    onRestoreDefaultsRequested: {
        root.appSettings.resetBehaviorDefaults();
        root.appSettings.save();
    }

    Forms.Form {
        Forms.FormGroup {
            title: KI18n.i18nc("@title:group", "Windows")
            visible: root.showWindowSettings

            Forms.FormEntry {
                title: KI18n.i18nc("@label:listbox", "When opened by other applications:")

                contentItem: Controls.ComboBox {
                    objectName: "openNewInstancesInTabsSetting"
                    Layout.fillWidth: true
                    Layout.minimumWidth: 0
                    enabled: !root.appSettings.isOpenNewInstancesInTabsImmutable
                    model: [KI18n.i18nc("@item:inlistbox", "Open a new window"), KI18n.i18nc("@item:inlistbox", "Open a new tab")]
                    currentIndex: root.appSettings.openNewInstancesInTabs ? 1 : 0

                    onActivated: index => {
                        root.appSettings.openNewInstancesInTabs = index === 1;
                        root.appSettings.save();
                    }
                }
            }

            Forms.FormEntry {
                contentItem: Controls.CheckBox {
                    objectName: "useCustomWindowSizeSetting"
                    Layout.fillWidth: true
                    text: KI18n.i18nc("@option:check", "Use custom size for new windows")
                    enabled: !root.appSettings.isUseCustomWindowSizeImmutable
                    checked: root.appSettings.useCustomWindowSize

                    onToggled: {
                        root.appSettings.useCustomWindowSize = checked;
                        root.appSettings.save();
                    }
                }
            }

            Forms.FormEntry {
                title: KI18n.i18nc("@label:spinbox", "Columns:")
                enabled: root.appSettings.useCustomWindowSize && !root.appSettings.isWindowColumnsImmutable

                contentItem: Controls.SpinBox {
                    objectName: "windowColumnsSetting"
                    editable: true
                    from: 2
                    to: 999
                    value: root.appSettings.windowColumns

                    onValueModified: {
                        root.appSettings.windowColumns = value;
                        root.appSettings.save();
                    }
                }
            }

            Forms.FormEntry {
                title: KI18n.i18nc("@label:spinbox", "Rows:")
                enabled: root.appSettings.useCustomWindowSize && !root.appSettings.isWindowRowsImmutable

                contentItem: Controls.SpinBox {
                    objectName: "windowRowsSetting"
                    editable: true
                    from: 1
                    to: 999
                    value: root.appSettings.windowRows

                    onValueModified: {
                        root.appSettings.windowRows = value;
                        root.appSettings.save();
                    }
                }
            }
        }

        Forms.FormGroup {
            title: KI18n.i18nc("@title:group", "Confirmations")

            Forms.FormEntry {
                contentItem: Controls.CheckBox {
                    Layout.fillWidth: true
                    objectName: "confirmCloseRunningSetting"
                    text: KI18n.i18nc("@option:check", "Confirm quit when a command is running")
                    enabled: !root.appSettings.isConfirmCloseRunningImmutable
                    checked: root.appSettings.confirmCloseRunning

                    onToggled: {
                        root.appSettings.confirmCloseRunning = checked;
                        root.appSettings.save();
                    }
                }
            }

            Forms.FormEntry {
                contentItem: Controls.CheckBox {
                    Layout.fillWidth: true
                    objectName: "confirmMultilinePasteSetting"
                    text: KI18n.i18nc("@option:check", "Confirm pasting multiple lines")
                    enabled: !root.appSettings.isConfirmMultilinePasteImmutable
                    checked: root.appSettings.confirmMultilinePaste

                    onToggled: {
                        root.appSettings.confirmMultilinePaste = checked;
                        root.appSettings.save();
                    }
                }
            }
        }

        Forms.FormGroup {
            title: KI18n.i18nc("@title:group", "Clipboard")

            Forms.FormEntry {
                subtitle: KI18n.i18nc("@info", "Middle-click paste also works with this option off.")

                contentItem: Controls.CheckBox {
                    Layout.fillWidth: true
                    objectName: "copyOnSelectSetting"
                    text: KI18n.i18nc("@option:check", "Copy mouse-selected text to clipboard")
                    enabled: !root.appSettings.isCopyOnSelectImmutable
                    checked: root.appSettings.copyOnSelect

                    onToggled: {
                        root.appSettings.copyOnSelect = checked;
                        root.appSettings.save();
                    }
                }
            }
        }

        Forms.FormGroup {
            title: KI18n.i18nc("@title:group", "Links")

            Forms.FormEntry {
                contentItem: Controls.CheckBox {
                    objectName: "requireCtrlForLinksSetting"
                    Layout.fillWidth: true
                    text: KI18n.i18nc("@option:check", "Require Ctrl when clicking links")
                    enabled: !root.appSettings.isRequireCtrlForLinksImmutable
                    checked: root.appSettings.requireCtrlForLinks

                    onToggled: {
                        root.appSettings.requireCtrlForLinks = checked;
                        root.appSettings.save();
                    }
                }
            }
        }
    }
}
