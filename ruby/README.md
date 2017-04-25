# Libral

This directory contains the CRuby bindings for using libral, and builds the
libral CRuby gem. This gem is needed for provider development, especially
to run tests, but is not needed to actually run libral; unless, of course,
running libral entails talking to it from CRuby code.

The gem should be built with the larger CMake setup of libral. The native
code for the extension will be built by a simple 'make', which eventually
invokes the `ruby_compile` target for this directory. To produce a
packaged gem, you need to explicitly run `make ruby_gem`.

The easiest way to try this gem out when you are building from source is to
run `./bin/dev irb -rlibral` in your build directory.
