

TRIBITS_INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

TRIBITS_INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../../utils)

ASSERT_DEFINED(NOX_ENABLE_ABSTRACT_IMPLEMENTATION_THYRA
  NOX_ENABLE_Epetra NOX_ENABLE_EpetraExt
  NOX_ENABLE_Stratimikos Stratimikos_ENABLE_AztecOO)

SET(UNIT_TEST_DRIVER ${TEUCHOS_STD_UNIT_TEST_MAIN})

IF(NOX_ENABLE_ABSTRACT_IMPLEMENTATION_THYRA AND
    NOX_ENABLE_THYRA_EPETRA_ADAPTERS AND
    NOX_ENABLE_Epetra AND NOX_ENABLE_EpetraExt AND
    NOX_ENABLE_Stratimikos AND Stratimikos_ENABLE_AztecOO)

  TRIBITS_ADD_EXECUTABLE_AND_TEST(
    Thyra_NonlinearSolver_NOX_2Dsim
    SOURCES ${UNIT_TEST_DRIVER} ModelEvaluator2DSim.hpp ModelEvaluator2DSim_def.hpp
            Thyra_NonlinearSolver_NOX_2Dsim.C
    NUM_MPI_PROCS 1
    )

  TRIBITS_ADD_EXECUTABLE_AND_TEST(
    Thyra_2Dsim
    SOURCES ModelEvaluator2DSim.hpp ModelEvaluator2DSim_def.hpp Thyra_2Dsim.C
    NUM_MPI_PROCS 1
    PASS_REGULAR_EXPRESSION "Test passed!"
    )

  TRIBITS_ADD_EXECUTABLE_AND_TEST(
    Thyra_Rosenbrock_AndersonAcceleration
    SOURCES ${UNIT_TEST_DRIVER}  ModelEvaluatorRosenbrock.hpp ModelEvaluatorRosenbrock.cpp
    Thyra_Rosenbrock_AndersonAcceleration.cpp Observer_PrintTest.hpp Observer_PrintTest.cpp
    NUM_MPI_PROCS 1
    PASS_REGULAR_EXPRESSION "Test passed!"
    )

  TRIBITS_ADD_EXECUTABLE_AND_TEST(
    Thyra_RowSumScaling
    SOURCES ModelEvaluator2DSim.hpp ModelEvaluator2DSim_def.hpp Thyra_RowSumScaling.C
    NUM_MPI_PROCS 1
    PASS_REGULAR_EXPRESSION "Test passed!"
    )

  TRIBITS_ADD_EXECUTABLE_AND_TEST(
    Thyra_RightScaling
    SOURCES ${UNIT_TEST_DRIVER} ModelEvaluator2DSim.hpp ModelEvaluator2DSim_def.hpp Thyra_RightScaling.C
    NUM_MPI_PROCS 1
    )

  TRIBITS_ADD_EXECUTABLE_AND_TEST(
    Thyra_Heq
    SOURCES  ModelEvaluatorHeq.hpp ModelEvaluatorHeq_def.hpp Thyra_Heq.C
    NUM_MPI_PROCS 1
    PASS_REGULAR_EXPRESSION "Test passed!"
    )

  TRIBITS_ADD_EXECUTABLE_AND_TEST(
    Thyra_Heq_DelayedStart
    SOURCES  ModelEvaluatorHeq.hpp ModelEvaluatorHeq_def.hpp Thyra_Heq_DelayedStart.C
    NUM_MPI_PROCS 1
    PASS_REGULAR_EXPRESSION "Test passed!"
    )

  TRIBITS_ADD_EXECUTABLE_AND_TEST(
    Thyra_PseudoTransient
    SOURCES  ${UNIT_TEST_DRIVER} Thyra_PseudoTransient.cpp ModelEvaluatorRosenbrock.hpp ModelEvaluatorRosenbrock.cpp
    NUM_MPI_PROCS 1
    )

  TRIBITS_ADD_EXECUTABLE_AND_TEST(
    Thyra_JFNK
    SOURCES  ${UNIT_TEST_DRIVER} ModelEvaluator2DSim.hpp ModelEvaluator2DSim_def.hpp Thyra_2Dsim_JFNK.C ModelEvaluator_LoggerDecorator.hpp
    NUM_MPI_PROCS 1
    )

  TRIBITS_ADD_EXECUTABLE_AND_TEST(
    Thyra_1DFEM
    SOURCES  ${UNIT_TEST_DRIVER} ModelEvaluator1DFEM.hpp ModelEvaluator1DFEM_def.hpp Thyra_1DFEM.C
    )

  TRIBITS_ADD_EXECUTABLE_AND_TEST(
    Thyra_1DPoisson_SingleStepSolver
    SOURCES  ${UNIT_TEST_DRIVER} ModelEvaluator1DPoisson.hpp ModelEvaluator1DPoisson_def.hpp Thyra_SingleStepSolver_1D.C
    )

  TRIBITS_ADD_EXECUTABLE_AND_TEST(
    Thyra_Rosenbrock_SafeguardedStepLineSearch
    SOURCES ${UNIT_TEST_DRIVER}  ModelEvaluatorRosenbrock.hpp ModelEvaluatorRosenbrock.cpp
    Thyra_Rosenbrock_SafeguardedStepLineSearch.cpp
    NUM_MPI_PROCS 1
    PASS_REGULAR_EXPRESSION "Test passed!"
    )

  TRIBITS_ADD_EXECUTABLE_AND_TEST(
    Thyra_Rosenbrock_SafeguardedDirectionLineSearch
    SOURCES ${UNIT_TEST_DRIVER}  ModelEvaluatorRosenbrock.hpp ModelEvaluatorRosenbrock.cpp
    Thyra_Rosenbrock_SafeguardedDirectionLineSearch.cpp
    NUM_MPI_PROCS 1
    PASS_REGULAR_EXPRESSION "Test passed!"
    )

ENDIF()
