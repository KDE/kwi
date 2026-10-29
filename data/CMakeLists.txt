# SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(ECMInstallIcons)

qt_add_resources(kwi-core kwi_notification_data
    PREFIX "/knotifications6"
    BASE "${CMAKE_CURRENT_SOURCE_DIR}"
    FILES kwi.notifyrc)

set_source_files_properties(icons/sc-apps-org.kde.kwi.svg PROPERTIES
    QT_RESOURCE_ALIAS org.kde.kwi.svg)
set_source_files_properties(icons/index.theme PROPERTIES
    QT_RESOURCE_ALIAS hicolor/index.theme)

foreach(size IN ITEMS 16 22 24 32 48)
    set_source_files_properties(icons/${size}-apps-org.kde.kwi.svg PROPERTIES
        QT_RESOURCE_ALIAS hicolor/${size}x${size}/apps/org.kde.kwi.svg)
endforeach()

qt_add_resources(kwi-core kwi_icon_data
    PREFIX "/icons"
    FILES icons/sc-apps-org.kde.kwi.svg icons/index.theme
        icons/16-apps-org.kde.kwi.svg icons/22-apps-org.kde.kwi.svg
        icons/24-apps-org.kde.kwi.svg icons/32-apps-org.kde.kwi.svg
        icons/48-apps-org.kde.kwi.svg)

configure_file(org.kde.kwi.desktop.in org.kde.kwi.desktop @ONLY)
configure_file(org.kde.kwi.metainfo.xml.in org.kde.kwi.metainfo.xml @ONLY)
install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/org.kde.kwi.desktop" DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/org.kde.kwi.metainfo.xml" DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES kwi.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})

configure_file("${PROJECT_SOURCE_DIR}/logo.png" 256-apps-org.kde.kwi.png COPYONLY)
ecm_install_icons(ICONS
    icons/16-apps-org.kde.kwi.svg
    icons/22-apps-org.kde.kwi.svg
    icons/24-apps-org.kde.kwi.svg
    icons/32-apps-org.kde.kwi.svg
    icons/48-apps-org.kde.kwi.svg
    "${CMAKE_CURRENT_BINARY_DIR}/256-apps-org.kde.kwi.png"
    DESTINATION ${KDE_INSTALL_ICONDIR})

qt_add_resources(kwi kwi_license
    PREFIX "/kwi/licenses"
    BASE "${PROJECT_SOURCE_DIR}/LICENSES"
    FILES "${PROJECT_SOURCE_DIR}/LICENSES/LicenseRef-KDE-Accepted-GPL.txt")
install(DIRECTORY "${PROJECT_SOURCE_DIR}/LICENSES/"
    DESTINATION "${KDE_INSTALL_DATADIR}/licenses/kwi")
