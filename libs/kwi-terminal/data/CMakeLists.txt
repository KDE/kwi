# SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

# Keep Konsole's GPL palettes as installed data, separate from the LGPL library.
set(kwi_stock_color_schemes
    color-schemes/BlackOnLightYellow.colorscheme
    color-schemes/BlackOnRandomLight.colorscheme
    color-schemes/BlackOnWhite.colorscheme
    color-schemes/BlueOnBlack.colorscheme
    color-schemes/Breeze.colorscheme
    color-schemes/Campbell.colorscheme
    color-schemes/DarkPastels.colorscheme
    color-schemes/GreenOnBlack.colorscheme
    color-schemes/Linux.colorscheme
    color-schemes/RedOnBlack.colorscheme
    color-schemes/Solarized.colorscheme
    color-schemes/SolarizedLight.colorscheme
    color-schemes/WhiteOnBlack.colorscheme)
install(FILES ${kwi_stock_color_schemes}
    DESTINATION ${KDE_INSTALL_DATADIR}/kwi/color-schemes/stock)

qt_add_resources(kwi-terminal kwi_host_process_scripts
    PREFIX "/kwi/process"
    BASE "${CMAKE_CURRENT_SOURCE_DIR}/process"
    FILES
        process/resolve-command.sh
        process/inspect-process.sh
        process/cleanup-process.sh)
