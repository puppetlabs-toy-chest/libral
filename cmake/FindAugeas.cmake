# Finds augeas and its libraries
#
# Distributed under the BSD license. See COPYING-CMAKE-SCRIPTS for details.

find_package(PkgConfig QUIET REQUIRED)
pkg_check_modules(AUGEAS "augeas")
set (AUGEAS_DEFINITIONS ${PC_AUGEAS_CFLAGS_OTHER})

find_path (AUGEAS_INCLUDE_DIR augeas.h
  HINTS ${PC_AUGEAS_INCLUDEDIR} ${PC_AUGEAS_INCLUDE_DIRS} )

find_library (AUGEAS_LIBRARY NAMES augeas
  HINTS ${PC_AUGEAS_LIBDIR} ${PC_AUGEAS_LIBRARY_DIRS} )


include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBXML2_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(AUGEAS DEFAULT_MSG
  AUGEAS_LIBRARY AUGEAS_INCLUDE_DIR)

mark_as_advanced(AUGEAS_INCLUDE_DIR AUGEAS_LIBRARY)

set(AUGEAS_LIBRARIES ${AUGEAS_LIBRARY} )
