# Jactorio

[![Build Status](https://dev.azure.com/jaihysc/Jactorio/_apis/build/status/jaihysc.Jactorio?branchName=master)](https://dev.azure.com/jaihysc/Jactorio/_build/latest?definitionId=1&branchName=master)

My attempt at recreating [Factorio](https://factorio.com), inspired by [Hopson98's recreation of Minecraft](https://github.com/Hopson97/MineCraft-One-Week-Challenge)

Check out a [weekly log here](https://github.com/jaihysc/Jactorio/wiki/Devlog)

## Work in progress

The game is currently playable with a procedurally generated world and transport lines.

## Building

General requirements:

**NOTE:** Functionality not guaranteed on MacOS

* Compiler supporting c++17 [gcc >= 9, clang >= 9, msvc >= 142]
* Python interpreter 3.7.x
* CMake

Remarks:
 * Windows - Ensure CMake is added to the `Paths` environmental variable
 * Windows - Use git bash to run the bash files
 * MacOS - Version >= 10.15
 
---

**Build types:** `Debug, Release, RelWithDebInfo`

**Parameters:** `--notest` if you do not want to build the tests

---

In a bash shell on Mac and Linux / Git bash on Windows, run

```bash
git clone https://github.com/jaihysc/Jactorio.git
cd Jactorio
sh ./build.sh <See build types above> --notest
```

Executable will be placed in `out/<Build type>/src/`

## Running tests

After following the build steps above **without** `--notest`, run the following in a shell or Git bash

```bash
sh ./runtests.sh <Build type used to build>
```

The test results will be placed in the directory of `runtests.sh`

## Dependencies

The installation of dependencies listed below is automatic

* [backward-cpp](https://github.com/bombela/backward-cpp)
* [decimal_for_cpp](https://github.com/vpiotr/decimal_for_cpp)
* [GLEW](http://glew.sourceforge.net/)
* [GLFW](https://www.glfw.org/)
* [glm](https://github.com/g-truc/glm)
* [imgui](https://github.com/ocornut/imgui)
* [libnoise](https://github.com/jaihysc/libnoise)
* [pybind11](https://github.com/pybind/pybind11)
* [SDL2](https://github.com/spurious/SDL-mirror)
* [StackWalker](https://github.com/JochenKalmbach/StackWalker)
* [stb](https://github.com/nothings/stb)

