#!/bin/bash

if [[ -z "$1" ]] || [[ "$1" != "Debug"  &&  "$1" != "Release" &&  "$1" != "RelWithDebInfo" ]]; then
    echo "Invalid build type"
    echo "Valid build types: Debug, Release, RelWithDebInfo"
    echo "Example usage: sh ./build.sh Debug"

    exit 1
fi


cd "$(dirname "$0")" || exit 2

# create out directory
if [ ! -d "out" ]; then
  mkdir out || exit 2
fi
# Create build type directory
if [ ! -d "out/$1" ]; then
  mkdir out/"$1" || exit 2
fi

cd ./out/"$1" || exit 2


# Build tests if --notest is not defined
if [[ "$2" == "--notest" ]]; then
    cmake ../.. -DCMAKE_BUILD_TYPE="$1" -DJACTORIO_BUILD_TESTS:BOOL="False" || exit 2
elif [ -z "$2" ]; then
    cmake ../.. -DCMAKE_BUILD_TYPE="$1" -DJACTORIO_BUILD_TESTS:BOOL="True" || exit 2
else
    echo "Unknown parameter $2, see README.md"
    exit 1
fi
cmake --build . --config "$1" || exit 2

exit 0
