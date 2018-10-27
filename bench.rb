require 'benchmark'
require_relative 'lib/stringsink/stringsink'
require 'stringio'

ss = StringSink.new
io = StringIO.new

suites = {}

suites[:putc] = Proc.new do |io|
  1_000_000.times { io.putc(65) }
end

suites[:puts_big] = Proc.new do |io|
  string = 'a' * 50_000
  10000.times { io.puts(string) }
end

suites[:puts_big_multi] = Proc.new do |io|
  string = 'a' * 50_000
  1000.times { io.puts(string, string, string, string) }
end

suites[:write_big] = Proc.new do |io|
  string = 'a' * 50_000
  10000.times { io.write(string) }
end

suites[:write_big_random_string] = Proc.new do |io|
  strings = ('a'..'z').map { |ch| ch * 10_000 }
  10000.times { io.write(strings.sample) }
end

suites[:puts_small] = Proc.new do |io|
  string = 'the quick brown fox jumps over the lazy dogs'
  100000.times { io.puts(string) }
end

suites[:puts_huge] = Proc.new do |io|
  io.puts('a' * 100_000_000)
end

suites[:printf_num] = Proc.new do |io|
  1_000_000.times { io.printf("%d\n\n", rand(1_000_000_000_000)) }
end

suites[:shift_small] = Proc.new do |io|
  1_000_000.times { io << 'aaa' << 'bbb' << 'ccc' }
end

suites.keys.each do |name|
  puts("==== #{name}\n")
  proc = suites.fetch(name)

  Benchmark.bm(12) do |b|
    b.report('StringSink') { proc.call(StringSink.new) }
    b.report('StringIO')   { proc.call(StringIO.new) }
  end

  puts
end
