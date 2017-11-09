#---
# This module sets MSP_FOUND if variables are valid.
#--- 

SET(CMAKE_FIND_FRAMEWORK "LAST")

##############################################################

function(FIND_MSP_LIBRARY LIBNAME LIBSOK)
  find_library( TARGET_LIBRARY_${LIBNAME} NAMES "${LIBNAME}" PATHS
        ${MSP_HOME}/lib 
        /usr/lib64
        /usr/lib
        /usr/local/lib )
  if (TARGET_LIBRARY_${LIBNAME})
    set (MSP_LIBRARIES ${MSP_LIBRARIES} ${TARGET_LIBRARY_${LIBNAME}} PARENT_SCOPE)
  else()
    set(${LIBSOK} "NO" PARENT_SCOPE)
  endif()
endfunction(FIND_MSP_LIBRARY)

##############################################################

set(MSP_FOUND "NO")
set(OSSIM_HAS_MSP 0)

FIND_PATH(MSP_INCLUDE_DIRS Mensuration/MensurationService.h
        PATHS
        ${MSP_HOME}/include
        /usr/include
		    /usr/local/include)

if (MSP_INCLUDE_DIRS)
  set (MSP_INCLUDE_DIRS ${MSP_INCLUDE_DIRS} 
                        ${MSP_INCLUDE_DIRS}/CoordinateConversion
                        ${MSP_INCLUDE_DIRS}/Terrain
                        ${MSP_INCLUDE_DIRS}/common
                        ${MSP_INCLUDE_DIRS}/common/utilities
                        ${MSP_INCLUDE_DIRS}/common/geometry
                        ${MSP_INCLUDE_DIRS}/common/math
                        ${MSP_INCLUDE_DIRS}/common/csmutil
                        ${MSP_INCLUDE_DIRS}/common/ntmtre
                        ${MSP_INCLUDE_DIRS}/common/nitf
                        ${MSP_INCLUDE_DIRS}/common/deiutil
                        ${MSP_INCLUDE_DIRS}/common/dtcc
                        ${MSP_INCLUDE_DIRS}/common/csm)
  set(LIBS_OK "YES")
  FIND_MSP_LIBRARY(MSPPointExtractionService LIBS_OK)
  FIND_MSP_LIBRARY(MSPSensorModelService LIBS_OK)
  FIND_MSP_LIBRARY(MSPSourceSelectionService LIBS_OK)
  FIND_MSP_LIBRARY(MSPCovarianceService LIBS_OK)
  FIND_MSP_LIBRARY(MSPSupportDataService LIBS_OK)
  FIND_MSP_LIBRARY(MSPTerrainService LIBS_OK)
  FIND_MSP_LIBRARY(MSPCCSUtils LIBS_OK)
  FIND_MSP_LIBRARY(MSPCoordinateConversionService LIBS_OK)
  FIND_MSP_LIBRARY(MSPOutputMethodService LIBS_OK)
  FIND_MSP_LIBRARY(MSPasdetre LIBS_OK)
  FIND_MSP_LIBRARY(MSPcoordconverter LIBS_OK)
  FIND_MSP_LIBRARY(MSPcsisd LIBS_OK)
  FIND_MSP_LIBRARY(MSPcsm LIBS_OK)
  FIND_MSP_LIBRARY(MSPcsmutil LIBS_OK)
  FIND_MSP_LIBRARY(MSPdtcc LIBS_OK)
  FIND_MSP_LIBRARY(MSPlas LIBS_OK)
  FIND_MSP_LIBRARY(MSPgeometry LIBS_OK)
  FIND_MSP_LIBRARY(MSPmath LIBS_OK)
  FIND_MSP_LIBRARY(MSPnitf LIBS_OK)
  FIND_MSP_LIBRARY(MSPntmtre LIBS_OK)
  FIND_MSP_LIBRARY(MSPrage LIBS_OK)
  FIND_MSP_LIBRARY(MSPRageServiceUtils LIBS_OK)
  FIND_MSP_LIBRARY(MSPrageutilities LIBS_OK)
  FIND_MSP_LIBRARY(MSPSScovmodel LIBS_OK)
  FIND_MSP_LIBRARY(MSPSSrutil LIBS_OK)
  FIND_MSP_LIBRARY(MSPutilities LIBS_OK)
  FIND_MSP_LIBRARY(CSM_PCAPI LIBS_OK)
  FIND_MSP_LIBRARY(MSPrsme LIBS_OK)
  FIND_MSP_LIBRARY(MSPRsmGeneratorService LIBS_OK)
  FIND_MSP_LIBRARY(MSPdei LIBS_OK)
  FIND_MSP_LIBRARY(MSPrsmg LIBS_OK)
  FIND_MSP_LIBRARY(MSPDEIUtil LIBS_OK)
  FIND_MSP_LIBRARY(MSPrutil LIBS_OK)
  FIND_MSP_LIBRARY(MSPSensorSpecificService LIBS_OK)
  FIND_MSP_LIBRARY(MSPImagingGeometryService LIBS_OK)
  FIND_MSP_LIBRARY(MSPjson LIBS_OK)
  FIND_MSP_LIBRARY(MSPmens LIBS_OK)
  FIND_MSP_LIBRARY(MSPTerrainService LIBS_OK)
  FIND_MSP_LIBRARY(MSPMensurationService LIBS_OK)
  FIND_MSP_LIBRARY(MSPMensurationSessionRecordService LIBS_OK)
  FIND_MSP_LIBRARY(MSPwriteRsmNitf LIBS_OK)
  FIND_MSP_LIBRARY(MSPMSPVersionUtils LIBS_OK)
  FIND_MSP_LIBRARY(MSPmtdCommon LIBS_OK)
  FIND_MSP_LIBRARY(pthread  LIBS_OK)
  FIND_MSP_LIBRARY(dl   LIBS_OK)

# These are optional. Include only if present:
  FIND_MSP_LIBRARY(MSPsupportdata DUMMY_ARG) 
  FIND_MSP_LIBRARY(MSPHardCopyService DUMMY_ARG)

  if( LIBS_OK )
    set(MSP_FOUND "YES")
    set(OSSIM_HAS_MSP 1)
    message("-- MSP_INCLUDE_DIRS = ${MSP_INCLUDE_DIRS}")
    message("-- MSP_LIBRARIES = ${MSP_LIBRARIES}")
  endif()

endif()

mark_as_advanced(MSP_INCLUDE_DIRS MSP_LIBRARIES)

