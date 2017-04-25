# Nutty workaround for the fact that rake-compiler doesn't allow us to set
# ext.config_script to an absolute path. We use this file as far as
# rake-compiler is concerned and then just bounce to the generated file

load(ENV["EXTCONF_CMAKE"])
