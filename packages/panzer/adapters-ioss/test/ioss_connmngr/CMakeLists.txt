

TRIBITS_INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

SET(UNIT_TEST_DRIVER
  ${PANZER_UNIT_TEST_MAIN})
  
TRIBITS_ADD_EXECUTABLE_AND_TEST(
  tIOSSConnManager1
  SOURCES tIOSSConnManager.cpp ${UNIT_TEST_DRIVER}
  NUM_MPI_PROCS 1
  COMM serial mpi
  )

TRIBITS_ADD_EXECUTABLE_AND_TEST(
  tIOSSConnManager2
  SOURCES tIOSSConnManager.cpp ${UNIT_TEST_DRIVER}
  NUM_MPI_PROCS 2
  COMM serial mpi
  )
  
TRIBITS_ADD_EXECUTABLE_AND_TEST(
  tIOSSConnManager3
  SOURCES tIOSSConnManager.cpp ${UNIT_TEST_DRIVER}
  NUM_MPI_PROCS 3
  COMM serial mpi
  )
  
TRIBITS_COPY_FILES_TO_BINARY_DIR(
  tIOSSConnManagerRectanglePg
  SOURCE_FILES rectangle.pg
  EXEDEPS tIOSSConnManager1 tIOSSConnManager2 tIOSSConnManager3
)

TRIBITS_COPY_FILES_TO_BINARY_DIR(
  tIOSSConnManagerBrickPg
  SOURCE_FILES brick.pg
  EXEDEPS tIOSSConnManager1 tIOSSConnManager2 tIOSSConnManager3
)
