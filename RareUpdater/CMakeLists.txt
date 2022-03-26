cmake_minimum_required(VERSION 3.14)

project(RareUpdater VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS
    Core
    Gui
    Network
    Qml
    Quick
    QuickControls2
    Svg
    Xml
    QuickCompiler
    QmlWorkerScript
    QmlImportScanner
    LinguistTools
    REQUIRED
)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS
    Core
    Gui
    Network
    Qml
    Quick
    QuickControls2
    Svg
    Xml
    QuickCompiler
    QmlWorkerScript
    QmlImportScanner
    LinguistTools
    REQUIRED
)

if(${QT_VERSION_MAJOR} EQUAL 5)
    find_package(QT NAMES Qt5 COMPONENTS Zlib REQUIRED)
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Zlib REQUIRED)
endif()

if(${QT_VERSION_MAJOR} EQUAL 6)
    find_package(QT NAMES Qt6 COMPONENTS Core5Compat REQUIRED)
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core5Compat REQUIRED)
endif()

set(TS_FILES RareUpdater_de_DE.ts)

#set(QUICKDOWNLOAD_AUTO_REGISTER on)

set(PROJECT_SOURCES
    external/QuickDownload/src/quickdownload.cpp
    external/QuickDownload/src/quickdownload.h
#    external/QuickDownload/register_quickdownload.h
    backend.cpp backend.h
    main.cpp
    qml.qrc
    ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(RareUpdater
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET RareUpdater APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(RareUpdater SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(RareUpdater
          ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_compile_definitions(RareUpdater
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(RareUpdater PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::QuickControls2
    Qt${QT_VERSION_MAJOR}::Svg
    Qt${QT_VERSION_MAJOR}::Xml
)

# Only use static macro if linking with static qt
get_target_property(QT_TARGET_TYPE Qt5::Core TYPE)
if(${QT_TARGET_TYPE} STREQUAL "STATIC_LIBRARY")
  include(cmake/QtStaticCMake.cmake)

  qt_generate_plugin_import(RareUpdater VERBOSE)

  # EXTRA_PLUGIN are the one required by plugin loaded by qt_generate_plugin_import
  # It's not automatic yet :( All this workflow might change in future version of qt
  # with better and better cmake support
  qt_generate_qml_plugin_import(RareUpdater
    QML_SRC ${CMAKE_CURRENT_SOURCE_DIR}/qml
    EXTRA_PLUGIN
      QtQuickVirtualKeyboardPlugin
      QtQuickVirtualKeyboardSettingsPlugin
      QtQuickVirtualKeyboardStylesPlugin
      QmlFolderListModelPlugin
      QQuickLayoutsPlugin
    VERBOSE
  )

endif()

# This target needs to be linked after qt_generate_qml_plugin_import call.
# This is hiding a bug somewhere ...
if(TARGET Qt5::QmlWorkerScript)
  target_link_libraries(RareUpdater PRIVATE Qt5::QmlWorkerScript)
endif()

if(${QT_VERSION_MAJOR} EQUAL 5)
    target_link_libraries(RareUpdater PRIVATE Qt${QT_VERSION_MAJOR}::Zlib)
endif()
if(${QT_VERSION_MAJOR} EQUAL 6)
    target_link_libraries(RareUpdater PRIVATE Qt${QT_VERSION_MAJOR}::Core5Compat)
endif()

# OpenSSL stuff
set(OPENSSL_USE_STATIC_LIBS ON)

include(FindOpenSSL)
find_package(OpenSSL REQUIRED)
target_link_libraries(RareUpdater PRIVATE ${CMAKE_DL_LIBS} OpenSSL::SSL OpenSSL::Crypto)

# QuaZip stuff
set(QUAZIP_QT_MAJOR_VERSION ${QT_VERSION_MAJOR})
if(${QT_VERSION_MAJOR} EQUAL 5)
    set(QUAZIP_USE_QT_ZLIB ON)
endif()
set(BUILD_SHARED_LIBS OFF)

add_subdirectory(external/quazip)
target_link_libraries(RareUpdater PRIVATE QuaZip::QuaZip)

set_target_properties(RareUpdater PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

get_target_property(QT_TARGET_TYPE Qt5::Core TYPE)
if(${QT_TARGET_TYPE} STREQUAL "STATIC_LIBRARY")
    find_package(Qt5QmlImportScanner REQUIRED)
    qt5_import_qml_plugins(${CMAKE_PROJECT_NAME})
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_import_qml_plugins(RareUpdater)
    qt_finalize_executable(RareUpdater)
endif()
