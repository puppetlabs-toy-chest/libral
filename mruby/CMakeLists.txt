set(MRUBY_VERSION "1.3.0")
set(MRUBY "${PROJECT_BINARY_DIR}/mruby/host/bin/mruby")
set(MIRB "${PROJECT_BINARY_DIR}/mruby/host/bin/mirb")

# The source tarball
set(MRUBY_TARBALL "${CMAKE_CURRENT_SOURCE_DIR}/mruby-${MRUBY_VERSION}.tar.gz")
# The directory into which we unpack the source tarball
set(MRUBY_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/mruby-${MRUBY_VERSION}")

set(MRUBY_SOURCE_DIR "${PROJECT_SOURCE_DIR}/mruby")

add_custom_command(COMMENT "Unpack mruby"
  OUTPUT "${MRUBY_SOURCES}"
  DEPENDS "${MRUBY_TARBALL}"
  COMMAND tar xzf "${MRUBY_TARBALL}"
  COMMAND touch "${MRUBY_SOURCES}"
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

add_custom_target(mruby_unpack
  DEPENDS "${MRUBY_SOURCES}")

configure_file("${PROJECT_SOURCE_DIR}/mruby/build_config.rb.in"
  "${PROJECT_BINARY_DIR}/mruby/build_config.rb")

add_custom_command(OUTPUT "${MRUBY}"
  COMMENT "Build mruby"
  DEPENDS "${MRUBY_TARBALL}" "${PROJECT_BINARY_DIR}/mruby/build_config.rb"
  WORKING_DIRECTORY "${MRUBY_SOURCES}"
  COMMAND "rake"
          "MRUBY_BUILD_DIR=${PROJECT_BINARY_DIR}/mruby"
          "INSTALL_DIR=${PROJECT_BINARY_DIR}/bin"
          "MRUBY_CONFIG=${PROJECT_BINARY_DIR}/mruby/build_config.rb")

add_custom_target("mruby" DEPENDS "${MRUBY}")

add_dependencies(mruby mruby_unpack)

add_dependencies(libral mruby)

set(CLEAN_FILES "${PROJECT_BINARY_DIR}/mruby/host"
  "${PROJECT_BINARY_DIR}/bin/mirb"
  "${PROJECT_BINARY_DIR}/bin/mruby"
  "${PROJECT_BINARY_DIR}/bin/mruby-strip"
  "${PROJECT_BINARY_DIR}/bin/mrbc"
  "${PROJECT_BINARY_DIR}/bin/mrdb")

set_directory_properties(PROPERTIES
  ADDITIONAL_MAKE_CLEAN_FILES "${CLEAN_FILES}")

install(PROGRAMS ${MRUBY} ${MIRB} DESTINATION "${LIBRAL_LIBEXEC_DIR}")
