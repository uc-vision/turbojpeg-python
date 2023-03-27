
cmake_minimum_required(VERSION 3.10)
include(ExternalProject)

project(turbojpeg)
ExternalProject_Add(libjpeg-turbo
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/libjpeg-turbo
  STEP_TARGETS build
  EXCLUDE_FROM_ALL TRUE
  INSTALL_COMMAND cmake -E echo "Skipping install step."
)
add_subdirectory(pybind11)

pybind11_add_module(turbojpeg turbojpeg.cc)

target_link_libraries(turbojpeg PUBLIC)
add_dependencies(turbojpeg libjpeg-turbo)
