# Jactorio

My attempt at recreating [Factorio](https://factorio.com), inspired by [Hopson98's recreation of Minecraft](https://github.com/Hopson97/MineCraft-One-Week-Challenge)

Check out a [weekly devlog here](https://github.com/jaihysc/Jactorio/wiki/Devlog)

## Work in progress

This is nowhere near ready for usage, this will be updated as conditions change

## Building

**Build types:** `Debug, Release`

Executables will be placed in `out/`

### Windows

Open the `jactorio/` directory in Visual Studio, ensure you have the [sufficient packages for CMake and C++](https://docs.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=vs-2019)

### Linux

```bash
git clone https://github.com/jaihysc/Jactorio.git
cd Jactorio
sh ./build.sh `See build types above` --no-build-tests
```

Add `--no-build-tests` if you don't want to build the tests

## Dependencies

These are automatically installed with cmake

- [GLEW](http://glew.sourceforge.net/)
- [GLFW](https://www.glfw.org/)
- [stb](https://github.com/nothings/stb)
- [glm](https://github.com/g-truc/glm)
- [imgui](https://github.com/ocornut/imgui)
- [SFML](https://github.com/SFML/SFML)
- [libnoise](https://github.com/jaihysc/libnoise)