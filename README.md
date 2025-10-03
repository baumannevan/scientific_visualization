# SciVis 2025

A C++ project for CS 453 Scientific Visualization homework assignments and in-class demos.

## Dependencies

This project uses OpenGL for rendering and [GLFW](https://www.glfw.org/) for creating the application window and handling user input. An implementation of OpenGL should be included with the graphics drivers for your system. For GLFW installation, please see below:

### Windows

A 64-bit Windows installation of GLFW is included in the `glfw-3.4` directory. If you are using Windows, you should not need to install any additional libraries to run the code.

### macOS

If you have [Homebrew](https://brew.sh/) installed, GLFW can be installed using
```
brew install glfw
```
and the appropriate environment variables should be set so that CMake can find where it is installed. Alternatively, you can download pre-compiled binaries [here](https://www.glfw.org/download.html) and set the include and library paths in `CMakeLists.txt` manually.

### Linux

On Ubuntu GLFW can be installed using

```
sudo apt-get install libglfw3
sudo apt-get install libglfw3-dev
```

For other distributions, check your package manager to see if it is included. Otherwise, you may have to [compile GLFW from its source files](https://www.glfw.org/docs/latest/compile.html).

## Building the Project

CMake is used as the build system for the project. A CMake version of 3.5 or higher is required, which can be downloaded for Windows, macOS and Linux [here](https://cmake.org/download/).

The general steps to build any CMake project are as follows:

1. From a terminal, navigate to your main project directory containing a `CMakeLists.txt` file, and create a new build directory
    ```bash
    mkdir build
    ```
2. `cd` into the build directory and run `cmake ..`, which will generate build files (e.g., a `.sln` file for Windows Visual Studio users)
    ```bash
    cd build
    cmake ..
    ```
3. You can then use the generated build files to build the project (e.g., by opening the `.sln` file in Visual Studio and building from there), or perform a platform-agnostic build with
    ```bash
    cmake --build .
    ```
    (note the `.`, which specifies the *build* directory created in step 2, not the *source* directory for the project)

In step 3, you can specify the type of *generator* (build files) you want CMake to generate using
```bash
cmake -G <generator> ..
```
Use `cmake -G` to see the full list of the supported generators. Most of the time, the default generator with an asterisk next to it is what you want.

### Windows

The newest version of [Visual Studio](https://visualstudio.microsoft.com/downloads/) installed on a Windows computer is usually the default generator for build files. In this case, a Visual Studio solution file (`SciVis_2025.sln`) will be created in your build directory. You can open the solution file in Visual Studio by double-clicking it.

Once you are in Visual Studio, you can build the project by right-clicking the `SciVis_2025` project in the solution explorer and selecting **Build**.

### macOS/Linux

Unix makefiles are usually the default build files for macOS and Linux. If using this generator, you can build the project by running `make` from inside the build directory.

You can use [gdb](https://sourceware.org/gdb/) on Linux or [lldb](https://lldb.llvm.org/) on Mac to debug the program. Alternatively, you can use an IDE with a built-in debugger for your CMake generator, such as Xcode on macOS.

If you use Xcode as the generator, an `.xcodeproj` file will be created in the build directory.

## Running the Program

The program expects a single command line argument specifying a `.ply` file to visualize. `.ply` files are used to store information about surface meshes, but unlike common `.obj` files, they can also store vertex attributes like scalar, vector, and matrix values.

### Windows

In Visual Studio with the `SciVis_2025.sln` file open, you must first set the project to be run on startup. To do this, right-click the `SciVis_2025` project in the solution explorer, and select **Set as Startup Project**.

You can now run the project clicking the green **Start** button, or by pressing **F5** on your keyboard.

### Mac/Linux

If you used Unix Makefiles as the CMake generator, the executable will be created in your build directory after running `make`. From inside the build directory, all you need to do is run
```bash
./learnply <path_to_ply_file>
```

### Xcode

If using Xcode, you must set the scheme you want to run. The current scheme is displayed at the top of the window, near the center. It will likely say `SciVis_2025`, `ZERO_CHECK`, or `ALL_BUILD`. Click on that text, select `SciVis_2025`, and then run the project by clicking the **Start** button.