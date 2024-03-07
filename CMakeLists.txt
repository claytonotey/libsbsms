cmake_minimum_required(VERSION 3.5)
project(sbsms VERSION 2.3.0 LANGUAGES CXX)

option(BUILD_SHARED_LIBS "Build dynamic library" ON)

add_library(sbsms
  src/buffer.cpp
  src/dBTable.cpp
  src/fft.cpp
  src/grain.cpp
  src/resample.cpp
  src/sbsms.cpp
  src/slide.cpp
  src/sms.cpp
  src/subband.cpp
  src/track.cpp
  src/trackpoint.cpp
)

target_compile_definitions(sbsms PRIVATE CMAKE)

set_target_properties(sbsms PROPERTIES
  SOVERSION ${PROJECT_VERSION_MAJOR}
  VERSION ${PROJECT_VERSION})

target_include_directories(sbsms PUBLIC
   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
   $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set_target_properties(sbsms PROPERTIES
  WINDOWS_EXPORT_ALL_SYMBOLS TRUE
)

#
# Installation
#

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Library
install(TARGETS sbsms
  EXPORT sbsmsTargets
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
  INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(FILES include/sbsms.h DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

# pkgconfig
set(prefix "${CMAKE_INSTALL_PREFIX}")
set(exec_prefix "\${prefix}")
set(libdir "\${exec_prefix}/${CMAKE_INSTALL_LIBDIR}")
set(includedir "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
set(VERSION "${CMAKE_PROJECT_VERSION}")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sbsms.pc.in
  ${CMAKE_CURRENT_BINARY_DIR}/sbsms.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sbsms.pc DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

# CMake config
set(SBSMS_INSTALL_CMAKEDIR "lib/cmake/sbsms")
install(
  EXPORT sbsmsTargets
  FILE sbsmsTargets.cmake
  NAMESPACE sbsms::
  DESTINATION "${SBSMS_INSTALL_CMAKEDIR}"
)
configure_package_config_file(sbsmsConfig.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/sbsmsConfig.cmake"
  INSTALL_DESTINATION "${SBSMS_INSTALL_CMAKEDIR}"
)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/sbsmsConfigVersion.cmake"
  VERSION "${CMAKE_PROJECT_VERSION}"
  COMPATIBILITY SameMajorVersion
)
install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/sbsmsConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/sbsmsConfigVersion.cmake"
  DESTINATION "${SBSMS_INSTALL_CMAKEDIR}"
)
