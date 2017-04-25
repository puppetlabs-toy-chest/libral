# Building the CRuby gem
set(RUBY_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(RUBY_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

set(RUBY_EXTCONF_IN "${RUBY_SOURCE_DIR}/ext/libral/extconf.rb.in")
set(RUBY_EXTCONF "${RUBY_BINARY_DIR}/ext/libral/extconf.rb")

set(RUBY_RAKEFILE_IN "${RUBY_SOURCE_DIR}/Rakefile.in")
set(RUBY_RAKEFILE "${RUBY_BINARY_DIR}/Rakefile")

# file(GLOB_RECURSE RUBY_FILES libral.gemspec *.rb *.cc *.h)

configure_file("${RUBY_EXTCONF_IN}" "${RUBY_EXTCONF}" @ONLY)
configure_file("${RUBY_RAKEFILE_IN}" "${RUBY_RAKEFILE}" @ONLY)
add_custom_target("ruby_compile" ALL)
add_custom_target("ruby_gem"
  COMMENT "Package the CRuby bindings as a gem")

if (LIBRAL_STATIC)
  add_custom_command(TARGET "ruby_gem"
    COMMENT "Build the CRuby gem with bindings to libral"
    WORKING_DIRECTORY "${RUBY_SOURCE_DIR}"
    COMMAND "echo" "The rubygem can only be build with nonstatic builds \\(LIBRAL_STATIC=OFF\\)")
else (LIBRAL_STATIC)
  add_dependencies(ruby_compile libral_shared)
  add_custom_command(TARGET "ruby_compile"
    WORKING_DIRECTORY "${RUBY_SOURCE_DIR}"
    COMMAND "rake" "-s" "-f" "${RUBY_RAKEFILE}" "compile")
  add_custom_command(TARGET "ruby_gem"
    WORKING_DIRECTORY "${RUBY_SOURCE_DIR}"
    COMMAND "rake" "-s" "-f" "${RUBY_RAKEFILE}" "build")
endif(LIBRAL_STATIC)
