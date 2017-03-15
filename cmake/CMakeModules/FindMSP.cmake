SET(CMAKE_FIND_FRAMEWORK "LAST")
#FIND_PATH(MSP_INCLUDE_DIRS msp/Mensuration/MensurationService.h
#        PATHS
#        ${OSSIM_DEPENDENCIES}/include
#		${OSSIM_INSTALL_PREFIX}/include
#)

# Need to copy pattern from FindQt5 for extensive lib list...
#set(MSP_NAMES ${MSP_NAMES} ??? )
#find_library(MSP_LIBRARY NAMES ${MSP_NAMES}
#   PATHS
#      ${OSSIM_DEPENDENCIES}/lib
#      ${OSSIM_INSTALL_PREFIX}/lib${LIBSUFFIX}
#      ${OSSIM_INSTALL_PREFIX}
#)

#---
# This function sets MSP_FOUND if variables are valid.
#--- 
#include(FindPackageHandleStandardArgs)
#find_package_handle_standard_args( MSP DEFAULT_MSG 
#                                   MSP_LIBRARY 
#                                   MSP_INCLUDE_DIRS )

# 
# Temporary expect an ENV var called MSP_HOME until fully integrated:
set (MSP_INCLUDE_DIRS ${MSP_HOME}/include)
set (MSP_LIBRARIES 
    ${MSP_HOME}/lib/libCSM_PCAPI.so                           
    ${MSP_HOME}/lib/libMSPnitf.so
    ${MSP_HOME}/lib/libMSPasdetre.so                          
    ${MSP_HOME}/lib/libMSPntmtre.so
    ${MSP_HOME}/lib/libMSPCCSUtils.so                         
    ${MSP_HOME}/lib/libMSPOutputMethodService.so
    ${MSP_HOME}/lib/libMSPcoordconverter.so                   
    ${MSP_HOME}/lib/libMSPPointExtractionService.so
    ${MSP_HOME}/lib/libMSPCoordinateConversionService.so      
    ${MSP_HOME}/lib/libMSPRageServiceUtils.so
    ${MSP_HOME}/lib/libMSPCovarianceService.so                
    ${MSP_HOME}/lib/libMSPrage.so
    ${MSP_HOME}/lib/libMSPcsisd.so                            
    ${MSP_HOME}/lib/libMSPrageutilities.so
    ${MSP_HOME}/lib/libMSPcsm.so                              
    ${MSP_HOME}/lib/libMSPrsme.so
    ${MSP_HOME}/lib/libMSPcsmutil.so                          
    ${MSP_HOME}/lib/libMSPRsmGeneratorService.so
    ${MSP_HOME}/lib/libMSPdei.so                              
    ${MSP_HOME}/lib/libMSPrsmg.so
    ${MSP_HOME}/lib/libMSPDEIUtil.so                          
    ${MSP_HOME}/lib/libMSPrutil.so
    ${MSP_HOME}/lib/libMSPdtcc.so                             
    ${MSP_HOME}/lib/libMSPSensorModelService.so
    ${MSP_HOME}/lib/libMSPgeometry.so                         
    ${MSP_HOME}/lib/libMSPSensorSpecificService.so
    ${MSP_HOME}/lib/libMSPImagingGeometryService.so           
    ${MSP_HOME}/lib/libMSPSourceSelectionService.so
    ${MSP_HOME}/lib/libMSPjson.so                             
    ${MSP_HOME}/lib/libMSPSScovmodel.so
    ${MSP_HOME}/lib/libMSPlas.so                              
    ${MSP_HOME}/lib/libMSPSSrutil.so
    ${MSP_HOME}/lib/libMSPmath.so                             
    ${MSP_HOME}/lib/libMSPSupportDataService.so
    ${MSP_HOME}/lib/libMSPmens.so                             
    ${MSP_HOME}/lib/libMSPTerrainService.so
    ${MSP_HOME}/lib/libMSPMensurationService.so               
    ${MSP_HOME}/lib/libMSPutilities.so
    ${MSP_HOME}/lib/libMSPMensurationSessionRecordService.so  
    ${MSP_HOME}/lib/libMSPwriteRsmNitf.so
    ${MSP_HOME}/lib/libMSPMSPVersionUtils.so                  
    ${MSP_HOME}/lib/libMSPmtdCommon.so
) 

set (MSP_FOUND ON) 

#if(MSP_FOUND)
#   set( MSP_LIBRARIES ${MSP_LIBRARY} )
#else( MSP_FOUND )
#   if( NOT MSP_FIND_QUIETLY )
#      message( WARNING "Could not find MSP" )
#   endif( NOT MSP_FIND_QUIETLY )
#endif(MSP_FOUND)

#if( NOT MSP_FIND_QUIETLY )
#   message( STATUS "MSP_INCLUDE_DIRS=${MSP_INCLUDE_DIRS}" )
#   message( STATUS "MSP_LIBRARY=${MSP_LIBRARY}" )
#endif( NOT MSP_FIND_QUIETLY )

mark_as_advanced(MSP_INCLUDE_DIRS MSP_LIBRARY)
