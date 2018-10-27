require_relative '../lib/stringsink/stringsink'
require 'stringio'

module Utils
  CHARS = '😀😻🤖🙈ͷהⱤ▦▱'.split('') + (32..127).map(&:chr)

  def random_string(range = nil)
    length =
      case range
      when nil then rand(1000)
      when Numeric then range
      when Enumerator then range.to_a.sample
      end

    (0...length).map { CHARS.sample }.join
  end

  def random_char_or_ascii_code
    if rand < 0.5
      (32..127).to_a.sample
    else
      CHARS.sample
    end
  end
end

RSpec.configure do |config|
  config.include(Utils)
end
