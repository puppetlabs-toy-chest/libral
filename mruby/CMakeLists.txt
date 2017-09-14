set(MRUBY_VERSION "1.3.0")
set(MRUBY "${PROJECT_BINARY_DIR}/mruby/host/bin/mruby")
set(MIRB "${PROJECT_BINARY_DIR}/mruby/host/bin/mirb")

# The source tarball
set(MRUBY_TARBALL "${CMAKE_CURRENT_SOURCE_DIR}/mruby-${MRUBY_VERSION}.tar.gz")
# The directory into which we unpack the source tarball
set(MRUBY_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/mruby-${MRUBY_VERSION}")

configure_file("${PROJECT_SOURCE_DIR}/mruby/build_setup.rb.in"
  "${PROJECT_BINARY_DIR}/mruby/build_setup.rb")

set(MRUBY_SOURCE_DIR "${PROJECT_SOURCE_DIR}/mruby")
file(GLOB_RECURSE MRUBY_FILES build_config.rb libral.gembox *.rake *.rb *.c *.h)

add_custom_command(COMMENT "Unpack mruby"
  OUTPUT "${MRUBY_SOURCES}"
  DEPENDS "${MRUBY_TARBALL}"
  COMMAND tar xzf "${MRUBY_TARBALL}"
  COMMAND touch "${MRUBY_SOURCES}"
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

add_custom_target(mruby_unpack
  DEPENDS "${MRUBY_SOURCES}")

add_custom_command(OUTPUT "${MRUBY}"
  COMMENT "Build mruby"
  DEPENDS "${MRUBY_FILES}"
  WORKING_DIRECTORY "${MRUBY_SOURCES}"
  COMMAND "rake"
          "MRUBY_BUILD_DIR=${PROJECT_BINARY_DIR}/mruby"
          "INSTALL_DIR=${PROJECT_BINARY_DIR}/bin"
          "MRUBY_CONFIG=${PROJECT_SOURCE_DIR}/mruby/build_config.rb")

add_custom_target("mruby" DEPENDS "${MRUBY}")

add_dependencies(mruby mruby_unpack)

add_dependencies(libral mruby)

install(PROGRAMS ${MRUBY} ${MIRB} DESTINATION "${LIBRAL_LIBEXEC_DIR}")
