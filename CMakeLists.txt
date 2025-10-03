cmake_minimum_required(VERSION 3.5)
project(SciVis_2025)

set(CMAKE_CXX_STANDARD 17)

set(SRC src)
set(GLAD glad)
set(GLM glm-1.0.1)

set(SOURCES
    ${SRC}/main.cpp
    ${SRC}/quadmesh.cpp
    ${SRC}/drawitem.cpp
    ${SRC}/shader.cpp
    ${SRC}/trackball.cpp
)
add_executable(SciVis_2025 ${SOURCES})

target_include_directories(SciVis_2025 PRIVATE include)
target_include_directories(SciVis_2025 PRIVATE ${GLM})

add_library(glad_lib STATIC ${GLAD}/src/glad.c)
target_include_directories(glad_lib PUBLIC ${GLAD}/include)
target_link_libraries(SciVis_2025 PRIVATE glad_lib)

if(WIN32)

    set(GLFW ${CMAKE_CURRENT_SOURCE_DIR}/glfw-3.4)

    target_include_directories(SciVis_2025 PRIVATE ${GLFW}/include)

    target_link_libraries(SciVis_2025 PRIVATE ${GLFW}/lib-vc2022/glfw3dll.lib)

    set(TO_COPY ${GLFW}/lib-vc2022/glfw3.dll)

    add_custom_command(
        TARGET SciVis_2025 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${TO_COPY}
            ${CMAKE_CURRENT_BINARY_DIR}
    )

else()

    find_package(glfw3 CONFIG REQUIRED)
    target_link_libraries(SciVis_2025 PRIVATE glfw)

    find_package(OpenGL REQUIRED)
    target_link_libraries(SciVis_2025 PRIVATE OpenGL::GL)
    
    if(APPLE) # Silence OpenGL deprecation warnings on macOS
        add_definitions(-DGL_SILENCE_DEPRECATION)
    endif()

endif()
