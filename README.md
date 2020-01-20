# Jactorio

My attempt at recreating [Factorio](https://factorio.com), inspired by [Hopson98's recreation of Minecraft](https://github.com/Hopson97/MineCraft-One-Week-Challenge)

Check out a [weekly devlog here](https://github.com/jaihysc/Jactorio/wiki/Devlog)

## Work in progress

The game is currently playable with a procedurally generated world to navigate around

## Building

**Build types:** `Debug, Release`

**Parameters:** `--notest` if you do not want to build the tests

---

General requirements:

* Compiler supporting c++17 [gcc >= 9, clang >= 9, msvc >= 142]
  * MacOS version >= 10.15
* CMake
  * Windows - ensure CMake is added to the `Paths` environmental variable
* Git / Git bash
  * Windows - Use Git bash

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

However, they may carry additional dependencies you will need to manually install

* [GLEW](http://glew.sourceforge.net/)
* [GLFW](https://www.glfw.org/)
* [stb](https://github.com/nothings/stb)
* [glm](https://github.com/g-truc/glm)
* [imgui](https://github.com/ocornut/imgui)
* [SDL2](https://github.com/spurious/SDL-mirror)
* [libnoise](https://github.com/jaihysc/libnoise)
