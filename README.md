# Jactorio

My attempt at recreating [Factorio](https://factorio.com), inspired by [Hopson98's recreation of Minecraft](https://github.com/Hopson97/MineCraft-One-Week-Challenge)

## Building

**Build types:** `Debug, Release`

### Linux

```
git clone https://github.com/jaihysc/Jactorio.git
cd Jactorio
mkdir out
cd out
cmake .. -D CMAKE_BUILD_TYPE=**See build types above**
make
```

Executable will be placed in `out/src/`

## Dependencies

These are automatically installed with cmake

- [GLEW](http://glew.sourceforge.net/)
- [GLFW](https://www.glfw.org/)
- [stb](https://github.com/nothings/stb)
- [glm](https://github.com/g-truc/glm)
- [imgui](https://github.com/ocornut/imgui)

## Work in progress

This is nowhere near ready for usage, this will be updated as conditions change
