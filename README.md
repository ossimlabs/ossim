![ossim logo](/ossim-logo.png) 

Welcome to OSSIM, an open source, C++, geospatial image processing library used by government, commercial, educational, and private entities throughout the planet. It has been in active development since 1996. This repository contains the full OSSIM package including core library, applications, tests, and build system

# Related OssimLabs Repositories

Coming soon.

# How to Build

This repository provides the CMAKE infrastructure necessary to build the OSSIM core library and related plugins and applications. Throughout this document, reference is made to the local, top-level directory corresponding to this repository (./OSSIM). We'll call this directory simply *ossim-top-dir*. 

The following 3rd-party SDKs are needed in order to build the core ossim library:

    libtiff (preferrably version 4.x for BigTIFF support)
    OpenThreads
    libjpeg
    libgeos

Plugins will require additional 3rd-party packages.

Building OSSIM and related repos from source is a two-step process: first create the make files using CMake, then run `make` to build the binaries. Scripts are available for Linux, Windows, and Mac to run CMake with default settings. You can run the script from any working directory and default settings will be used for creating the default build environment. It is possible to override specific defaults for your own custom requirements or preferences.

## Creating the Makefiles

### Creating a Default Build Environment
The cmake configuration scripts are available in the `build/scripts` subdirectory. Assuming no OSSIM environment variables are available to override the defaults, an "out-of-source" build directory will be created under the `build` directory. On linux systems, it will be named "Debug", "Release", "RelWithDebInfo", or "MinSizeRel" depending on the type of build requested. The script (if run in an interactive shell) will query for the build type. If the script is run as part of a batch process, "Release" is assumed. If the build directory does not exist, it will be created.

### Customizing the Build
There are two ways to customize the build: via environment variables and by directly editing the cmake configuration script. 

#### Environment Variables
The CMake system will locally define certain environment variables that live for the lifetime of the cmake config script execution. The following shell variables, if defined, are referenced to override the default settings.

The developer has the option to override the default build directory location by setting the environment variable `OSSIM_BUILD_DIR` prior to running the cmake config script. If not present, the build directory will be located under `build` as described above. (There is a `.gitignore` in this repo that lists both "Debug" and "Release", so you won't be "polluting" your repository with unversioned items.)

The location of the source code repository is assumed to be the *ossim-top-dir* directory containing build subdirectory. This top-level directory is represented in the cmake scripts by the local environment variable `OSSIM_DEV_HOME` and defaults to the *ossim-top-dir* directory. If, for some reason, the source is located elsewhere, you can override this with an environment var of the same name.

Another defaulted environment variable is `OSSIM_INSTALL_PREFIX`. This variable as two distinct functions. First, it indicates where to install the OSSIM SDK when running `make install`. Second, it serves to specify a path to SDKs that OSSIM depends on. The CMake system will scan `OSSIM_INSTALL_PREFIX` for the presence of dependency packages such as GeoTiff, JPEG, and others. This secondary purpose of `OSSIM_INSTALL_PREFIX` used to be handled by the now obsolete environment variable `OSSIM_DEPENDENCIES`. It is a reasonable consolidation since the OSSIM install will need to include these dependencies if they are not available in their standard installation locations (/usr/lib, /usr/local/lib, etc.), so placing these SDKs in the final OSSIM install directory prior to building OSSIM makes sense. If no override is defined for `OSSIM_INSTALL_PREFIX`, then the cmake config script will default to `$OSSIM_DEV_HOME/install`. You can populate that directory with non-standard installs of the OSSIM dependencies prior to running the script. If the directory does not exist, it is created by the script. Obviously, in that case, all OSSIM dependencies will be expected to be found in standard system install folders.

#### Editing the CMake Config Script
The default configuration relies on the presence of the OSSIM repositories under the *ossim-top-dir* to decide whether to include those in the build. You may want to selectively exclude certain plugins or applications from the build without having to hide the workspaces from CMake. The flags enabling those are defined in the script. Simply set the corresponding variable to "OFF" and rerun the script. 

There are other flags available that direct CMake to generate project files for IDEs such as Eclipse and Visual Studio. You can also specify multi-threaded builds, non-standard output directories, and more. Feel free to experiment, but know that you'll be voiding the warranty.

### _How do I know what dependencies are needed?_ 

The CMake system will attempt to locate all necessary dependency SDKs in the system's standard install directories. On linux, this includes, but not limited to, /usr and /usr/local. If it cannot find them, the script will exit with an error message indicating the missing library. You can then install the missing library from the third-party repository before trying to run the OSSIM cmake config script again. You may get several dependency errors before CMake succeeds in generating all the Makefiles needed.

Remember, you can provide "sandbox" installations of dependency libraries that you want to link with, even though your system may (or may not) have installed different versions of those libraries. That's achieved by "installing" those SDKs inside the directory indicated by the CMake environment variable `OSSIM_INSTALL_PREFIX` (defaults to *ossim-top-dir*/install, see above).

Eventually, we hope to provide an artifact repository so that CMake itself can access any missing dependencies without the need for the developer to manually install them.

## Building the Binaries

Once the cmake configuration script terminates successfully, you are ready to build the binaries. Build instructions vary slightly between OS's, but generally involve simply running `make` inside the build directory.

First change directory to the build folder created by the cmake config script (defaulted to either "Debug" or "Release"). There you will find a top-level Makefile that will bootstrap the build. From a terminal, run `make`. The build should proceed normally for a few minutes, longer if the first time through. Upon successful completion, you should find a "lib" and "bin" folder containing the ossim library and executable, respectively.

Developers have different ways of working -- all we're helping you with here is building the OSSIM library and associated plugins and executables. You may choose to then run `make install` to copy the binaries to some standard system location (you'll need to have the appropriate permissions). Alternatively, you can append your `PATH` environment variable to include the Release/bin (or Debug/bin) folder containing the executables. You also need to update the library path to include Release/lib (or Debug/lib). These settings vary by OS so you're on your own here. (Side note: Ubuntu doesn't use the Linux standard `$LD_LIBRARY_PATH`, for example). 

## Integrated Development Environments

You may be able to import the build environment into your IDE "as a Makefile project." The Windows cmake configuration script will generate Visual Studio project files for direct import into Visual Studio. You may want to take a look at that script to see if there is a custom setting (or command line argument) you need to tinker with.

# How to Test

Coming soon. NOTE: We expect this will be a script in \test\scripts that will run all unit and functional tests.

# How to Package and Install

Coming soon. NOTE: We expect this will be a script in \test\scripts that will run all unit and functional tests.


ll
