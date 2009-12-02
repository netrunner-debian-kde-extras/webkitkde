# - Try to find WebKitPart
# Once done this will define
#
#  WEBKITPART_FOUND - system has WebKitPart
#  WEBKITPART_INCLUDE_DIR - the WebKitPart include directory
#  WEBKITPART_LIBRARIES - Link these to use WebKitPart
#  WEBKITPART_DEFINITIONS - Compiler switches required for using WebKitPart
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if ( WEBKITPART_INCLUDE_DIR AND WEBKITPART_LIBRARIES )
   # in cache already
   SET(WebKitPart_FIND_QUIETLY TRUE)
endif ( WEBKITPART_INCLUDE_DIR AND WEBKITPART_LIBRARIES )

# Little trick I found in FindKDE4Interal... If we're building WebKitPart, set the variables to point to the build directory.
if(webkitpart_SOURCE_DIR)
    set(WEBKITPART_LIBRARIES webkitkpart)
    set(WEBKITPART_INCLUDE_DIR ${CMAKE_SOURCE_DIR})
endif(webkitpart_SOURCE_DIR)

# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
if( NOT WIN32 )
  find_package(PkgConfig)

  pkg_check_modules(PC_WEBKITPART kwebkitpart)

  set(PCRE_DEFINITIONS ${PC_WEBKITPART_CFLAGS_OTHER})
endif( NOT WIN32 )

FIND_PATH(WEBKITPART_INCLUDE_DIR NAMES webkitpart.h
  PATHS
  ${PC_WEBKITPART_INCLUDEDIR} 
  ${PC_WEBKITPART_INCLUDE_DIRS}
  ${KDE4_INCLUDE_INSTALL_DIR}
  PATH_SUFFIXES webkitkde
)

FIND_LIBRARY(WEBKITPART_LIBRARIES NAMES kwebkit
  PATHS
  ${PC_WEBKITPART_LIBDIR} 
  ${PC_WEBKITPART_LIBRARY_DIRS}
  ${KDE4_LIB_INSTALL_DIR}
)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(WebKitPart DEFAULT_MSG WEBKITPART_INCLUDE_DIR WEBKITPART_LIBRARIES )

# show the WEBKITPART_INCLUDE_DIR and WEBKITPART_LIBRARIES variables only in the advanced view
MARK_AS_ADVANCED(WEBKITPART_INCLUDE_DIR WEBKITPART_LIBRARIES)

