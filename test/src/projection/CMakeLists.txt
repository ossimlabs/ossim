# $Id: CMakeLists.txt 23496 2015-08-28 15:26:18Z okramer $

OSSIM_SETUP_APPLICATION(ossim-epsg-factory-test INSTALL COMMAND_LINE COMPONENT_NAME ossim SOURCE_FILES ossim-epsg-factory-test.cpp)
OSSIM_SETUP_APPLICATION(ossim-eq-projection-test INSTALL COMMAND_LINE COMPONENT_NAME ossim SOURCE_FILES ossim-eq-projection-test.cpp)
OSSIM_SETUP_APPLICATION(ossim-image-geometry-test INSTALL COMMAND_LINE COMPONENT_NAME ossim SOURCE_FILES ossim-image-geometry-test.cpp)
OSSIM_SETUP_APPLICATION(ossim-nitf-rsm-model-test INSTALL COMMAND_LINE COMPONENT_NAME ossim SOURCE_FILES ossim-nitf-rsm-model-test.cpp)
OSSIM_SETUP_APPLICATION(ossim-projection-factory-test INSTALL COMMAND_LINE COMPONENT_NAME ossim SOURCE_FILES ossim-projection-factory-test.cpp)
OSSIM_SETUP_APPLICATION(ossim-projection-test INSTALL COMMAND_LINE COMPONENT_NAME ossim SOURCE_FILES ossim-projection-test.cpp)
OSSIM_SETUP_APPLICATION(ossim-wkt-test INSTALL COMMAND_LINE COMPONENT_NAME ossim SOURCE_FILES ossim-wkt-test.cpp)
OSSIM_SETUP_APPLICATION(ossim-wkt-proj-factory-test INSTALL COMMAND_LINE COMPONENT_NAME ossim SOURCE_FILES ossim-wkt-proj-factory-test.cpp)
