MRuby::Gem::Specification.new('mruby-libral') do |spec|
  spec.license = 'MIT'
  spec.author  = 'lutter'
  spec.summary = 'Libral special sauce'

  spec.rbfiles = Dir.glob("#{dir}/mrblib/**/*.rb")
end
