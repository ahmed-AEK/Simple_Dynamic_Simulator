# Dummy NodeEditor
A trial implementation of a node editor for solving differential algebraic systems.

Implementation is heavily optimized, so it can acheive 60 fps on a potato using under 1% of CPU

![alt text](https://github.com/ahmed-AEK/dummy_NodeEditor/blob/main/preview.png?raw=true)

### Currently Working:
- adding and removing nodes (use ctrl to fork an edge, and shift to delete nodes and edges)
- moving and connecting nodes
- Algebraic Differential Solver

### Not Implemented yet:
- persistent storing and loading nodes
- storing functions in nodes
- link solver and GUI
- ability to customize nodes
- ability to preview solver results

### Dependencies:
- Boost (containers, odeint)
- nlopt
- SDL, SDL-TTF, SDL-image
- google-test

### testing

currently only the solver has full test coverage, the GUI has no tests, but it works.

### supported platforms
- Windows on MSVC
- Linux using GCC

### build instructions

the dependencies make it hard to build this on other machines, i don't want to use a package manager yet so that i don't have to re-download the dependencies on each platform.

you need `SDL` and `nlopt` in your `CMAKE_PREFIX_PATH` and you need gtest and boost in the `external` folder, renamed to `boost` and `googletest` (omit the version numbers), then the following is enough.

```
mkdir build
cd build
cmake ..
cmake --build .
```
then you need to manually copy the `assets` folder to the folder containing the executable, because the folder changes with the IDE.

License
-----------
MIT license (@ 2024 ahmed-AEK)