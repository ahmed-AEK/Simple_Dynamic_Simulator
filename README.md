# Simple Dynamic System Simulator
A trial implementation of a node editor for solving differential algebraic systems.

Implementation is heavily optimized, so it can acheive 60 fps on a potato using under 1% of CPU
# [Try It Online](https://ahmed-aek.github.io/DynamicSimulator-Online/)

**[Demo Video](https://youtu.be/xRRwlTSGhQI)**

![alt text](https://github.com/ahmed-AEK/dummy_NodeEditor/blob/main/preview.png?raw=true)

### Currently Working:
- adding and removing nodes (use ctrl to fork an edge, and shift to delete nodes and edges)
- moving and connecting nodes
- Algebraic Differential Solver
- Display simulation result on Scope block
- Block pallete to add blocks
- customized block shapes
- persistent storing and loading blocks (experimental)
- undo/redo

### Not Implemented yet:
- ability to add user defined blocks
- scripting engine
- more blocks, saving output to disk, etc..


### Dependencies:
- Boost (containers, odeint)
- nlopt
- SDL3, SDL3-TTF, SDL3-image
- google-test
- lunasvg

### testing

- the GUI has no tests, but it works.
- NLDiffsolver has full test coverage. 
- SceneLoader has full tests to catch any changes in schema.
- NetsSolver has tests to gurantee correctness

testing is done using ctest, just run `ctest .` in the appropriate build directory.

### supported platforms
- Windows on MSVC
- Linux using GCC

### build instructions

the dependencies make it hard to build this on other machines, i don't want to use a package manager yet so that i don't have to re-download the dependencies on each platform.

you need `SDL3`, `SDL3-TTF`, `SDL3-Image`, `nlopt` and `lunasvg` on your `CMAKE_PREFIX_PATH` and you need gtest and boost and SQLiteCpp in the `external` folder, renamed to `boost` and `googletest` and `SQLiteCpp` (omit the version numbers), then the following is enough.

you need to run `python AssetsPacker/packer.py` to create the `AssetsManager` project, then:

```
mkdir build
cd build
cmake ..
cmake --build .
```

License
-----------
GPL-3.0 license (@ 2024 ahmed-AEK)

This prevents re-packaging and selling the software, but it can still use commercial proprietary plugins.

[def]: https://dynamicsimulator-beta.static.domains/