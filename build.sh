#!/bin/sh

if [ -z "$1" ]; then
    echo "Build type not defined,"
    echo "See valid build types in README.md"
    echo "Example usage: sh ./build.sh Debug"

    exit 1
fi


cd "$(dirname "$0")"

# create out directory
if [ ! -d "out" ]; then
  mkdir out
fi
# Create build type directory
if [ ! -d "out/$1" ]; then
  mkdir out/$1
fi

cd ./out/$1


# Build tests if --notest is not defined
if [ "$2" == "--notest" ]; then
    cmake ../.. -DCMAKE_BUILD_TYPE=$1 -DJACTORIO_BUILD_TESTS:BOOL="False"
elif [ -z "$2" ]; then
    cmake ../.. -DCMAKE_BUILD_TYPE=$1 -DJACTORIO_BUILD_TESTS:BOOL="True"
else
    echo "Unknown parameter $2, see README.md"
    exit 1
fi
make

exit 0
