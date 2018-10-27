# StringSink

Toy Ruby extension implement a write-only version of `StringIO`. Performs better for large buffers being written to in small chunks (because of a better allocation strategy - try running `bench.rb`).
