cmake_minimum_required(VERSION 3.25)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "")
endif()

string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_ALL_CAPS)

cmake_policy(SET CMP0091 NEW)
if("${BUILD_TYPE_ALL_CAPS}" STREQUAL "MINSIZEREL")
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
else()
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

project(MSVCColorizer LANGUAGES CXX)

string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

add_executable(MSVCColorizer
  src/cmdline.cpp
  src/consolecp.cpp
  src/main.cpp
  src/printlasterror.cpp
  src/processline.cpp
  MSVCColorizer.manifest
)

set_target_properties(MSVCColorizer
  PROPERTIES
    CXX_STANDARD               20
    CXX_EXTENSIONS            OFF
    CXX_STANDARD_REQUIRED      ON
    POSITION_INDEPENDENT_CODE  ON
    EXPORT_COMPILE_COMMANDS    ON
)

target_compile_definitions(MSVCColorizer
  PUBLIC
    -D_WIN32_IE=0x0A00          # _WIN32_IE_WIN10
    -D_WIN32_WINNT=0x0A00       # _WIN32_WINNT_WIN10
    -DNTDDI_VERSION=0x0A00000B  # NTDDI_WIN10_CO
    -DWINVER=0x0A00             # _WIN32_WINNT_WIN10
    -DWIN32_LEAN_AND_MEAN
    -DVC_EXTRALEAN
    -D_HAS_EXCEPTIONS=0
)

if("${BUILD_TYPE_ALL_CAPS}" STREQUAL "DEBUG")
  target_compile_definitions(MSVCColorizer
    PUBLIC
      -D_ITERATOR_DEBUG_LEVEL=2
      -DDEBUG
      -D_DEBUG
  )
  target_link_options(MSVCColorizer
    PRIVATE
      /nodefaultlib:libucrtd.lib
      /defaultlib:ucrtd.lib
  )
else()
  target_compile_definitions(MSVCColorizer
    PUBLIC
      -DNDEBUG
  )
  target_link_options(MSVCColorizer
    PRIVATE
      /nodefaultlib:libucrt.lib
      /defaultlib:ucrt.lib
  )
endif()

target_compile_options(MSVCColorizer
  PRIVATE
    /Zc:__cplusplus
    /Zc:enumTypes
    /Zc:externConstexpr
    /Zc:inline
    /Zc:preprocessor
    /Zc:templateScope
    /Zc:throwingNew
    /volatile:iso
    /utf-8
    /diagnostics:caret
    /EHa-
    /EHs-
    /GR-
    /Wall
    /wd4577 # 'noexcept' used with no exception handling mode specified
    /wd4710 # function not inlined despite inline
    /wd4711 # function inlined despite no inline
)

target_link_options(MSVCColorizer
  PRIVATE
    /DEPENDENTLOADFLAG:0x800
)
