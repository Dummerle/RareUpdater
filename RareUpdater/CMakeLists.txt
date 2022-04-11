cmake_minimum_required(VERSION 3.5)

project(RareUpdater VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static -static-libgcc -static-libstdc++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -static-libgcc -static-libstdc++")

find_package(QT NAMES Qt6 Qt5 COMPONENTS
    Widgets
    Network
    LinguistTools
    REQUIRED
)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS
    Widgets
    Network
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

set(TS_FILES
    translations/RareUpdater_de_DE.ts
)

set(PROJECT_SOURCES
    main.cpp
    rareupdater.cpp rareupdater.h
    rareupdater.ui
    uninstalldialog.h uninstalldialog.cpp
    uninstalldialog.ui
    enums.cpp
    ${TS_FILES}
    resources.qrc
    uninstalldialog.h uninstalldialog.cpp
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

target_link_libraries(RareUpdater PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
)

if(${QT_VERSION_MAJOR} EQUAL 5)
    target_link_libraries(RareUpdater PRIVATE Qt${QT_VERSION_MAJOR}::Zlib)
endif()
if(${QT_VERSION_MAJOR} EQUAL 6)
    target_link_libraries(RareUpdater PRIVATE Qt${QT_VERSION_MAJOR}::Core5Compat)
endif()

# OpenSSL stuff
#set(OPENSSL_ROOT_DIR "C:\\Devel\\Qt\\Tools\\OpenSSL\\Win_x64\\lib")
set(OPENSSL_USE_STATIC_LIBS ON)

find_package(OpenSSL)
target_link_libraries(RareUpdater PRIVATE OpenSSL::SSL OpenSSL::Crypto)

# QuaZip stuff
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
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(RareUpdater)
endif()
