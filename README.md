# soot-dem-gui

### Intro

Software for soot aggregate mechanics simulations with a
graphical user interface. Description of the contact model is available
in the preprint. A tutorial is available on [Egor Demidov's web page](https://www.edemidov.com/posts/soot-dem-gui).
Windows and macOS binaries can be downloaded in the [releases section](releases) of this
repository. See below for instructions on building from source.

### Building from source

System dependencies:

- CMake
- git
- Ninja build
- Qt Widgets
- VTK built with Qt support
- C++ compiler with C++20 and OpenMP support

The dependencies can be installed with your system's package manager or vcpkg.

#### Step 1: obtaining the source code

Run:
```shell
git clone https://github.com/egor-demidov/soot-dem-gui.git
cd soot-dem-gui
git submodule update --init
```

#### Step 2: configure the build

If VTK and Qt are installed system-wide, run in `soot-dem-gui` directory:
```shell
mkdir build
cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
```

If VTK and Qt are to be installed through vcpkg:
```shell
cmake -G Ninja --preset=default .
```

#### Step 3: build

In the `build` directory, run:
```shell
cmake --build . --config Release
```

