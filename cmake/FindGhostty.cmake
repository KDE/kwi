# SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

#[=======================================================================[.rst:
FindGhostty
-----------

Find the public libghostty-vt C API installed by Ghostty.

This module defines the imported target ``Ghostty::Vt`` and the result
variables ``Ghostty_FOUND`` and ``Ghostty_VERSION``. It prefers Ghostty's
installed CMake package and otherwise uses ``libghostty-vt.pc`` from a Zig
installation. Use ``CMAKE_PREFIX_PATH`` or ``Ghostty_ROOT`` to locate a prefix,
``ghostty-vt_DIR`` to select a CMake package, or ``PKG_CONFIG_PATH`` for pkg-config.

Shared linkage is the default. Set ``Ghostty_USE_STATIC_LIBS`` to true for
static linkage.
#]=======================================================================]

block(SCOPE_FOR VARIABLES PROPAGATE Ghostty_FOUND Ghostty_VERSION)
    set(_Ghostty_target Ghostty::Vt)

    if(NOT TARGET Ghostty::Vt)
        set(_Ghostty_target "")
        set(Ghostty_VERSION "")

        # FindPkgConfig uses CMAKE_PREFIX_PATH, but not package root hints.
        # Scope these additions to this module, including environment hints.
        set(_Ghostty_roots ${Ghostty_ROOT} ${GHOSTTY_ROOT})
        foreach(_Ghostty_root IN ITEMS Ghostty_ROOT GHOSTTY_ROOT)
            cmake_path(CONVERT "$ENV{${_Ghostty_root}}" TO_CMAKE_PATH_LIST _Ghostty_env_roots)
            list(APPEND _Ghostty_roots ${_Ghostty_env_roots})
        endforeach()
        list(PREPEND CMAKE_PREFIX_PATH ${_Ghostty_roots})

        find_package(ghostty-vt QUIET CONFIG)

        if(ghostty-vt_FOUND)
            set(Ghostty_VERSION "${ghostty-vt_VERSION}")
            set(_Ghostty_target ghostty-vt::ghostty-vt)

            if(Ghostty_USE_STATIC_LIBS)
                set(_Ghostty_target ghostty-vt::ghostty-vt-static)
            endif()
        else()
            find_package(PkgConfig QUIET)

            if(PkgConfig_FOUND)
                pkg_check_modules(PC_Ghostty QUIET IMPORTED_TARGET libghostty-vt)
            endif()

            if(PkgConfig_FOUND AND PC_Ghostty_FOUND)
                set(Ghostty_VERSION "${PC_Ghostty_VERSION}")
                set(_Ghostty_target PkgConfig::PC_Ghostty)

                if(Ghostty_USE_STATIC_LIBS)
                    # --static supplies private dependencies but does not select
                    # the archive. Find it only in this package's library directory.
                    find_library(_Ghostty_archive
                        NAMES "${CMAKE_STATIC_LIBRARY_PREFIX}ghostty-vt${CMAKE_STATIC_LIBRARY_SUFFIX}"
                        PATHS "${PC_Ghostty_LIBDIR}"
                        NO_DEFAULT_PATH NO_CACHE
                    )
                    set(_Ghostty_target "")

                    if(_Ghostty_archive)
                        set(_Ghostty_dependencies ${PC_Ghostty_STATIC_LIBRARIES})
                        list(REMOVE_ITEM _Ghostty_dependencies ghostty-vt)

                        if(NOT TARGET Ghostty::_Static)
                            add_library(Ghostty::_Static STATIC IMPORTED)
                        endif()

                        set_target_properties(Ghostty::_Static PROPERTIES
                            IMPORTED_LOCATION "${_Ghostty_archive}"
                            INTERFACE_INCLUDE_DIRECTORIES "${PC_Ghostty_STATIC_INCLUDE_DIRS}"
                            INTERFACE_COMPILE_OPTIONS "${PC_Ghostty_STATIC_CFLAGS_OTHER}"
                            INTERFACE_LINK_LIBRARIES "${_Ghostty_dependencies}"
                            INTERFACE_LINK_DIRECTORIES "${PC_Ghostty_STATIC_LIBRARY_DIRS}"
                            INTERFACE_LINK_OPTIONS "${PC_Ghostty_STATIC_LDFLAGS_OTHER}"
                        )
                        set(_Ghostty_target Ghostty::_Static)
                    endif()
                endif()
            endif()
        endif()
    endif()

    include(CMakePushCheckState)
    include(CheckCSourceCompiles)

    cmake_push_check_state(RESET)
    set(CMAKE_REQUIRED_LIBRARIES "${_Ghostty_target}")
    set(CMAKE_REQUIRED_QUIET "${Ghostty_FIND_QUIETLY}")

    unset(Ghostty_HAS_REQUIRED_API CACHE)

    if(_Ghostty_target)
        check_c_source_compiles([=[
        #include <ghostty/vt.h>
        typedef void (*ApiFunction)(void);
        static ApiFunction volatile required[] = {
            (ApiFunction)ghostty_render_state_image_get_multi,
            (ApiFunction)ghostty_render_state_image_iterator_new,
            (ApiFunction)ghostty_search_get_multi,
            (ApiFunction)ghostty_search_feed,
            (ApiFunction)ghostty_search_tick,
            (ApiFunction)ghostty_selection_gesture_event,
            (ApiFunction)ghostty_terminal_selection_adjust,
            (ApiFunction)ghostty_kitty_graphics_animation_tick,
            (ApiFunction)ghostty_unicode_grapheme_width,
            (ApiFunction)ghostty_color_perceived_luminance,
            (ApiFunction)ghostty_color_scheme_report_encode,
        };
        int main(void) {
            GhosttyTerminalOption history_limit = GHOSTTY_TERMINAL_OPT_SCROLLBACK_MAX_LINES;
            GhosttyTerminalOption pwd_changed = GHOSTTY_TERMINAL_OPT_PWD_CHANGED;
            return required[0] == 0 || history_limit == GHOSTTY_TERMINAL_OPT_SCROLLBACK_MAX_BYTES || pwd_changed == history_limit;
        }
        ]=] Ghostty_HAS_REQUIRED_API)

        if(NOT Ghostty_HAS_REQUIRED_API)
            string(CONCAT _Ghostty_failure
                "The selected libghostty-vt lacks APIs required by Kwi or cannot be linked. "
                "Install a compatible development version. "
                "See the configure log for the failed compile/link check."
            )
        endif()
    else()
        set(Ghostty_HAS_REQUIRED_API FALSE)
        string(CONCAT _Ghostty_failure
            "Install libghostty-vt with its CMake package or pkg-config metadata. "
            "The pkg-config route also requires the pkg-config executable. "
            "Set CMAKE_PREFIX_PATH or Ghostty_ROOT to its prefix. "
            "Static linkage also requires the archive."
        )
    endif()

    cmake_pop_check_state()

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(Ghostty
        REQUIRED_VARS _Ghostty_target Ghostty_HAS_REQUIRED_API
        VERSION_VAR Ghostty_VERSION
        HANDLE_VERSION_RANGE
        REASON_FAILURE_MESSAGE "${_Ghostty_failure}"
    )

    if(Ghostty_FOUND AND NOT TARGET Ghostty::Vt)
        add_library(Ghostty::Vt INTERFACE IMPORTED)
        target_link_libraries(Ghostty::Vt INTERFACE "${_Ghostty_target}")
    endif()
endblock()
