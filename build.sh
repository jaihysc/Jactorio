#!/bin/sh

if [ -z "$1" ]; then
    echo "Build type not defined,"
    echo "See valid build types on in README.md"
    echo "Example usage: sh ./build.sh Debug"

    exit 1
fi


cd "$(dirname "$0")"

# create out directory
if [ ! -d "out" ]; then
  mkdir out
fi

cd ./out
# Build tests if --no-build-tests is not defined
if [ "$2" == "--no-build-tests" ]; then
    cmake .. -DCMAKE_BUILD_TYPE=$1
else
    cmake .. -DCMAKE_BUILD_TYPE=$1 -DJACTORIO_BUILD_TESTS:BOOL="True"
fi
make

exit 0
