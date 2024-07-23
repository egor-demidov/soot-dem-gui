cmake_minimum_required(VERSION 3.5)

project(gui_design_soot_dem VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
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
        icons/icons.qrc
        fonts/fonts.qrc
        fsolve.h
        fsolve.cpp
        parameter_set.h
)

set(CMAKE_NINJA_FORCE_RESPONSE_FILE "ON" CACHE BOOL "Force Ninja to use response files")

add_executable(gui_design_soot_dem
        ${PROJECT_SOURCES}
)

target_link_libraries(gui_design_soot_dem PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${VTK_LIBRARIES})

set_target_properties(gui_design_soot_dem PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(gui_design_soot_dem)
endif()

vtk_module_autoinit(
    TARGETS gui_design_soot_dem
    MODULES ${VTK_LIBRARIES}
)
