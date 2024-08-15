cmake_minimum_required(VERSION 3.21)

project(soot_dem_gui VERSION 0.2 LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if (${APPLE})
    set(CMAKE_C_COMPILER "/opt/homebrew/opt/llvm/bin/clang")
    set(CMAKE_CXX_COMPILER "/opt/homebrew/opt/llvm/bin/clang++")
endif ()

cmake_policy(SET CMP0060 NEW)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
if (${QT_VERSION_MAJOR} STREQUAL 6)
    qt_standard_project_setup()
endif ()

find_package(VTK COMPONENTS
        CommonCore
        CommonColor
        CommonDataModel
        FiltersSources
        GUISupportQt
        InteractionStyle
        RenderingContextOpenGL2
        RenderingCore
        RenderingFreeType
        RenderingGL2PSOpenGL2
        RenderingOpenGL2
        GUISupportQt
        RenderingQt
)

if(NOT VTK_FOUND)
    message(FATAL_ERROR "Unable to find VTK")
endif()

if(NOT(TARGET VTK::GUISupportQt))
    message(FATAL_ERROR "VTK not built with Qt support")
endif()

if(NOT DEFINED VTK_QT_VERSION)
    set(VTK_QT_VERSION 5)
endif()

set(PROJECT_SOURCES
        src/main.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        src/mainwindow.ui
        src/aboutdialog.ui
        src/aboutdialog.h
        src/aboutdialog.cpp
        src/compute_thread.h
        src/compute_thread.cpp
        src/aggregation.h
        src/aggregation.cpp
        src/restructuring_fixed_fraction.cpp
        src/restructuring_fixed_fraction.h
        src/aggregate_deposition.cpp
        src/aggregate_deposition.h
        src/simulation.h
        src/simulation.cpp
        src/config.h
        src/config.cpp
        src/exceptions.h
        fonts/fonts.qrc
        icons/icons.qrc
        icons/app.rc
)

add_compile_definitions(LIBGRAN_USE_OMP)
add_compile_definitions(_USE_MATH_DEFINES)

include_directories(deps/eigen)
include_directories(deps/libgran/include)
include_directories(deps/libtimestep/include)
include_directories(deps/soot-dem/src)
include_directories(deps)

set(CMAKE_NINJA_FORCE_RESPONSE_FILE "ON" CACHE BOOL "Force Ninja to use response files")

if (${MSVC})
    set(CMAKE_CXX_FLAGS "/O2 /openmp /EHsc /GL")
else ()
    set(CMAKE_CXX_FLAGS "-O3 -flto=auto -fopenmp ${CMAKE_CXX_FLAGS}")
endif ()

#include_directories(dummy)

set(MISC_SOURCES
    deps/tinyxml2/tinyxml2.cpp
    deps/soot-dem/src/aggregate_stats.cpp
    deps/soot-dem/src/break_neck.cpp
    deps/soot-dem/src/energy.cpp
    deps/soot-dem/src/random_engine.cpp
    deps/soot-dem/src/reader.cpp
    deps/soot-dem/src/writer.cpp
)

if (${APPLE})
    set(MACOS_APP_ICON ${CMAKE_SOURCE_DIR}/icons/app-mac.icns)
    set_source_files_properties(${MACOS_APP_ICON} PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources")
endif ()

add_executable(soot_dem_gui MACOSX_BUNDLE ${MACOS_APP_ICON}
    ${PROJECT_SOURCES} ${MISC_SOURCES}
)

target_link_libraries(soot_dem_gui PUBLIC Qt${QT_VERSION_MAJOR}::Widgets ${VTK_LIBRARIES})

if (${MSVC})
    set_property(TARGET soot_dem_gui PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()

set_target_properties(soot_dem_gui PROPERTIES
    MACOSX_BUNDLE_BUNDLE_NAME "soot-dem-gui by Egor Demidov"
    MACOSX_BUNDLE_COPYRIGHT "Copyright (c) 2024 Egor Demidov"
    MACOSX_BUNDLE_GUI_IDENTIFIER com.edemidov.soot-dem-gui
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE_ICON_FILE app-mac.icns
    MACOSX_BUNDLE_INFO_STRING "GUI for soot-dem project (https://github.com/egor-demidov/gui-design-soot-dem)"
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(soot_dem_gui)
endif()

vtk_module_autoinit(
    TARGETS soot_dem_gui
    MODULES ${VTK_LIBRARIES}
)

if (${APPLE})
    set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR})

    install(CODE [[
      include(BundleUtilities)
      fixup_bundle("${CMAKE_INSTALL_PREFIX}/soot_dem_gui.app" "" "")
    ]] COMPONENT Runtime)

    install(TARGETS soot_dem_gui
            BUNDLE  DESTINATION .
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    qt_generate_deploy_app_script(
            TARGET soot_dem_gui
            OUTPUT_SCRIPT deploy_script
            NO_UNSUPPORTED_PLATFORM_ERROR
    )

    install(SCRIPT ${deploy_script})
endif ()


#
#include(CPack)
