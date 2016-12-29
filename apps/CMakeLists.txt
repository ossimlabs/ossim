
FILE(GLOB OSSIM_APP_DIRS "ossim-*")
FILE(GLOB OSSIM_CONFIG_DIR "ossim-config*")
LIST(REMOVE_ITEM OSSIM_APP_DIRS ${OSSIM_CONFIG_DIR})

#set(TARGET_COMMON_LIBRARIES ${ossimDependentLibs})

FOREACH(f ${OSSIM_APP_DIRS})
	ADD_SUBDIRECTORY(${f})
ENDFOREACH(f)

#---
# Building curl dependant apps optional. default=OFF
#---
if(BUILD_OSSIM_CURL_APPS)
  add_subdirectory(curl_apps)
endif()
