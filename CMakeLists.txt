
cmake_minimum_required(VERSION 3.3 FATAL_ERROR)

project(VtkG3D)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(VTK COMPONENTS 
  vtkCommonColor
  vtkCommonCore
  vtkCommonDataModel
  vtkInteractionStyle
  vtkRenderingCore
  vtkRenderingFreeType
  vtkRenderingOpenGL2 QUIET)
if (NOT VTK_FOUND)
  message(SEND_ERROR "VTK NOT FOUND")
endif()
message (STATUS "VTK_VERSION: ${VTK_VERSION}")
if (VTK_VERSION VERSION_LESS "8.90.0")
  # old system
  include(${VTK_USE_FILE})
  add_executable(${PROJECT_NAME} MACOSX_BUNDLE main.cpp Mesh.cpp VolumeVtkImplementation.cpp VtkVolume.cpp VtkNodes.cpp)
  target_link_libraries(${PROJECT_NAME} PRIVATE ${VTK_LIBRARIES})
else ()
  # include all components
  add_executable(${PROJECT_NAME} MACOSX_BUNDLE main.cpp Mesh.cpp VolumeVtkImplementation.cpp VtkVolume.cpp VtkNodes.cpp)
  target_link_libraries(${PROJECT_NAME} PRIVATE ${VTK_LIBRARIES})
  # vtk_module_autoinit is needed
  vtk_module_autoinit(
    TARGETS ${PROJECT_NAME}
    MODULES ${VTK_LIBRARIES}
    )
endif () 
