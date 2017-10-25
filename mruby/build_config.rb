begin
  build_setup="#{ENV["MRUBY_BUILD_DIR"]}/build_setup.rb"
  load build_setup
rescue LoadError
  puts "WARNING: #{build_setup} does not exist. Assuming default toolchain and options will work"
  # Dummy definitions of functions that build_setup.rb would provide
  def libral_configure_mruby_augeas(gem); end
  def libral_configure_mirb(gem); end
  def libral_configure_toolchain(conf); end
end

MRuby::Build.new do |conf|
  toolchain :gcc

  enable_cxx_abi

  enable_debug

  # the gembox path is relative to core/mrbgems
  conf.gembox "#{Libral::SOURCE_DIR}/mruby/libral"

  libral_configure_toolchain(conf)

  # C compiler settings
  conf.cc do |cc|
    cc.flags.flatten!
    cc.flags << "-fPIC"
    cc.flags.reject! { |x| ["-std=gnu99", "-Wdeclaration-after-statement"].include?(x) }
  end
end
