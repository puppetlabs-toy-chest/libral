set(MRUBY "${PROJECT_BINARY_DIR}/mruby/host/bin/mruby")

configure_file("${PROJECT_SOURCE_DIR}/mruby/build_setup.rb.in"
  "${PROJECT_BINARY_DIR}/mruby/build_setup.rb")

set(MRUBY_SOURCE_DIR "${PROJECT_SOURCE_DIR}/mruby")
file(GLOB_RECURSE MRUBY_FILES build_config.rb libral.gembox *.rake *.rb *.c *.h)

add_custom_command(OUTPUT "${MRUBY}"
  COMMENT "Build mruby"
  DEPENDS "${MRUBY_FILES}"
  WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/mruby/core"
  COMMAND "rake"
          "MRUBY_BUILD_DIR=${PROJECT_BINARY_DIR}/mruby"
          "INSTALL_DIR=${PROJECT_BINARY_DIR}/bin"
          "MRUBY_CONFIG=${PROJECT_SOURCE_DIR}/mruby/build_config.rb")

add_custom_target("mruby" DEPENDS "${MRUBY}")

add_dependencies(libral mruby)
