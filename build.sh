cmake -S . -B build \
  -DCODEGREP_BUILD_TESTS=ON \
  -DCODEGREP_TEST_FIXTURE_ROOT=/home/tszyc/llp/etc/mini-java-playground

cmake --build build -j
