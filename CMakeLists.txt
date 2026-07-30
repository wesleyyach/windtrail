cmake_minimum_required(VERSION 3.20)

project(windtrail VERSION 1.0.0 LANGUAGES CXX)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "WindTrail currently supports Linux only.")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(ECM REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets DBus)
find_package(KF6 REQUIRED COMPONENTS CoreAddons Config KCMUtils)
find_package(KWin 6.7 REQUIRED)
find_package(epoxy REQUIRED)

# Native KWin effects must be installed where this Qt 6 build searches
# for plugins. This also avoids distro-specific paths such as lib vs lib64.
find_program(QTPATHS6_EXECUTABLE NAMES qtpaths6 REQUIRED)
execute_process(
    COMMAND "${QTPATHS6_EXECUTABLE}" --plugin-dir
    OUTPUT_VARIABLE WINDTRAIL_QT_PLUGIN_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
)
if(NOT IS_ABSOLUTE "${WINDTRAIL_QT_PLUGIN_DIR}")
    message(FATAL_ERROR "qtpaths6 returned a non-absolute plugin directory: ${WINDTRAIL_QT_PLUGIN_DIR}")
endif()
set(KDE_INSTALL_PLUGINDIR "${WINDTRAIL_QT_PLUGIN_DIR}" CACHE PATH
    "Qt 6 plugin directory used by KWin" FORCE)

add_subdirectory(src)

message(STATUS "WindTrail version: ${PROJECT_VERSION}")
message(STATUS "Qt plugin directory: ${WINDTRAIL_QT_PLUGIN_DIR}")
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
