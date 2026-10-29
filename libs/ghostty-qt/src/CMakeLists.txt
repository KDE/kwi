# SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

add_library(ghostty-qt STATIC
    Terminal.cpp Terminal.h
    Frame.h
    Unicode.cpp Unicode.h
    QtInput.cpp QtInput_p.h
    Search.cpp Search.h Search_p.h
    HistoryReader.cpp HistoryReader.h HistoryReader_p.h
)
set_target_properties(ghostty-qt PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(ghostty-qt PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(ghostty-qt PUBLIC Qt6::Core Qt6::Gui PRIVATE Ghostty::Vt)
target_compile_definitions(ghostty-qt PRIVATE QT_NO_KEYWORDS)
