require "rake/extensiontask"

gemspec = Gem::Specification.new do |s|
  s.name       = 'stringsink'
  s.version    = '0.1.0'
  s.summary    = 'Fast library for file-like output to strings'
  s.authors    = ['Adam Richardson (as3richa)']
  s.licenses   = ['MIT']
  s.files      = ['lib/stringsink.rb', 'ext/stringsink/stringsink.c']
  s.extensions = ['ext/stringsink/extconf.rb']
end

Gem::PackageTask.new(gemspec) { |_pkg| }

Rake::ExtensionTask.new('stringsink') do |ext|
  ext.lib_dir = "lib/stringsink"
end
