cmake_minimum_required(VERSION 3.21)

project(gui_design_soot_dem VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

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
        icons/icons.qrc
        fonts/fonts.qrc
        fsolve.h
        fsolve.cpp
        restructuring_fixed_fraction.cpp
        restructuring_fixed_fraction.h
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
    set(CMAKE_CXX_FLAGS "/O2 /openmp /EHsc /GL /fp:excep")
else ()
    set(CMAKE_CXX_FLAGS "-O3 -flto=auto ${CMAKE_CXX_FLAGS}")
endif ()

set(MISC_SOURCES
    deps/tinyxml2/tinyxml2.cpp
    deps/soot-dem/src/aggregate_stats.cpp
    deps/soot-dem/src/break_neck.cpp
    deps/soot-dem/src/energy.cpp
    deps/soot-dem/src/random_engine.cpp
    deps/soot-dem/src/reader.cpp
    deps/soot-dem/src/writer.cpp
)

add_executable(gui_design_soot_dem
    ${PROJECT_SOURCES} ${MISC_SOURCES}
)

target_link_libraries(gui_design_soot_dem PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${VTK_LIBRARIES})

set_target_properties(gui_design_soot_dem PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER soot-dem.edemidov.com
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

add_custom_command(TARGET gui_design_soot_dem POST_BUILD
        COMMAND Qt6::windeployqt
        ARGS $<TARGET_FILE:gui_design_soot_dem>
)

#add_custom_command(TARGET gui_design_soot_dem POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:gui_design_soot_dem> $<TARGET_FILE_DIR:gui_design_soot_dem>
#        COMMAND_EXPAND_LISTS
#)
