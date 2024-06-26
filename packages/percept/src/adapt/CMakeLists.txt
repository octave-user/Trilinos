
INCLUDE(TribitsLibraryMacros)

ADD_SUBDIRECTORY(sierra_element)

TRIBITS_CONFIGURE_FILE(${PACKAGE_NAME}_config.h)

SET(HEADERS "")
SET(SOURCES "")

TRIBITS_SET_AND_INC_DIRS(DIR ${CMAKE_CURRENT_SOURCE_DIR})
APPEND_GLOB(HEADERS ${DIR}/*.hpp)
APPEND_GLOB(SOURCES ${DIR}/*.cpp)

#TRIBITS_SET_AND_INC_DIRS(DIR ${CMAKE_CURRENT_SOURCE_DIR}/main)
#APPEND_GLOB(HEADERS ${DIR}/*.hpp)
#APPEND_GLOB(SOURCES ${DIR}/*.cpp)

#IF( TPL_ENABLE_OpenNURBS)
IF( STK_ADAPT_HAS_GEOMETRY )
  TRIBITS_SET_AND_INC_DIRS(DIR ${CMAKE_CURRENT_SOURCE_DIR}/geometry)
  APPEND_GLOB(HEADERS ${DIR}/*.hpp)
  APPEND_GLOB(SOURCES ${DIR}/*.cpp)
ENDIF()

TRIBITS_SET_AND_INC_DIRS(DIR ${CMAKE_CURRENT_SOURCE_DIR}/sierra_element)
APPEND_GLOB(HEADERS ${DIR}/*.hpp)
APPEND_GLOB(SOURCES ${DIR}/*.cpp)

# Must glob the binary dir last to get all of the auto-generated headers
TRIBITS_SET_AND_INC_DIRS(DIR ${CMAKE_CURRENT_BINARY_DIR})
APPEND_GLOB(HEADERS ${DIR}/*.hpp)
APPEND_SET(HEADERS ${DIR}/${PACKAGE_NAME}_config.h )

#APPEND_GLOB(HEADERS ${DIR}/*.hpp)
#APPEND_GLOB(SOURCES ${DIR}/*.cpp)

TRIBITS_SET_AND_INC_DIRS(DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

TRIBITS_ADD_LIBRARY(
	adapt
	HEADERS	${HEADERS}
  DEPLIBS stk_percept
	SOURCES ${SOURCES}
)

INSTALL(FILES ${HEADERS} DESTINATION
${CMAKE_INSTALL_PREFIX}/include/stk_adapt/)

ADD_SUBDIRECTORY(main)

#INCLUDE(TribitsAddExecutableAndTest)

#TRIBITS_ADD_EXECUTABLE(
#	adapt
#  COMM serial mpi
#	SOURCES ${SOURCES}
#  DIRECTORY .
#)
