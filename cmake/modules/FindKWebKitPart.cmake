# - Try to find KWebKitPart
# Once done this will define
#
#  KWEBKITPART_FOUND - system has KWebKitPart
#  KWEBKITPART_INCLUDE_DIR - the KWebKitPart include directory
#  KWEBKITPART_LIBRARIES - Link these to use KWebKitPart
#  KWEBKITPART_DEFINITIONS - Compiler switches required for using KWebKitPart
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
include(MacroOptionalDependPackage)
if ( KWEBKITPART_INCLUDE_DIR AND KWEBKITPART_LIBRARIES )
   # in cache already
   SET(KWebKitPart_FIND_QUIETLY TRUE)
endif ( KWEBKITPART_INCLUDE_DIR AND KWEBKITPART_LIBRARIES )

# Little trick I found in FindKDE4Interal... If we're building KWebKitPart, set the variables to point to the build directory.
if(kwebkitpart_SOURCE_DIR)
    set(KWEBKITPART_LIBRARIES kwebkit)
    set(KWEBKITPART_INCLUDE_DIR ${CMAKE_SOURCE_DIR})
endif(kwebkitpart_SOURCE_DIR)

FIND_PATH(KWEBKITPART_INCLUDE_DIR NAMES kwebkitpart.h
  PATHS
  ${KDE4_INCLUDE_INSTALL_DIR}
)

FIND_LIBRARY(KWEBKITPART_LIBRARIES NAMES kwebkit
  PATHS
  ${KDE4_LIB_INSTALL_DIR}
)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(KWebKitPart DEFAULT_MSG KWEBKITPART_INCLUDE_DIR KWEBKITPART_LIBRARIES )

# show the KWEBKITPART_INCLUDE_DIR and KWEBKITPART_LIBRARIES variables only in the advanced view
MARK_AS_ADVANCED(KWEBKITPART_INCLUDE_DIR KWEBKITPART_LIBRARIES)
