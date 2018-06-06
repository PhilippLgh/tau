
Tau uses CEF as "webview" and Node.js as main process runtime. It is meant to be a safer Electron alternative.
The [Chromium Embedded Framework](https://bitbucket.org/chromiumembedded/cef/) (CEF) is a simple framework for embedding Chromium-based browsers in other applications.

# Quick Links



# Setup

First install some necessary tools and download the project source code.

1\. Install [CMake](https://cmake.org/), a cross-platform open-source build system. Version 2.8.12.1 or newer is required.

2\. Install [Python](https://www.python.org/downloads/). Version 2.x is required. If Python is not installed to the default location you can set the `PYTHON_EXECUTABLE` environment variable before running CMake (watch for errors during the CMake generation step below).

3\. Install platform-specific build tools.

* Linux: Currently supported distributions include Debian Wheezy, Ubuntu Precise, and related. Ubuntu 14.04 64-bit is recommended. Newer versions will likely also work but may not have been tested. Required packages include: build-essential, libgtk2.0-dev, libgtkglext1-dev.
* macOS: Xcode 6 or newer building on macOS 10.9 (Mavericks) or newer is required. Xcode 8.3 and macOS 10.12 (Sierra) are recommended. The Xcode command-line tools must also be installed. Only 64-bit builds are supported on macOS.
* Windows: Visual Studio 2013 or newer building on Windows 7 or newer is required. Visual Studio 2017 and Windows 10 64-bit are recommended.

4\. Download the tau source code by using [Git](https://git-scm.com/) command-line tools:

```
git clone https://github.com/PhilippLgh/tau.git
```

# Build

Now run CMake which will download the CEF binary distribution from the [Spotify automated builder](http://opensource.spotify.com/cefbuilds/index.html) and generate build files for your platform. Then build using platform build tools. For example, using the most recent tool versions on each platform:

```
cd tau

# Create and enter the build directory.
mkdir build
cd build

# To perform a Linux build using a 32-bit CEF binary distribution on a 32-bit
# Linux platform or a 64-bit CEF binary distribution on a 64-bit Linux platform:
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ..
make -j4 cefclient cefsimple

# To perform a macOS build using a 64-bit CEF binary distribution:
cmake -G "Xcode" ..
# Then, open build\cef.xcodeproj in Xcode and select Product > Build.

# To perform a Windows build using a 32-bit CEF binary distribution:
cmake -G "Visual Studio 15" ..
# Then, open build\cef.sln in Visual Studio 2017 and select Build > Build Solution.

# To perform a Windows build using a 64-bit CEF binary distribution:
cmake -G "Visual Studio 15 Win64" ..
# Then, open build\cef.sln in Visual Studio 2017 and select Build > Build Solution.
```

CMake supports different generators on each platform. Run `cmake --help` to list all supported generators. Generators that have been tested include:

* Linux: Ninja, Unix Makefiles
* macOS: Ninja, Xcode 6+
* Windows: Ninja, Visual Studio 2013+

Ninja is a cross-platform open-source tool for running fast builds using pre-installed platform toolchains (GNU, clang, Xcode or MSVC). See comments in the "third_party/cef/cef_binary_*/CMakeLists.txt" file for Ninja usage instructions.