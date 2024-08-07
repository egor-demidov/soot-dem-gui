cmake_minimum_required(VERSION 3.21)

project(gui_design_soot_dem VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "V8NHL8MF4D")
set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Egor Demidov")
set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Manual")

cmake_policy(SET CMP0060 NEW)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

find_package(VTK COMPONENTS
        CommonCore
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
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        compute_thread.h
        compute_thread.cpp
        aggregation.h
        aggregation.cpp
        icons/icons.qrc
        fonts/fonts.qrc
        icons/app.rc
        restructuring_fixed_fraction.cpp
        restructuring_fixed_fraction.h
        simulation.h
        simulation.cpp
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

set(MISC_SOURCES
    deps/tinyxml2/tinyxml2.cpp
    deps/soot-dem/src/aggregate_stats.cpp
    deps/soot-dem/src/break_neck.cpp
    deps/soot-dem/src/energy.cpp
    deps/soot-dem/src/random_engine.cpp
    deps/soot-dem/src/reader.cpp
    deps/soot-dem/src/writer.cpp
        simulation.cpp
)

if (${APPLE})
    set(MACOS_APP_ICON ${CMAKE_SOURCE_DIR}/icons/app-mac.icns)
    set_source_files_properties(${MACOS_APP_ICON} PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources")
endif ()

add_executable(gui_design_soot_dem MACOSX_BUNDLE ${MACOS_APP_ICON}
    ${PROJECT_SOURCES} ${MISC_SOURCES}
)

target_link_libraries(gui_design_soot_dem PUBLIC Qt${QT_VERSION_MAJOR}::Widgets ${VTK_LIBRARIES})

if (${MSVC})
    set_property(TARGET gui_design_soot_dem PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()

set_target_properties(gui_design_soot_dem PROPERTIES
    MACOSX_BUNDLE_BUNDLE_NAME "soot-dem-gui by Egor Demidov"
    MACOSX_BUNDLE_COPYRIGHT "Copyright (c) 2024 Egor Demidov"
    MACOSX_BUNDLE_GUI_IDENTIFIER soot-dem-gui.edemidov.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE_ICON_FILE app-mac.icns
    MACOSX_BUNDLE_INFO_STRING "GUI for soot-dem project (https://github.com/egor-demidov/gui-design-soot-dem)"
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(gui_design_soot_dem)
endif()

vtk_module_autoinit(
    TARGETS gui_design_soot_dem
    MODULES ${VTK_LIBRARIES}
)

if (${APPLE})
    set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR})

    install(CODE [[
      include(BundleUtilities)
      fixup_bundle("${CMAKE_INSTALL_PREFIX}/gui_design_soot_dem.app" "" "")
    ]] COMPONENT Runtime)

    install(TARGETS gui_design_soot_dem
            BUNDLE  DESTINATION .
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    qt_generate_deploy_app_script(
            TARGET gui_design_soot_dem
            OUTPUT_SCRIPT deploy_script
            NO_UNSUPPORTED_PLATFORM_ERROR
    )

    install(SCRIPT ${deploy_script})
endif ()


#
#include(CPack)
