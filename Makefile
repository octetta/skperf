all:

builder:
	mkdir -p build && \
  cd build && \
  cmake .. && \
  make

clean:
	rm -rf build

run:
	./build/skperf
