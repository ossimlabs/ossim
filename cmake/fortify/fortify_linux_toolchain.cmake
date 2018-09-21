INCLUDE (CMakeForceCompiler)

SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_VERSION 1)

#specify the compilers
SET(CMAKE_C_COMPILER ${CMAKE_SOURCE_DIR}/fortify/fortify_cc)
SET(CMAKE_CXX_COMPILER ${CMAKE_SOURCE_DIR}/fortify/fortify_cxx)
SET(CMAKE_AR_COMPILER ${CMAKE_SOURCE_DIR}/fortify/fortify_ar)
