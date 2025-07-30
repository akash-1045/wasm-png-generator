# Step 1: Clone zlib
if [ ! -d "zlib" ]; then
  git clone https://github.com/madler/zlib.git
fi

# Step 2: Clone libpng
if [ ! -d "libpng" ]; then
  git clone https://github.com/pnggroup/libpng.git
fi

# Step 3: Build zlib
mkdir -p zlib/build
cd zlib/build
emcmake cmake .. \
  -DZLIB_BUILD_SHARED=OFF \    # disables building shared libraries
  -DZLIB_BUILD_TESTING=OFF \   # skips building test binaries
  -DZLIB_BUILD_STATIC=ON       # enables building a static library (libz.a)
cmake --build .
cd ../../


# Step 4: Build libpng
mkdir -p libpng/build
cd libpng/build
emcmake cmake .. \
  -DPNG_SHARED=OFF \           # disables building shared libraries
  -DPNG_TESTS=OFF \            # skips building test binaries
  -DZLIB_LIBRARY="$PWD/../../zlib/build/libz.a" \     # points to the built static zlib library
  -DZLIB_INCLUDE_DIR="$PWD/../../zlib"                # points to the zlib headers
cmake --build .
cd ../../

# Step 5: Compile  wrapper to WebAssembly
emcc \
  png_wrapper.c \
  libpng/build/libpng.a zlib/build/libz.a \
  -Ilibpng -Ilibpng/build -Izlib -Izlib/build \
  -sEXPORTED_FUNCTIONS='["_generate_png", "_malloc", "_free"]' \
  -sEXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "HEAPU8", "HEAP32"]' \
  -sENVIRONMENT=web \
  -sMODULARIZE \
  -sEXPORT_NAME="createPNGModule" \
  -o png.js


