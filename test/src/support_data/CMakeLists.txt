# $Id: CMakeLists.txt 23496 2015-08-28 15:26:18Z okramer $

OSSIM_SETUP_APPLICATION(ossim-aux-dot-xml-test COMMAND_LINE COMPONENT_NAME ossim SOURCE_FILES ossim-aux-dot-xml-test.cpp)
OSSIM_SETUP_APPLICATION(ossim-envi-hdr-test COMMAND_LINE COMPONENT_NAME ossim SOURCE_FILES ossim-envi-hdr-test.cpp)
OSSIM_SETUP_APPLICATION(ossim-fgdc-txt-doc-test COMMAND_LINE COMPONENT_NAME ossim SOURCE_FILES ossim-fgdc-txt-doc-test.cpp)
OSSIM_SETUP_APPLICATION(ossim-srtm-support-data-test COMMAND_LINE COMPONENT_NAME ossim SOURCE_FILES ossim-srtm-support-data-test.cpp)
OSSIM_SETUP_APPLICATION(ossim-tiff-info-test COMMAND_LINE COMPONENT_NAME ossim SOURCE_FILES ossim-tiff-info-test.cpp)
OSSIM_SETUP_APPLICATION(ossim-wavelength-test COMMAND_LINE COMPONENT_NAME ossim SOURCE_FILES ossim-wavelength-test.cpp)

