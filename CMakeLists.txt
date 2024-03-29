cmake_minimum_required(VERSION 3.14)

set(CMAKE_DEBUG_POSTFIX "-d") # append -d to debug targets so they can be
                              # installed alongside release ones

include(cmake/prelude.cmake)

project(
  veg
  VERSION 0.1.0
  DESCRIPTION "c++ vocabulary types and utilities"
  HOMEPAGE_URL "https://example.com/"
  LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(
  veg_veg
  source/veg.cpp
  source/ub.cpp
  source/parse.cpp
  source/time.cpp
)
add_library(veg::veg ALIAS veg_veg)

include(GenerateExportHeader)
generate_export_header(
  veg_veg
  BASE_NAME
  veg
  EXPORT_FILE_NAME
  export/veg/veg_export.hpp
  CUSTOM_CONTENT_FROM_VARIABLE
  pragma_suppress_c4251
)

if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(veg_veg PUBLIC VEG_STATIC_DEFINE)
endif()

set_target_properties(
  veg_veg
  PROPERTIES VERSION "${PROJECT_VERSION}"
             SOVERSION "${PROJECT_VERSION_MAJOR}"
             EXPORT_NAME veg
             OUTPUT_NAME veg
)

target_include_directories(
  veg_veg ${warning_guard}
  PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_include_directories(
  veg_veg SYSTEM PUBLIC "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>"
)

target_compile_features(veg_veg PUBLIC cxx_std_11)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Examples ----

if(PROJECT_IS_TOP_LEVEL)
  option(BUILD_EXAMPLES "Build examples tree." "${veg_DEVELOPER_MODE}")
  if(BUILD_EXAMPLES)
    add_subdirectory(example)
  endif()
endif()

# ---- Developer mode ----

if(NOT veg_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(AUTHOR_WARNING "Developer mode is intended for developers of veg")
endif()

find_package(fmt REQUIRED)
target_link_libraries(veg_veg PRIVATE fmt::fmt)

include(cmake/dev-mode.cmake)
