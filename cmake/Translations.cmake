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
# Defines targets for lupdate (extract strings) and lrelease (compile .qm)

find_package(Qt6 COMPONENTS LinguistTools QUIET)

if(NOT Qt6_FOUND OR NOT Qt6LinguistTools_FOUND)
    message(STATUS "Qt6 LinguistTools not found; translation targets disabled")
    return()
endif()

# Core translations
set(CORE_TS_FILES
    ${CMAKE_SOURCE_DIR}/i18n/core_en_GB.ts
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

# Create translation source files (.ts) from sources
qt6_add_lupdate(
    SOURCES ${CORE_SOURCES}
    TS_FILES ${CORE_TS_FILES}
    OPTIONS -no-obsolete
    NO_GLOBAL_TARGET
)

# Compile .ts to .qm
qt6_add_lrelease(
    TS_FILES ${CORE_TS_FILES}
    QM_FILES_OUTPUT_VARIABLE CORE_QM_FILES
    OPTIONS -idbased
)

# Install .qm files
install(FILES ${CORE_QM_FILES}
    DESTINATION ${CMAKE_INSTALL_DATADIR}/crankshaft_reborn/i18n
)

# Custom target to update all translations
add_custom_target(translations
    DEPENDS ${CORE_QM_FILES}
    COMMENT "Building translation files"
)

# Per-extension translation support
function(add_extension_translations EXTENSION_NAME EXTENSION_DIR)
    set(EXT_TS_FILE "${EXTENSION_DIR}/i18n/${EXTENSION_NAME}_en_GB.ts")
    
    file(GLOB_RECURSE EXT_SOURCES
        "${EXTENSION_DIR}/*.cpp"
        "${EXTENSION_DIR}/*.hpp"
        "${EXTENSION_DIR}/*.qml"
    )
    
    if(EXT_SOURCES)
        qt6_add_lupdate(
            SOURCES ${EXT_SOURCES}
            TS_FILES ${EXT_TS_FILE}
            OPTIONS -no-obsolete
            NO_GLOBAL_TARGET
        )
        
        qt6_add_lrelease(
            TS_FILES ${EXT_TS_FILE}
            QM_FILES_OUTPUT_VARIABLE EXT_QM_FILES
            OPTIONS -idbased
        )
        
        install(FILES ${EXT_QM_FILES}
            DESTINATION ${CMAKE_INSTALL_DATADIR}/crankshaft_reborn/extensions/${EXTENSION_NAME}/i18n
        )
        
        add_dependencies(translations ${EXTENSION_NAME}Extension)
    endif()
endfunction()

# Register extension translations
add_extension_translations(navigation ${CMAKE_SOURCE_DIR}/extensions/navigation)
add_extension_translations(bluetooth ${CMAKE_SOURCE_DIR}/extensions/bluetooth)
add_extension_translations(media_player ${CMAKE_SOURCE_DIR}/extensions/media_player)
add_extension_translations(dialer ${CMAKE_SOURCE_DIR}/extensions/dialer)
add_extension_translations(wireless ${CMAKE_SOURCE_DIR}/extensions/wireless)

message(STATUS "Translation targets configured: run 'make translations' to build")
