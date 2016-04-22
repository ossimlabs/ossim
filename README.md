### GoCD Status builds 
Resource | *master* | *dev* | 
------------ | ------------- | -------------
centos6 | ![centos6 Status](http://omar.ossim.org/gocd/status/centos6_master_status.svg) | ![centos6 Status](http://omar.ossim.org/gocd/status/centos6_dev_status.svg)
centos7 | ![centos7 Status](http://omar.ossim.org/gocd/status/centos7_master_status.svg) | ![centos7 Status](http://omar.ossim.org/gocd/status/centos7_dev_status.svg)
 
<a href="https://scan.coverity.com/projects/ossimlabs">
  <img alt="Coverity Scan Build Status"
       src="https://img.shields.io/coverity/scan/7516.svg"/>
</a>


# Welcome to OSSIM

OSSIM is an open source, C++ (mostly), geospatial image processing library used by government, commercial, educational, and private entities throughout the solar system. It has been in active development since the last millenium. This repository contains the full OSSIM package including core library, applications, tests, and build system. It does *not* contain the OSSIM plugins and other OSSIM-related code such as java-bindings ([ossim-oms](https://github.com/ossimlabs/ossim-oms)), and GUI. Those are available for individually cloning. Refer to the root github page for [ossimlabs](https://github.com/ossimlabs).

Quick links:

* [What to Get](#relationship-between-repositories)
* [How to Build](#how-to-build)
* [How to Test](#how-to-test)
* [OSSIM Command Line Utility](#ossim-command-line-utility)
* [Doxygen Source Code Documentation](https://trac.osgeo.org/ossim/doxygen)

# Relationship between Repositories (What to Get)

The principal repository is this one (ossim), containing not only the core classes but also the command line utility applications code as well as the cmake build system files and related scripts. This repository must be cloned first before attempting to work with other _ossim-*_ repositories.

The remaining _ossim-*_ repositories are independent of each other and can be cloned in any order. However, it will be necessary to re-run the [build script (linux version)](/scripts/linux/build.sh) or at least regenerate makefiles with the [cmake script (linux version)](/cmake/scripts/ossim-cmake-config-LINUX.sh) before building. This must be done each time a new ossim-* repository is added to the ossimlabs build. See below for detailed build instructions.

# Branching Scheme and Pull Requests

The "master" branches (for all OSSIM family repos) contain the most stable code. It is periodically merged with "dev" branch. If you don't plan on developing code, but want to build from source, this is the branch you want. For bleeding edge development, use the "dev" branch. This branch serves as the parent to all feature branches and is frequently merged with those feature branches once those branches are tested by the developers responsible for them.

If you want to contribute code, you'll need to:
 * create your own feature branch from dev, 
 * test your modification, 
 * then create a pull request. 

The moderators will review the request and merge your changes to "dev". At that point, the automated continuous integration system (GoCD) will get triggered to perform a build and test. If there are any failures, your merge commit will be backed out.

#How to Build
The easy way is to just run the script in `ossim/scripts/build.sh`.  This approach should work for the default build configuration. This currently works for both linux and Mac. We hope to soon have a Windows `build.bat` file and world peace. Assuming all dependencies were met and no compile/link errors occured, this script will generate all build binaries under a sibling directory to the ossim repo directory, namely, `ossim/../build`. See the section on environment variables below for options on customizing the build location.

This repository provides the CMAKE infrastructure necessary to build the OSSIM core library and related plugins and applications. Throughout this document, reference is made to the local, top-level directory containing this repository. We'll call this directory simply *ossim-dev-home*. (In fact, you'll find the shell variable `OSSIM_DEV_HOME` used throughout the various scripts in this and other repositories.

The following 3rd-party SDKs are needed in order to build the core ossim library:

    libtiff-devel (preferably version 4.x for BigTIFF support)
    OpenThreads-devel
    libjpeg-devel
    libgeos-devel (on later Debian / Ubuntu versions, libgeos++-dev)

Plugins will require additional 3rd-party packages.

Building OSSIM and related repos from source is a two-step process: first create the make files using CMake, then run `make` to build the binaries. Scripts are available for Linux/Mac and (soon) Windows to run CMake with default settings. You can run the script from any working directory and default settings will be used for creating the default build environment. It is possible to override specific defaults for your own custom requirements or preferences.

Presently, the default setting is to disable the building of all plugins. If you want specific plugin libraries to be built, you'll need to edit your cmake script at `cmake/scripts/ossim-cmake-config.sh` (or similar `.bat` file for Windows). Scroll down to the "Plugins" section and set the corresponding variables to "ON". Note that you could also define those variables in your shell environment in lieu of editing the script.

## Creating the Makefiles

### Creating a Default Build Environment
The cmake configuration scripts are available in the `cmake/scripts` subdirectory. Assuming no OSSIM environment variables are available to override the defaults, the "out-of-source" `build` directory will be created under the same parent directory *ossim-dev-home* as this repo. The linux/Mac script `ossim/scripts/build.sh`, if run in an interactive shell will query for the build type. If the script is run as part of a batch process, "Release" is assumed. If the build directory does not exist, it will be created.

### Customizing the Build
There are two ways to customize the build: via environment variables and by directly editing the cmake configuration script. 

#### Environment Variables
The CMake system will locally define certain environment variables that live for the lifetime of the cmake config script execution. The following shell variables, if defined, are referenced to override the default settings.

The developer has the option to override the default build directory location by setting the environment variable `OSSIM_BUILD_DIR` prior to running the cmake config script. If not present, the `build` directory will be located just under *ossim-dev-home* as described above. 

Another defaulted environment variable is `OSSIM_INSTALL_PREFIX`. This variable as two distinct functions. First, it indicates where to install the OSSIM SDK when running `make install`. Second, it serves to specify a path to SDKs that OSSIM depends on. The CMake system will scan `OSSIM_INSTALL_PREFIX` for the presence of dependency packages such as GeoTiff, JPEG, and others. This secondary purpose of `OSSIM_INSTALL_PREFIX` used to be handled by the now obsolete environment variable `OSSIM_DEPENDENCIES`. It is a reasonable consolidation since the OSSIM install will need to include these dependencies if they are not available in their standard installation locations (/usr/lib, /usr/local/lib, etc.), so placing these SDKs in the final OSSIM install directory prior to building OSSIM makes sense. If no override is defined for `OSSIM_INSTALL_PREFIX`, then the cmake config script will default to `$OSSIM_DEV_HOME/install`. You can populate that directory with non-standard installs of the OSSIM dependencies prior to running the script. If the directory does not exist, it is created by the script. Obviously, in that case, all OSSIM dependencies will be expected to be found in standard system install folders.

As already mentioned, you can enable the building of specific plugins by defining the corresponding environment variable: `BUILD_<NAME>_PLUGIN=ON`.

NOTE: For legacy reasons, `OSSIM_DEPENDENCIES` is still scanned for dependency SDKs. New dependencies however should be "installed" in `OSSIM_INSTALL_PREFIX`.

#### Editing the CMake Config Script
The default configuration relies on the presence of the OSSIM repositories under the *ossim-dev-home* to decide whether to include those in the build (with the exception of the plugins, which default to "OFF"). You may want to selectively exclude certain applications from the build without having to hide the workspaces from CMake. The flags enabling those are defined in the script. Simply set the corresponding variable to "OFF" and rerun the script. Likewise, for the plugins, you can modify the default ("OFF") by changing the corresponding variables in the script to "ON".

There are other flags available that direct CMake to generate project files for IDEs such as Eclipse and Visual Studio. You can also specify multi-threaded builds, non-standard output directories, and more. Feel free to experiment, but know that you'll be voiding the warranty.

### _How do I know what dependencies are needed?_ 

The CMake system will attempt to locate all necessary dependency SDKs in the system's standard install directories. On linux, this includes, but not limited to, /usr and /usr/local. If it cannot find them, the script will exit with an error message indicating the missing library. You can then install the missing library from the third-party repository before trying to run the OSSIM cmake config script again. You may get several dependency errors before CMake succeeds in generating all the Makefiles needed.

Remember, you can provide "sandbox" installations of dependency libraries that you want to link with, even though your system may (or may not) have installed different versions of those libraries. That's achieved by "installing" those SDKs inside the directory indicated by the CMake environment variable `OSSIM_INSTALL_PREFIX` (defaults to *ossim-dev-home*/install, see above).

Eventually, we hope to provide an artifact repository so that CMake itself can access any missing dependencies without the need for the developer to manually install them.

## Building the Binaries

Once the cmake configuration script terminates successfully, you are ready to build the binaries. Build instructions vary slightly between OS's, but generally involve simply running `make` inside the build directory.

First change directory to the build folder created by the cmake config script. There you will find a top-level Makefile that will bootstrap the build. From a terminal, run `make`. The build should proceed normally for a few minutes, longer if the first time through. Upon successful completion, you should find a "lib" and "bin" folder containing the ossim library and executable, respectively.

Developers have different ways of working -- all we're helping you with here is building the OSSIM library and associated plugins and executables. You may choose to then run `make install` to copy the binaries to some standard system location (you'll need to have the appropriate permissions). Alternatively, you can append your `PATH` environment variable to include the `build/bin` folder containing the executables. You also need to update the library path to include `build/lib`. These settings vary by OS so you're on your own here.

## Integrated Development Environments

You may be able to import the build environment into your IDE "as a Makefile project." The Windows cmake configuration script will generate Visual Studio project files for direct import into Visual Studio. You may want to take a look at that script to see if there is a custom setting (or command line argument) you need to tinker with.

### Eclipse Users
For convenience, you can specify "eclipse" as the build-type to the script at `ossim/cmake/scripts/ossim-cmake-config-LINUX.sh` that will generate the Eclipse CDT4 project files along with the makefiles for a debug build. The output build directory will be one level up from *ossim-dev-home* since Eclipse does not support the build directory as sibling of source. Eclipse will properly generate "Subprojects" corresponding to each ossimlabs repo present in your *ossim-dev-home*  for indexed navigation of the source files. It is built with debug. To import into your Eclipse workspace, right-click in the Project Explorer and select Import->General->"Existing Project...", then select the root directory by browsing to `<ossim-dev-home>/../build` and clicking "OK".

# How to Test

The first check of a successful build is to run the ossim command line utility `ossim-cli --version`. You should get the current version output to the console. More on the `ossim-cli` command below.

Testing is generally divided into unit, functional, and integration tests. At the moment, true unit testing in OSSIM is very limited, and likely to stay that way. There is however a _de facto_ scheme in place that has been used for implementing integration and functional testing using the `ossim-batch-test` command line executable. Much of the functional testing supplied through `ossim-batch-test` configuration files is fairly fine-grained, almost to the unit-test level of granularity. This testing platform lets the developer create any level of test, from top-level integration test to individual class method test. This application spawns other OSSIM utility applications to generate results, including dedicated test utilities, which are then compared against an expected-results dataset. This command-line app reads a configuration file that specifies a test or series of tests to run and possibly compare against expected results. There is an option to accept current results as the new expected results. While not explicitly required, the majority of testing done with `ossim-batch-test` will involve input source data, typically imagery, that the tester must preinstall on the target machine. The expected results will also need to be generated and its location specified on the command line. See the usage for `ossim-batch-test` for more detail.

A rudimentary test suite is encapsulated in this [test script (linux version)](/scripts/test.sh). It defines the locations of the input source data and expected results, and runs the command-line test applications. 

The _ossim_ repository contains `test/src` and `test/config` subdirectories which contain items available for testing OSSIM core functionality. At some point when we start including unit tests, there will be a third directory, `test/scripts`, that will contain the commands to run those unit tests.

## test/src

This directory contains the source code for unit and functional testing of core OSSIM objects. It is automatically built by the cmake/make system, assuming `BUILD_OSSIM_TESTS` is set to `ON` (the default). The resulting executables, located in `build/bin`, can be run standalone on the command line. The code in this directory is also a good source of examples for interfacing to many important OSSIM classes. This directory also contains the source code for `ossim-batch-test`.

## test/config

This subdirectory contains, primarily, configuration files for existing `ossim-batch-test` runs. 

The input data referenced in the `ossim-batch-test` configuration files are presently hosted on an Amazon S3 storage. Contact the [OSSIM developer list](mailto:ossim-developer@lists.sourceforge.net) if you want access to this public data for your own testing.

# OSSIM Command Line Utility

The executable `ossim-cli` will eventually consolidate the plethora of ossim utility applications currently existing. For now it supports a small subset of utilities though among them is `info` that reproduces the functionality of the ubiquitous `ossim-info`.

The usage is:   `ossim-cli <command> [options and parameters]`

With no arguments, the app will give a list of capabilities. Currently those are

* hillshade -- Computes shaded representation of input elevation surface with specified lighting parameters.
* hlz -- Computes bitmap of helicopter landing zones given ROI and DEM.
* info -- Dumps metadata information about input image and OSSIM in general.
* potrace -- Computes vector representation of input raster image.
* shoreline -- Computes bitmap of water versus land areas in an input image.
* slope -- Utility for computing the slope at each elevation post and generating a corresponding slope image.
* viewshed -- Computes bitmap image representing the viewshed from specified location using only DEM information.

For example, to get projection and image information for an image file, use

 `ossim-cli info -p -i <image-filename>`

You can also give a keyword list (KWL) file as the only arg. The KWL must contain the keyword "tool" with one of the supported commands above, along with the keywords expected by that utility. You can even get KWL templates or enter the keyword/values interactively.
