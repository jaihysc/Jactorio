#!/bin/sh

if [[ -z "$1" ]] || [[ "$1" != "Debug"  &&  "$1" != "Release" &&  "$1" != "RelWithDebInfo" ]]; then
    echo "Invalid build type"
    echo "Valid build types: Debug, Release, RelWithDebInfo"
    echo "Example usage: sh ./build.sh Debug"

    exit 1
fi

gtestExecutor=""
gtestFilter="*"

if [[ "$2" == "--leakcheck" ]]; then
    gtestExecutor="valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose"
    gtestFilter="*:-PybindManagerTest.*:DataManagerTest.LoadData*"
    echo "Using $gtestExecutor for leak check"
fi

cd "$(dirname "$0")" || exit 2

cd out/"$1"/test/ || exit 2
$gtestExecutor ./jactorioTest --gtest_shuffle --gtest_repeat=10 --gtest_filter="$gtestFilter" --gtest_output="xml:../../../TEST-gtest_results.xml" || exit 2

exit 0
