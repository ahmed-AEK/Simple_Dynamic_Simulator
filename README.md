# Simple Dynamic System Simulator
A trial implementation of a node editor for solving differential algebraic systems.

Implementation is heavily optimized, so it can acheive 60 fps on a potato using under 1% of CPU
# [Try It Online](https://ahmed-aek.github.io/DynamicSimulator-Online/)

see the **[Demo Video](https://youtu.be/uQ1XzY0SoTk)**  and also check the **[Documentation](https://simple-dynamic-simulator.readthedocs.io/en/latest/)**

![alt text](https://github.com/ahmed-AEK/dummy_NodeEditor/blob/main/docs/preview.png?raw=true)

### Currently Working:
- adding and removing nodes (use ctrl to fork an edge, and shift to delete nodes and edges)
- moving and connecting nodes
- Algebraic Differential Solver
- Display simulation result on Scope block
- Block palette to add blocks
- customized block shapes
- persistent storing and loading blocks (experimental)
- undo/redo
- log in the bottom panel for important information
- lua custom user defined blocks

### Not Implemented yet:
- more blocks, saving output to disk, etc..
- more scripting languages.
- extensions management


### Dependencies:
- Boost (containers, odeint, charconv)
- nlopt
- SDL3, SDL3-TTF, SDL3-image
- google-test
- lunasvg
- SQLiteCpp
- lua, sol2
- tl-expected

### testing

- the GUI has no tests, but it works.
- NLDiffsolver has full test coverage. 
- SceneLoader has full tests to catch any changes in schema.
- NetsSolver has tests to gurantee correctness
- lua extension has tests for the interop

testing is done using ctest, just run `ctest .` in the appropriate build directory.

### supported platforms
- Windows using MSVC
- Linux using GCC
- Wasm using emscripten

### build instructions

have python installed. on linux you also need cmake, ninja and vcpkg and VCPKG_ROOT environment variable setup correctly. x64 Native Tools Command Prompt for VS sets them up on windows.

you need to run `python AssetsPacker/packer.py` to create the `AssetsManager` project, then:

```
cmake . --preset=vcpkg-win-rel
cmake --build ./build/Release_win
```

replace `win` with `linux` for the linux version, and you will need to use `python3` instead of `python`.

License
-----------
GPL-3.0 license (@ 2024 ahmed-AEK)

Any change to the application source needs to be made publicly available at no cost under the GPL. But the application can use commercial proprietary plugins without disclosing the plugin source code.
