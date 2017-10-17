#---
# This module sets MSP_FOUND if variables are valid.
#--- 

SET(CMAKE_FIND_FRAMEWORK "LAST")

##############################################################

macro(FIND_MSP_LIBRARY LIBS_OK MYLIBRARYNAME)
  if (LIBS_OK)
     find_library( TARGET_LIBRARY_${MYLIBRARYNAME}
        NAMES "${MYLIBRARYNAME}"
        PATHS
        $ENV{MSP_HOME}/lib 
        /usr/lib64
        /usr/lib
        /usr/local/lib )
     if (TARGET_LIBRARY_${MYLIBRARYNAME})
         set (MSP_LIBRARIES ${MSP_LIBRARIES} ${TARGET_LIBRARY_${MYLIBRARYNAME}})
     else()
       set(LIBS_OK "NO")
     endif()
   endif()
endmacro(FIND_MSP_LIBRARY LIBS_OK MYLIBRARYNAME)

##############################################################

set(MSP_FOUND "NO")
set(OSSIM_HAS_MSP 0)

FIND_PATH(MSP_INCLUDE_DIRS Mensuration/MensurationService.h
        PATHS
        $ENV{MSP_HOME}/include
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
  FIND_MSP_LIBRARY(LIBS_OK MSPPointExtractionService)
  FIND_MSP_LIBRARY(LIBS_OK MSPSensorModelService)
  FIND_MSP_LIBRARY(LIBS_OK MSPSourceSelectionService)
  FIND_MSP_LIBRARY(LIBS_OK MSPCovarianceService)
  FIND_MSP_LIBRARY(LIBS_OK MSPSupportDataService)
  FIND_MSP_LIBRARY(LIBS_OK MSPTerrainService)
  FIND_MSP_LIBRARY(LIBS_OK MSPCCSUtils)
  FIND_MSP_LIBRARY(LIBS_OK MSPCoordinateConversionService)
  FIND_MSP_LIBRARY(LIBS_OK MSPOutputMethodService)
  FIND_MSP_LIBRARY(LIBS_OK MSPasdetre)
  FIND_MSP_LIBRARY(LIBS_OK MSPcoordconverter)
  FIND_MSP_LIBRARY(LIBS_OK MSPcsisd)
  FIND_MSP_LIBRARY(LIBS_OK MSPcsm)
  FIND_MSP_LIBRARY(LIBS_OK MSPcsmutil)
  FIND_MSP_LIBRARY(LIBS_OK MSPdtcc)
  FIND_MSP_LIBRARY(LIBS_OK MSPlas)
  FIND_MSP_LIBRARY(LIBS_OK MSPgeometry)
  FIND_MSP_LIBRARY(LIBS_OK MSPmath)
  FIND_MSP_LIBRARY(LIBS_OK MSPnitf)
  FIND_MSP_LIBRARY(LIBS_OK MSPntmtre)
  FIND_MSP_LIBRARY(LIBS_OK MSPrage)
  FIND_MSP_LIBRARY(LIBS_OK MSPRageServiceUtils)
  FIND_MSP_LIBRARY(LIBS_OK MSPrageutilities)
  FIND_MSP_LIBRARY(LIBS_OK MSPSScovmodel)
  FIND_MSP_LIBRARY(LIBS_OK MSPSSrutil)
  FIND_MSP_LIBRARY(LIBS_OK MSPutilities)
  FIND_MSP_LIBRARY(LIBS_OK CSM_PCAPI)
  FIND_MSP_LIBRARY(LIBS_OK MSPrsme)
  FIND_MSP_LIBRARY(LIBS_OK MSPRsmGeneratorService)
  FIND_MSP_LIBRARY(LIBS_OK MSPdei)
  FIND_MSP_LIBRARY(LIBS_OK MSPrsmg)
  FIND_MSP_LIBRARY(LIBS_OK MSPDEIUtil)
  FIND_MSP_LIBRARY(LIBS_OK MSPrutil)
  FIND_MSP_LIBRARY(LIBS_OK MSPSensorSpecificService)
  FIND_MSP_LIBRARY(LIBS_OK MSPImagingGeometryService)
  FIND_MSP_LIBRARY(LIBS_OK MSPjson)
  FIND_MSP_LIBRARY(LIBS_OK MSPmens)
  FIND_MSP_LIBRARY(LIBS_OK MSPTerrainService)
  FIND_MSP_LIBRARY(LIBS_OK MSPMensurationService)
  FIND_MSP_LIBRARY(LIBS_OK MSPMensurationSessionRecordService)
  FIND_MSP_LIBRARY(LIBS_OK MSPwriteRsmNitf)
  FIND_MSP_LIBRARY(LIBS_OK MSPMSPVersionUtils)
  FIND_MSP_LIBRARY(LIBS_OK MSPmtdCommon)
  FIND_MSP_LIBRARY(LIBS_OK pthread )
  FIND_MSP_LIBRARY(LIBS_OK dl  )

  if( LIBS_OK )
    set(MSP_FOUND "YES")
    set(OSSIM_HAS_MSP 1)
    #message("-- MSP_INCLUDE_DIRS = ${MSP_INCLUDE_DIRS}")
    #message("-- MSP_LIBRARIES = ${MSP_LIBRARIES}")
  endif()

endif()

mark_as_advanced(MSP_INCLUDE_DIRS MSP_LIBRARIES)

