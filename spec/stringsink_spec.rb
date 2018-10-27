require 'spec_helper'

describe StringSink do
  it 'is totally workalike with StringIO for the methods we care about' do
    100.times do
      ss = StringSink.new
      io = StringIO.new

      expect(ss.string).to eq('')
      expect(io.string).to eq(io.string)

      50.times do
        case rand(6)
        when 0
          string = random_string
          expect(ss.write(string)).to eq(string.bytesize)
          expect(io.write(string)).to eq(string.bytesize)
        when 1
          string = random_string
          expect(ss << string).to eq(ss)
          expect(io << string).to eq(io)
        when 2
          strings = (0...rand(5)).map { random_string }
          expect(ss.print(*strings)).to be_nil
          expect(io.print(*strings)).to be_nil
        when 3
          strings = (0...rand(5)).map { random_string }
          expect(ss.puts(*strings)).to be_nil
          expect(io.puts(*strings)).to be_nil
        when 4
          char_or_code = random_char_or_ascii_code
          expect(ss.putc(char_or_code)).to eq(char_or_code)
          expect(io.putc(char_or_code)).to eq(char_or_code)
        when 5
          string = random_string
          number = rand(10**9)
          expect(ss.printf("%s %d", string, number)).to eq(nil)
          expect(io.printf("%s %d", string, number)).to eq(nil)
        end

        expect(ss.string).to eq(io.string)
      end
    end
  end
end
