#!/bin/sh

if [ -z "$1" ]; then
    echo "Build type not defined,"
    echo "See valid build types on https://github.com/jaihysc/Jactorio"
    echo "Example usage: build.sh Debug"

    exit 1
fi


cd "$(dirname "$0")"

# create out directory
if [ ! -d "out" ]; then
  mkdir out
fi

cd ./out
cmake .. -DCMAKE_BUILD_TYPE=$1
make

exit 0
