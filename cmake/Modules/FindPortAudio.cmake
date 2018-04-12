
find_path(PORTAUDIO_INCLUDE_DIR 
    portaudio.h
  HINTS 
	${CMAKE_INSTALL_PREFIX}/include 
	${CMAKE_INSTALL_PREFIX}/include/portaudio
	${CMAKE_INSTALL_PREFIX}/include/PortAudio
	$ENV{PORTAUDIO_ROOT}/include 
	$ENV{PORTAUDIO_ROOT}/include/portaudio
	$ENV{PORTAUDIO_ROOT}/include/PortAudio
	/usr/local/include
	/usr/local/include/portaudio
	/usr/local/include/PortAudio
)

set(PORTAUDIO_LIBNAME portaudio)

find_library(PORTAUDIO_LIBRARY 
  NAMES 
	${PORTAUDIO_LIBNAME}
  HINTS 
	${CMAKE_INSTALL_PREFIX}/lib 
	$ENV{PORTAUDIO_ROOT}/lib 
	/usr/local/lib
)

set(PORTAUDIO_LIBRARIES ${PORTAUDIO_LIBRARY})
set(PORTAUDIO_INCLUDE_DIRS ${PORTAUDIO_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBXML2_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(PORTAUDIO  DEFAULT_MSG
                                  PORTAUDIO_INCLUDE_DIR PORTAUDIO_LIBRARY)

if(NOT PORTAUDIO_FOUND)
  set(PORTAUDIO_INCLUDE_DIR $ENV{PORTAUDIO_INCLUDE_DIR} CACHE PATH "Set the directory location of the PORTAUDIO include folder")
  set(PORTAUDIO_LIB_DIR $ENV{PORTAUDIO_LIB_DIR} CACHE PATH "Set the directory location of the PORTAUDIO lib folder")
endif()

mark_as_advanced(PORTAUDIO_FOUND PORTAUDIO_INCLUDE_DIR PORTAUDIO_LIB_DIR PORTAUDIO_LIBRARY)

