# Build Instructions

## * Requirements

- [git](https://git-scm.com/)
- [git-lfs](https://git-lfs.github.com/)
- [cmake](https://cmake.org/) **>= 3.11**
- [gcc](https://www.gnu.org/software/gcc/) **>= 4.9** or [clang](https://clang.llvm.org/) **>= 3.8** or [msvc](https://visualstudio.microsoft.com/) **>= 2015**

## * Cloning

```bash
$ git clone --recursive git://github.com/enduro2d/enduro2d-toolset.git
$ cd enduro2d-toolset
$ git lfs install
$ git lfs pull
```

## * Building

```bash
$ cd your_toolset_repository_directory
$ mkdir build && cd build
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ cmake --build . -- -j8
```

## * Running

```bash
$ cd your_toolset_build_directory
$ ./model_converter/model2mesh
```

## * Links

- CMake: https://cmake.org/
- CMake documentation: https://cmake.org/documentation/
- CMake FAQ: https://gitlab.kitware.com/cmake/community/wikis/FAQ
