# Finds augeas and its libraries
#
# Distributed under the BSD license. See COPYING-CMAKE-SCRIPTS for details.

find_package(PkgConfig QUIET REQUIRED)
if (AUGEAS_STATIC)
  message("If CMake fails to find Augeas when you are using pl-build-tools, run cmake once more. Seems to be a bug in the pl-cmake setup")
endif (AUGEAS_STATIC)
pkg_check_modules(AUGEAS "augeas" REQUIRED)
set (AUGEAS_DEFINITIONS ${PC_AUGEAS_CFLAGS_OTHER})

find_path (AUGEAS_INCLUDE_DIR augeas.h
  HINTS ${PC_AUGEAS_INCLUDEDIR} ${PC_AUGEAS_INCLUDE_DIRS} )

if (AUGEAS_STATIC)
  find_library (AUGEAS_LIBRARY NAMES libaugeas.a
    HINTS ${PC_AUGEAS_LIBDIR} ${PC_AUGEAS_LIBRARY_DIRS} )
  # This works, but seems pretty kludgy. One of the problems we need to
  # work around is that upstream's augeas.pc is busted and is missing some
  # libraries from the linker flags. The other is that pkg-config wants to
  # statically link against libm, which is pointless as that is part of
  # glibc, and we still link to glibc dynamically even in static builds
  list(REMOVE_ITEM AUGEAS_STATIC_LDFLAGS "-lm")
  set(AUGEAS_LIBRARIES "-Wl,-Bstatic;${AUGEAS_STATIC_LDFLAGS};-lfa;-lselinux;-Wl,-Bdynamic")
else (AUGEAS_STATIC)
  find_library (AUGEAS_LIBRARY NAMES augeas
    HINTS ${PC_AUGEAS_LIBDIR} ${PC_AUGEAS_LIBRARY_DIRS} )
  set(AUGEAS_LIBRARIES ${AUGEAS_LIBRARY} )
endif (AUGEAS_STATIC)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set AUGEAS_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(AUGEAS DEFAULT_MSG
  AUGEAS_LIBRARY AUGEAS_INCLUDE_DIR)

mark_as_advanced(AUGEAS_INCLUDE_DIR AUGEAS_LIBRARY)
