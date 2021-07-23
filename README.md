# Jactorio

[![Build Status](https://dev.azure.com/jaihysc/Jactorio/_apis/build/status/jaihysc.Jactorio?branchName=master)](https://dev.azure.com/jaihysc/Jactorio/_build/latest?definitionId=1&branchName=master)

My attempt at recreating [Factorio](https://factorio.com), inspired by [Hopson98's recreation of Minecraft](https://github.com/Hopson97/MineCraft-One-Week-Challenge)

[Screenshots](https://github.com/jaihysc/Jactorio/wiki/Screenshots)

[Weekly log](https://github.com/jaihysc/Jactorio/wiki/Devlog)

## Building

Requirements:

* Compiler supporting c++17 [Tested on: gcc >= 9, clang >= 9, msvc >= 142]
* Python interpreter 3.x.x
* CMake

Remarks:

* Windows - Ensure CMake is added to the `Paths` environmental variable
* MacOS - Version >= 10.15

---

**Build types:** `Debug, Release, RelWithDebInfo`

**Parameters:** `--notest` if you do not want to build the tests

Executable will be placed in `out/<Build type>/bin/`

---

In a shell:

```bash
git clone https://github.com/jaihysc/Jactorio.git
cd Jactorio
./build.sh <See build types above> --notest
```

## Running tests

**Test parameters:** `--leakcheck` to perform a leak check using valgrind, skipping some false positive tests

The test results will be placed in the directory of `runtests.sh`

---

After following the build steps above **without** `--notest`

```bash
./runtests.sh <Build type used to build>
```

## Dependencies

The installation of dependencies listed below is automatic, but may carry additional dependencies **you** must install:

* [backward-cpp](https://github.com/bombela/backward-cpp)
* [cereal](https://github.com/USCiLab/cereal)
* [decimal_for_cpp](https://github.com/vpiotr/decimal_for_cpp)
* [freetype](https://gitlab.freedesktop.org/freetype/freetype)
* [GLEW](http://glew.sourceforge.net/)
* [glm](https://github.com/g-truc/glm)
* [imgui](https://github.com/ocornut/imgui)
* [libnoise](https://github.com/jaihysc/libnoise)
* [pybind11](https://github.com/pybind/pybind11)
* [SDL2](https://github.com/spurious/SDL-mirror)
* [StackWalker](https://github.com/JochenKalmbach/StackWalker)
* [stb](https://github.com/nothings/stb)
