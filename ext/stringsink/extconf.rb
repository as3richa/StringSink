require 'mkmf'

flags = %w[-std=c99 -pedantic -Wall -Wextra -Werror -O3]

flags << '-DSTRINGSINK_DEBUG' if ENV.key?('STRINGSINK_DEBUG')

$CFLAGS = flags.join(' ')

create_makefile 'stringsink/stringsink'
