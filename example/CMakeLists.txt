cmake_minimum_required(VERSION 3.1)
project(sbsmsx VERSION 2.3.0 LANGUAGES C CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

include( TestBigEndian )
test_big_endian( BIGENDIAN )

option(BUILD_SHARED_LIBS "Build dynamic library" OFF)

add_library(sbsmsx 
  src/mp3.cpp
  src/mp3tech.cpp
  src/import.cpp
  src/pcm.cpp
  src/audiobuffer.cpp
  src/convert.cpp
  src/audio.cpp
)

target_compile_definitions(sbsmsx PRIVATE BIGENDIAN=1)
   
include( CheckIncludeFile )
check_include_file(stdint.h HAVE_STDINT_H)
if( HAVE_STDINT_H )
  target_compile_definitions(sbsmsx PRIVATE HAVE_STDINT_H=1)
endif()

find_package( Threads )
if( Threads_FOUND AND CMAKE_USE_PTHREADS_INIT )
   target_compile_definitions(sbsmsx PRIVATE MULTITHREADED=1)
endif()


include(FindPkgConfig)
pkg_search_module(MAD REQUIRED mad)
if( MAD_FOUND )
   target_compile_definitions(sbsmsx PRIVATE HAVE_MAD=1)
   list( APPEND OPTIONS ${MAD_CFLAGS})
endif()

pkg_search_module(SNDFILE REQUIRED sndfile)
if( SNDFILE_FOUND )
   target_compile_definitions(sbsmsx PRIVATE HAVE_SNDFILE=1)
   list( APPEND OPTIONS ${SNDFILE_CFLAGS})
endif()

configure_file( src/config.h.in src/config.h )

list( APPEND OPTIONS
   PRIVATE
      $<$<C_COMPILER_ID:AppleClang,Clang,GNU>:-Wno-enum-compare>
      ${MMX_FLAG}
      ${SSE_FLAG}
)

target_compile_options( sbsmsx PRIVATE ${OPTIONS} )
target_include_directories( sbsmsx PRIVATE ../include )
add_subdirectory("../" ${CMAKE_CURRENT_BINARY_DIR}/libsbsms)
target_link_libraries( sbsmsx PRIVATE sbsms ${MAD_LINK_LIBRARIES} ${SNDFILE_LINK_LIBRARIES} ${PORTAUDIO_LINK_LIBRARIES} )

add_executable( sbsms-convert src/sbsms-convert.cpp )
target_compile_options( sbsms-convert PRIVATE ${OPTIONS} )
target_include_directories( sbsms-convert PRIVATE ../include )
target_link_libraries( sbsms-convert sbsmsx pthread)


if( 0 ) # sbsms-play not included currently
add_executable( sbsms-play src/sbsms-play.cpp src/audio.cpp )
pkg_search_module(PORTAUDIO REQUIRED portaudio-2.0)
if( PORTAUDIO_FOUND )
   target_compile_definitions(sbsms-play PRIVATE HAVE_PORTAUDIO=1)
   list( APPEND OPTIONS ${PORTAUDIO_CFLAGS})
endif()
target_compile_options( sbsms-play PRIVATE ${OPTIONS} )
target_include_directories( sbsms-play PRIVATE ../include )
target_link_options( sbsms-play PRIVATE "-L../" )
target_link_libraries( sbsms-play sbsmsx )
endif()
