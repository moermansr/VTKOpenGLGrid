cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(VTKOpenGLGridExample)

find_package(VTK
    9.0
    REQUIRED
    COMPONENTS
        CommonColor
        CommonCore
        CommonDataModel
        CommonMath
        CommonComputationalGeometry
        InteractionWidgets
        InteractionStyle
        RenderingOpenGL2
        RenderingFreeType
        RenderingAnnotation
        GUISupportQt
        IOGeometry
        IOPLY
        IOImage
        IOLegacy)

include_directories(
    ${VTK_INCLUDE_DIRS}
    )


# Prevent a "command line is too long" failure in Windows.
set(CMAKE_NINJA_FORCE_RESPONSE_FILE "ON" CACHE BOOL "Force Ninja to use response files.")
add_executable(VTKOpenGLGridExample MACOSX_BUNDLE 
  main.cpp
  VTKGridMapper.cpp VTKGridMapper.h
)
  target_link_libraries(VTKOpenGLGridExample PRIVATE ${VTK_LIBRARIES}
)
# vtk_module_autoinit is needed
vtk_module_autoinit(
  TARGETS VTKOpenGLGridExample
  MODULES ${VTK_LIBRARIES}
)
