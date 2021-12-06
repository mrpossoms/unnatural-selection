cmake_minimum_required(VERSION 3.10)

project(unnatural-selection VERSION 0.0.1 LANGUAGES CXX)

set(DEFAULT_BUILD_TYPE "Debug")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${DEFAULT_BUILD_TYPE}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# prefer new vendor provided GL implementations
if(POLICY CMP0072)
  cmake_policy (SET CMP0072 NEW)
  set(OpenGL_GL_PREFERENCE GLVND)
endif()

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -Wall -g -O0")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -Wall -g -O0")

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -Wall -O3")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -Wall -O3")


# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# c++ flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGL_SILENCE_DEPRECATION")

# add all source files
set(SOURCE ${SOURCE} src/main.cpp)

# add game includes
include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/renderers)

include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/gitman_sources/g.h/inc)
# include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/gitman_sources/g.h/gitman_sources/opengametools/src)
# include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/gitman_sources/g.h/gitman_sources/xmath.h/inc)

find_package(OpenGL REQUIRED)
find_package(OpenAL REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCE})

add_subdirectory(gitman_sources/g.h)

target_link_libraries(${PROJECT_NAME} g)
target_link_libraries(${PROJECT_NAME} glfw)
target_link_libraries(${PROJECT_NAME} OpenGL::GL)
# target_link_libraries(${PROJECT_NAME} OpenAL::AL)
target_link_libraries(${PROJECT_NAME} ${OPENAL_LIBRARY})
message("openal: ${OPENAL_LIBRARY}")

if (WIN32)
message(STATUS ">>> Will copy data directory")
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
                   COMMAND cmake -E copy_directory
                   ${CMAKE_SOURCE_DIR}/data
                   $<TARGET_FILE_DIR:${PROJECT_NAME}>/data
                  )
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
                   COMMAND cmake -E copy_directory
                   ${CMAKE_SOURCE_DIR}/data
                   ${CMAKE_BINARY_DIR}/data
                  )
else ()
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
                   COMMAND cmake -E create_symlink
                   ${CMAKE_SOURCE_DIR}/data
                   $<TARGET_FILE_DIR:${PROJECT_NAME}>/data
                  )
endif ()

# if (WIN32)
# set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
# endif ()
