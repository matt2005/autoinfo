# Project: Crankshaft
# This file is part of Crankshaft project.
# Copyright (C) 2025 OpenCarDev Team
#
#  Crankshaft is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 3 of the License, or
#  (at your option) any later version.
#
#  Crankshaft is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with Crankshaft. If not, see <http://www.gnu.org/licenses/>.

# Qt6 translation support for Crankshaft
# Defines targets for:
#  - update_translations: run lupdate across sources updating .ts files
#  - compile_translations: run lrelease compiling all .ts -> .qm
#  - translations (legacy aggregate target) depends on compile_translations

find_package(Qt6 COMPONENTS LinguistTools QUIET)

# Fallback to direct package in case component form isn't registered
if(NOT Qt6LinguistTools_FOUND)
    find_package(Qt6LinguistTools QUIET)
endif()

if(NOT Qt6LinguistTools_FOUND)
    message(STATUS "Qt6 LinguistTools not found; translation targets disabled")
    return()
endif()

find_program(LUPDATE lupdate)
find_program(LRELEASE lrelease)
if(NOT LUPDATE OR NOT LRELEASE)
    message(WARNING "lupdate/lrelease not found; ensure qttools6-dev-tools (or equivalent) is installed")
endif()

# Ensure core i18n directory exists
set(CORE_I18N_DIR "${CMAKE_SOURCE_DIR}/i18n")
if(NOT EXISTS "${CORE_I18N_DIR}")
    file(MAKE_DIRECTORY "${CORE_I18N_DIR}")
endif()

# Core translations
set(CORE_TS_FILES
    ${CORE_I18N_DIR}/core_en_GB.ts
)

set(CORE_SOURCES
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/core/config/ConfigManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/config/ConfigTypes.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/I18nManager.cpp
    ${CMAKE_SOURCE_DIR}/assets/qml/Main.qml
    ${CMAKE_SOURCE_DIR}/assets/qml/ConfigScreen.qml
    ${CMAKE_SOURCE_DIR}/assets/qml/ConfigPageView.qml
    ${CMAKE_SOURCE_DIR}/assets/qml/ConfigItemView.qml
)

# Create translation source files (.ts) from sources and compile to .qm via built-in Qt toolchain
qt6_create_translation(CORE_QM_FILES ${CORE_SOURCES} ${CORE_TS_FILES}
    OPTIONS -no-obsolete
)

# Install .qm files
install(FILES ${CORE_QM_FILES}
    DESTINATION ${CMAKE_INSTALL_DATADIR}/crankshaft_reborn/i18n
)

# Collect all QM files for the translations target
set(ALL_QM_FILES ${CORE_QM_FILES})
set(ALL_TS_FILES ${CORE_TS_FILES})

# Per-extension translation support
function(add_extension_translations EXTENSION_NAME EXTENSION_DIR)
    # Ensure i18n directory exists
    set(I18N_DIR "${EXTENSION_DIR}/i18n")
    if(NOT EXISTS "${I18N_DIR}")
        file(MAKE_DIRECTORY "${I18N_DIR}")
    endif()
    
    set(EXT_TS_FILE "${I18N_DIR}/${EXTENSION_NAME}_en_GB.ts")
    
    file(GLOB_RECURSE EXT_SOURCES
        "${EXTENSION_DIR}/*.cpp"
        "${EXTENSION_DIR}/*.hpp"
        "${EXTENSION_DIR}/*.qml"
    )
    
    if(EXT_SOURCES)
        set(EXT_QM_FILES)
        qt6_create_translation(EXT_QM_FILES ${EXT_SOURCES} ${EXT_TS_FILE}
            OPTIONS -no-obsolete
        )
        
        install(FILES ${EXT_QM_FILES}
            DESTINATION ${CMAKE_INSTALL_DATADIR}/crankshaft_reborn/extensions/${EXTENSION_NAME}/i18n
        )
        
        # Export QM files to parent scope
        set(ALL_QM_FILES ${ALL_QM_FILES} ${EXT_QM_FILES} PARENT_SCOPE)
    endif()

    # Track .ts files even if sources absent (stub for future strings)
    set(ALL_TS_FILES ${ALL_TS_FILES} ${EXT_TS_FILE} PARENT_SCOPE)
endfunction()

# Register extension translations
add_extension_translations(navigation ${CMAKE_SOURCE_DIR}/extensions/navigation)
add_extension_translations(bluetooth ${CMAKE_SOURCE_DIR}/extensions/bluetooth)
add_extension_translations(media_player ${CMAKE_SOURCE_DIR}/extensions/media_player)
add_extension_translations(dialer ${CMAKE_SOURCE_DIR}/extensions/dialer)
add_extension_translations(wireless ${CMAKE_SOURCE_DIR}/extensions/wireless)

if(LUPDATE)
    add_custom_target(update_translations
        COMMAND ${LUPDATE} ${CMAKE_SOURCE_DIR} -recursive -locations relative -no-obsolete -ts ${ALL_TS_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running lupdate to refresh all .ts files"
    )
else()
    add_custom_target(update_translations
        COMMAND ${CMAKE_COMMAND} -E echo "lupdate not found; install qttools6-dev-tools to enable translations extraction"
        COMMENT "Translations: lupdate unavailable"
    )
endif()

if(LRELEASE)
    add_custom_target(compile_translations
        COMMAND ${LRELEASE} ${ALL_TS_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Compiling .ts files to .qm"
    )
else()
    add_custom_target(compile_translations
        COMMAND ${CMAKE_COMMAND} -E echo "lrelease not found; install qttools6-dev-tools to enable translations compilation"
        COMMENT "Translations: lrelease unavailable"
    )
endif()

# Legacy aggregate target retained for existing dependency chain
add_custom_target(translations
    DEPENDS compile_translations
    COMMENT "Building translation files (aggregate target)"
)

# Copy generated QM files into the build tree for development runs (non-installed)
# Generate a small CMake script that copies only the QM files that actually exist.
# This avoids failing the build when lrelease hasn't produced some .qm files yet.
set(COPY_SCRIPT "${CMAKE_BINARY_DIR}/copy_qm_files.cmake")

# Allow verbose logging via cache variable or environment variable.
if(NOT DEFINED TRANSLATIONS_VERBOSE)
    if(DEFINED ENV{TRANSLATIONS_VERBOSE})
        set(TRANSLATIONS_VERBOSE ON)
    else()
        set(TRANSLATIONS_VERBOSE OFF)
    endif()
endif()
set(TRANSLATIONS_VERBOSE "${TRANSLATIONS_VERBOSE}" CACHE BOOL "Enable verbose logging for translation copy script")

file(WRITE "${COPY_SCRIPT}" "cmake_minimum_required(VERSION 3.10)\n")
file(APPEND "${COPY_SCRIPT}" "# Auto-generated by CMake to copy existing .qm files only\n")
file(APPEND "${COPY_SCRIPT}" "set(VERBOSE ${TRANSLATIONS_VERBOSE})\n")
file(APPEND "${COPY_SCRIPT}" "file(MAKE_DIRECTORY \"${CMAKE_BINARY_DIR}/i18n\")\n")

foreach(_qm_file IN LISTS ALL_QM_FILES)
    # Embed each path literally so the script can check existence at build time.
    file(APPEND "${COPY_SCRIPT}" "if(EXISTS \"${_qm_file}\")\n")
    file(APPEND "${COPY_SCRIPT}" "  file(COPY \"${_qm_file}\" DESTINATION \"${CMAKE_BINARY_DIR}/i18n\")\n")
    file(APPEND "${COPY_SCRIPT}" "  if(VERBOSE)\n")
    file(APPEND "${COPY_SCRIPT}" "    message(STATUS \"Translations: copied ${_qm_file}\")\n")
    file(APPEND "${COPY_SCRIPT}" "  endif()\n")
    file(APPEND "${COPY_SCRIPT}" "else()\n")
    file(APPEND "${COPY_SCRIPT}" "  if(VERBOSE)\n")
    file(APPEND "${COPY_SCRIPT}" "    message(STATUS \"Translations: skipping missing ${_qm_file}\")\n")
    file(APPEND "${COPY_SCRIPT}" "  endif()\n")
    file(APPEND "${COPY_SCRIPT}" "endif()\n")
endforeach()

add_custom_command(TARGET translations POST_BUILD
    COMMAND ${CMAKE_COMMAND} -P ${COPY_SCRIPT}
    COMMENT "Copying existing .qm files to build directory i18n/ for runtime use"
)

message(STATUS "Translation targets configured: use 'make update_translations' then 'make compile_translations' (or 'make translations')")
