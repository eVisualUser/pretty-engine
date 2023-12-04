echo Deprecated: use the go lang scripts in the tools directory

cmake -S . -B "build" -G "Ninja"
ninja -C build
