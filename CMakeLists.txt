cmake_minimum_required(VERSION 3.25)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "")
endif()

string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_ALL_CAPS)

cmake_policy(SET CMP0091 NEW)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project(cpp_playground LANGUAGES CXX)

add_executable(MSVCColorizer main.cpp MSVCColorizer.manifest)

set_target_properties(MSVCColorizer
  PROPERTIES
    CXX_STANDARD               20
    CXX_EXTENSIONS            OFF
    CXX_STANDARD_REQUIRED      ON
    POSITION_INDEPENDENT_CODE  ON
)

target_compile_definitions(MSVCColorizer
  PUBLIC
    -DWIN32_LEAN_AND_MEAN
    -DVC_EXTRALEAN
    -D_WIN32_IE=0x0A00          # _WIN32_IE_WIN10
    -D_WIN32_WINNT=0x0A00       # _WIN32_WINNT_WIN10
    -DNTDDI_VERSION=0x0A00000B  # NTDDI_WIN10_CO
    -DWINVER=0x0A00             # _WIN32_WINNT_WIN10
)

if("${BUILD_TYPE_ALL_CAPS}" STREQUAL "DEBUG")
  target_compile_definitions(MSVCColorizer
    PUBLIC
      -D_ITERATOR_DEBUG_LEVEL=2
      -DDEBUG
      -D_DEBUG
  )
elseif("${BUILD_TYPE_ALL_CAPS}" STREQUAL "RELEASE")
  target_compile_definitions(MSVCColorizer
    PUBLIC
      -DNDEBUG
  )
endif()

target_compile_options(MSVCColorizer
  PRIVATE
    /Zc:__cplusplus
    /utf-8
    /Wall
    /diagnostics:caret
)

target_link_libraries(MSVCColorizer
  PUBLIC
    Shell32
)
