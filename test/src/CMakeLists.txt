# $Id: CMakeLists.txt 23496 2015-08-28 15:26:18Z okramer $

# Only "install" the following. 
OSSIM_SETUP_APPLICATION(ossim-batch-test INSTALL COMPONENT_NAME ossim SOURCE_FILES ossim-batch-test.cpp)

# Remainder to be built but not installed
OSSIM_SETUP_APPLICATION(ossim-foo COMMAND_LINE COMPONENT_NAME ossim SOURCE_FILES ossim-foo.cpp)
OSSIM_SETUP_APPLICATION(ossim-test COMMAND_LINE COMPONENT_NAME ossim SOURCE_FILES ossim-test.cpp)
OSSIM_SETUP_APPLICATION(HelloWorld COMMAND_LINE COMPONENT_NAME ossim SOURCE_FILES HelloWorld.cpp)

add_subdirectory(base)
add_subdirectory(elevation)
add_subdirectory(gsoc)
add_subdirectory(imaging)
add_subdirectory(parallel)
add_subdirectory(point_cloud)
add_subdirectory(projection)
add_subdirectory(support_data)
add_subdirectory(util)
add_subdirectory(vec)

if (OSSIM_HAS_HDF5)
  add_subdirectory(hdf5)
endif (OSSIM_HAS_HDF5)
