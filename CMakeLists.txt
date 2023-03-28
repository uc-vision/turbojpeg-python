
cmake_minimum_required(VERSION 3.12)
include(ExternalProject)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

project(turbojpeg_python)
ExternalProject_Add(libjpeg-turbo
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/libjpeg-turbo
  STEP_TARGETS build
  CONFIGURE_COMMAND ${CMAKE_COMMAND} -S ${CMAKE_SOURCE_DIR}/libjpeg-turbo -B . -DCMAKE_POSITION_INDEPENDENT_CODE=ON
  EXCLUDE_FROM_ALL TRUE
  INSTALL_COMMAND cmake -E echo "Skipping install step."
)


add_subdirectory(pybind11)

pybind11_add_module(turbojpeg_python turbojpeg.cc)

target_link_libraries(turbojpeg_python PUBLIC ${CMAKE_BINARY_DIR}/libjpeg-turbo-prefix/src/libjpeg-turbo-build/libturbojpeg.a)
target_include_directories(turbojpeg_python PUBLIC ${CMAKE_SOURCE_DIR}/libjpeg-turbo)


add_dependencies(turbojpeg_python libjpeg-turbo)
