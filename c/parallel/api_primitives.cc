//------------------------------------------------------------------------------
// Copyright 2019 H2O.ai
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//------------------------------------------------------------------------------
#include "parallel/api_primitives.h"
namespace dt {


/**
 * Create NThreads object with the number of threads calculated
 * from the number of iterations and the minimum number of
 * iterations per thread.
 */
NThreads nthreads_from_niters(size_t niters,
                              size_t min_iters_per_thread /* = 1000 */)
{
  bool go_parallel = niters > min_iters_per_thread;
  size_t nthreads = go_parallel? niters / min_iters_per_thread : 1;
  return NThreads(nthreads);
}


} // namespace dt
