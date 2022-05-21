cmake_minimum_required(VERSION 3.21)
project(netDisk)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_PREFIX_PATH $ENV{QTDIR6_MINGW_64})
set(HEADERS
        common.h
        log/log.h
        lock/locker.h
        config.h
        request/HTTPRequest.h
        MainWindow.h
        LoginDialog.h
        components/ClickLabel.h)

set(SOURCES
        main.cpp
        log/log.cpp
        request/HTTPRequest.cpp
        MainWindow.cpp
        LoginDialog.cpp
        components/ClickLabel.cpp)

set(RESOURCES
        mainWindow.ui
        login.ui
        )

set(QRC
        sources/main.qrc
        )

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES("rapidjson/")

if (UNIX)
    add_definitions(-DUNIX)
    target_link_libraries(netDisk pthread)
elseif (WIN32)
    add_definitions(-DWIN32)
    link_libraries(ws2_32 wsock32)
endif ()


find_package(Qt6 REQUIRED COMPONENTS
        Core
        Gui
        Widgets
        Core5Compat
        Quick
        )
QT_WRAP_UI(UI_RESOURCES ${RESOURCES})

add_executable(netDisk
        ${HEADERS}
        ${SOURCES}
        ${RESOURCES}
        ${QRC}
        )

add_executable(test
        test.cpp)

target_link_libraries(netDisk PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Core5Compat
        Qt6::Quick
        )
