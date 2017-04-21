#! /usr/src/libral/build/bin/mruby

# Check for a static build that we only link to DSO's that we are ok with;
# those are the libraries that will be present on any glibc-based system

# The DSO's that it's ok to link to
ALLOWED=%w(linux-vdso.so.1 librt.so.1 libm.so.6 libpthread.so.0
           libc.so.6 libdl.so.2 /lib64/ld-linux-x86-64.so.2)

# Where to find the binaries to check
BINDIR="/usr/src/libral/build/bin"
BINARIES=%w(ralsh mirb mruby)


ok=true
BINARIES.each do |bin|
  %x{ ldd #{BINDIR}/#{bin} }.each_line do |l|
    lib = l.split[0]
    unless ALLOWED.include?(lib)
      $stderr.puts "#{bin} links to #{lib} but shouldn't"
      ok = false
    end
  end
end

exit 1 unless ok
