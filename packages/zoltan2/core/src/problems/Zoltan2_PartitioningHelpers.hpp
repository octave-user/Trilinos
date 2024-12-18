// @HEADER
// *****************************************************************************
//   Zoltan2: A package of combinatorial algorithms for scientific computing
//
// Copyright 2012 NTESS and the Zoltan2 contributors.
// SPDX-License-Identifier: BSD-3-Clause
// *****************************************************************************
// @HEADER

/*! \file Zoltan2_PartitioningHelpers.hpp
    \brief Helper functions for Partitioning Problems
*/

#ifndef _ZOLTAN2_PARTITIONINGHELPERS_HPP_
#define _ZOLTAN2_PARTITIONINGHELPERS_HPP_

#ifdef _MSC_VER
#define NOMINMAX
#include <windows.h>
#endif


#include <Zoltan2_PartitioningSolution.hpp>
#include <Zoltan2_AlltoAll.hpp>

namespace Zoltan2 {

/*! \brief  From a PartitioningSolution, get a list of IDs to be imported.
 *          Assumes part numbers in PartitioningSolution are to be used
 *          as rank numbers.
 *
 *          The list returned by solution.getPartListView() is an export list,
 *          detailing to which part each ID should move.  This method provides
 *          a new list, listing the IDs to be imported
 *          to this processor.
 *
 *          \param imports on return is the list of global Ids assigned to
 *                 this process under the Solution.
 *
 *          TODO:  Add a version that takes a Mapping solution as well for k!=p.
*/

template <typename SolutionAdapter, typename DataAdapter>
size_t getImportList(
   const PartitioningSolution<SolutionAdapter> &solution,
   const DataAdapter * const data,
   ArrayRCP<typename DataAdapter::gno_t> &imports // output
)
{
  typedef typename PartitioningSolution<SolutionAdapter>::part_t part_t;
  typedef typename PartitioningSolution<SolutionAdapter>::gno_t gno_t;

  size_t numParts = solution.getActualGlobalNumberOfParts();
  int numProcs = solution.getCommunicator()->getSize();

  if (numParts > size_t(numProcs)) {
    char msg[256];
    sprintf(msg, "Number of parts %lu exceeds number of ranks %d; "
                 "%s requires a MappingSolution for this case\n",
                  numParts, numProcs, __func__);
    throw std::logic_error(msg);
  }

  size_t localNumIds = data->getLocalNumIDs();
  const typename DataAdapter::gno_t *gids = NULL;
  data->getIDsView(gids);

  const part_t *parts = solution.getPartListView();

  // How many ids to each process?
  Array<int> counts(numProcs, 0);
  for (size_t i=0; i < localNumIds; i++)
    counts[parts[i]]++;

  Array<gno_t> offsets(numProcs+1, 0);
  for (int i=1; i <= numProcs; i++){
    offsets[i] = offsets[i-1] + counts[i-1];
  }

  Array<typename DataAdapter::gno_t> gidList(localNumIds);
  for (size_t i=0; i < localNumIds; i++) {
    gno_t idx = offsets[parts[i]];
    gidList[idx] = gids[i];
    offsets[parts[i]] = idx + 1;
  }

  Array<int> recvCounts(numProcs, 0);
  try {
    AlltoAllv<typename DataAdapter::gno_t>(*(solution.getCommunicator()),
                      *(solution.getEnvironment()),
                      gidList(), counts(), imports, recvCounts());
  }
  Z2_FORWARD_EXCEPTIONS;

  return imports.size();
}

} // namespace Zoltan2

#endif // _ZOLTAN2_PARTITIONINGHELPERS_HPP_
