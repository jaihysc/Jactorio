#!/bin/sh

if [[ -z "$1" ]] || [[ "$1" != "Debug"  &&  "$1" != "Release" ]]; then
    echo "Invalid build type"
    echo "Valid build types: Debug, Release"
    echo "Example usage: sh ./build.sh Debug"

    exit 1
fi


cd "$(dirname "$0")" || exit 2

cd out/"$1"/test/ || exit 2
./jactorioTest --gtest_output="xml:../../../TEST-gtest_results.xml" || exit 2

exit 0
