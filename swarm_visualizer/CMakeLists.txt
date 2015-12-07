# Tutorial 8 basic shading
FIND_PACKAGE(OpenCV REQUIRED)
FIND_PACKAGE(raspicam REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS}
    ${raspicam_INCLUDE_DIRS}
    include)


add_executable(swarm_visualizer
    tutorial08.cpp
)

target_link_libraries(swarm_visualizer
    common
    ${RPi_LIBS}
    ${GL_LIBS}
    ${OpenCV_LIBS}
    ${raspicam_CV_LIBS}
)
file(
COPY
suzanne.obj
cube-textures.obj
TextureFragmentShader.glsl
TransformVertexShader.glsl
uvtemplate.bmp
DESTINATION ${CMAKE_BINARY_DIR}/swarm_visualizer
)
