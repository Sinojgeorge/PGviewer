cmake_minimum_required(VERSION 3.16)
project(PGViewer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6 (fallback to Qt5)
find_package(Qt6 COMPONENTS Core Widgets Sql Network QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Core Widgets Sql Network REQUIRED)
    set(QT_VERSION_MAJOR 5)
else()
    set(QT_VERSION_MAJOR 6)
endif()

# Find PostgreSQL
find_package(PostgreSQL REQUIRED)

set(SOURCES
    src/main.cpp
    src/core/DatabaseConnection.cpp
    src/core/ConfigManager.cpp
    src/core/QueryResult.cpp
    src/ui/LoginDialog.cpp
    src/ui/MainWindow.cpp
    src/ui/DatabaseExplorer.cpp
    src/ui/TableViewer.cpp
    src/ui/QueryEditor.cpp
    src/ui/CellEditDelegate.cpp
    src/ui/ThemeManager.cpp
    src/ui/StatusBar.cpp
    src/services/DatabaseService.cpp
    src/services/ExportService.cpp
)

set(HEADERS
    src/core/DatabaseConnection.h
    src/core/ConfigManager.h
    src/core/QueryResult.h
    src/ui/LoginDialog.h
    src/ui/MainWindow.h
    src/ui/DatabaseExplorer.h
    src/ui/TableViewer.h
    src/ui/QueryEditor.h
    src/ui/CellEditDelegate.h
    src/ui/ThemeManager.h
    src/ui/StatusBar.h
    src/services/DatabaseService.h
    src/services/ExportService.h
)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resources/resources.qrc")
    set(RESOURCES resources/resources.qrc)
endif()

add_executable(pgviewer
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(pgviewer PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Sql
        Qt6::Network
        PostgreSQL::PostgreSQL
    )
else()
    target_link_libraries(pgviewer PRIVATE
        Qt5::Core
        Qt5::Widgets
        Qt5::Sql
        Qt5::Network
        PostgreSQL::PostgreSQL
    )
endif()

target_include_directories(pgviewer PRIVATE
    src
    ${PostgreSQL_INCLUDE_DIRS}
)

# Platform-specific settings
if(WIN32)
    set_target_properties(pgviewer PROPERTIES WIN32_EXECUTABLE TRUE)
    target_compile_definitions(pgviewer PRIVATE WIN32_LEAN_AND_MEAN)
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(pgviewer PRIVATE dl)
endif()

# Install rules
install(TARGETS pgviewer DESTINATION bin)
