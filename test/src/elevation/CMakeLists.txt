# $Id: CMakeLists.txt 23496 2015-08-28 15:26:18Z okramer $

# Remainder to be built but not installed
OSSIM_SETUP_APPLICATION(ossim-dted-handler-test COMMAND_LINE COMPONENT_NAME ossim SOURCE_FILES ossim-dted-handler-test.cpp)
OSSIM_SETUP_APPLICATION(ossim-elevation-manager-test COMMAND_LINE COMPONENT_NAME ossim SOURCE_FILES ossim-elevation-manager-test.cpp)
OSSIM_SETUP_APPLICATION(ossim-image-elevation-test COMMAND_LINE COMPONENT_NAME ossim SOURCE_FILES ossim-image-elevation-test.cpp)
OSSIM_SETUP_APPLICATION(ossim-threaded-elevation-test COMMAND_LINE COMPONENT_NAME ossim SOURCE_FILES ossim-threaded-elevation-test.cpp)
OSSIM_SETUP_APPLICATION(ossim-tiled-elevation-test COMMAND_LINE COMPONENT_NAME ossim SOURCE_FILES ossim-tiled-elevation-test.cpp)
