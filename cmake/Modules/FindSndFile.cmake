
find_path(SNDFILE_INCLUDE_DIR 
    sndfile.h
  HINTS 
	${CMAKE_INSTALL_PREFIX}/include 
	${CMAKE_INSTALL_PREFIX}/include/sndfile
	$ENV{SNDFILE_ROOT}/include 
	$ENV{SNDFILE_ROOT}/include/sndfile
	/usr/local/include
	/usr/local/include/sndfile
)

set(SNDFILE_LIBNAME sndfile)

find_library(SNDFILE_LIBRARY 
  NAMES 
	${SNDFILE_LIBNAME}
  HINTS 
	${CMAKE_INSTALL_PREFIX}/lib 
	$ENV{SNDFILE_ROOT}/lib 
	/usr/local/lib
)

set(SNDFILE_LIBRARIES ${SNDFILE_LIBRARY})
set(SNDFILE_INCLUDE_DIRS ${SNDFILE_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBXML2_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(SNDFILE  DEFAULT_MSG
                                  SNDFILE_INCLUDE_DIR SNDFILE_LIBRARY)

if(NOT SNDFILE_FOUND)
  set(SNDFILE_INCLUDE_DIR $ENV{SNDFILE_INCLUDE_DIR} CACHE PATH "Set the directory location of the SNDFILE include folder")
  set(SNDFILE_LIB_DIR $ENV{SNDFILE_LIB_DIR} CACHE PATH "Set the directory location of the SNDFILE lib folder")
endif()

mark_as_advanced(SNDFILE_FOUND SNDFILE_INCLUDE_DIR SNDFILE_LIB_DIR SNDFILE_LIBRARY)

