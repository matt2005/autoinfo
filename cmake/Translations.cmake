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

# Fallback to direct package in case component form isn't registered
if(NOT Qt6LinguistTools_FOUND)
    find_package(Qt6LinguistTools QUIET)
endif()

if(NOT Qt6LinguistTools_FOUND)
    message(STATUS "Qt6 LinguistTools not found; translation targets disabled")
    return()
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

# Create translation source files (.ts) from sources
qt6_create_translation(CORE_QM_FILES ${CORE_SOURCES} ${CORE_TS_FILES}
    OPTIONS -no-obsolete
)

# Install .qm files
install(FILES ${CORE_QM_FILES}
    DESTINATION ${CMAKE_INSTALL_DATADIR}/crankshaft_reborn/i18n
)

# Collect all QM files for the translations target
set(ALL_QM_FILES ${CORE_QM_FILES})

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
endfunction()

# Register extension translations
add_extension_translations(navigation ${CMAKE_SOURCE_DIR}/extensions/navigation)
add_extension_translations(bluetooth ${CMAKE_SOURCE_DIR}/extensions/bluetooth)
add_extension_translations(media_player ${CMAKE_SOURCE_DIR}/extensions/media_player)
add_extension_translations(dialer ${CMAKE_SOURCE_DIR}/extensions/dialer)
add_extension_translations(wireless ${CMAKE_SOURCE_DIR}/extensions/wireless)

# Custom target to update all translations
add_custom_target(translations
    DEPENDS ${ALL_QM_FILES}
    COMMENT "Building translation files"
)

message(STATUS "Translation targets configured: run 'make translations' to build")
