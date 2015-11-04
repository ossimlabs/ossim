# $Id: CMakeLists.txt 23496 2015-08-28 15:26:18Z okramer $

# Only "install" the following. 
OSSIM_SETUP_APPLICATION(ossim-batch-test INSTALL COMPONENT_NAME ossim SOURCE_FILES ossim-batch-test.cpp)

# Remainder to be built but not installed
OSSIM_SETUP_APPLICATION(ossim-foo COMMAND_LINE COMPONENT_NAME ossim SOURCE_FILES ossim-foo.cpp)
OSSIM_SETUP_APPLICATION(ossim-test COMMAND_LINE COMPONENT_NAME ossim SOURCE_FILES ossim-test.cpp)

subdirs(base)
subdirs(elevation)
subdirs(gsoc)
subdirs(imaging)
subdirs(parallel)
subdirs(point_cloud)
subdirs(projection)
subdirs(support_data)
subdirs(util)
subdirs(vec)

