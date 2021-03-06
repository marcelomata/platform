/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 *     Copyright 2020 Couchbase, Inc
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

#include <platform/cb_malloc.h>

#if defined(HAVE_JEMALLOC)
#include <jemalloc/jemalloc.h>
#endif

#include <folly/portability/GTest.h>

// This program is not linked with the arenas library, so we expect cb_malloc
// to return false for the is_using_arenas function
TEST(CbMallocDefault, cb_malloc_is_not_using_arenas) {
    EXPECT_FALSE(cb_malloc_is_using_arenas());
}

#if defined(HAVE_JEMALLOC)
// This program was built with je_malloc available, so we expect that the
// default cb_malloc will call down to jemalloc
TEST(CbMallocDefault, cb_malloc_is_jemalloc) {
    // Grab the current allocated/deallocated values for this thread
    uint64_t allocated, deallocated;
    size_t len = sizeof(allocated);
    je_mallctl("thread.allocated", &allocated, &len, nullptr, 0);
    len = sizeof(deallocated);
    je_mallctl("thread.deallocated", &deallocated, &len, nullptr, 0);

    // do an allocation and check je_malloc allocated increases
    auto* p = cb_malloc(512);

    uint64_t allocated_1;
    len = sizeof(allocated_1);
    je_mallctl("thread.allocated", &allocated_1, &len, nullptr, 0);

    EXPECT_EQ(allocated_1, allocated + 512);

    // do a deallocation and check je_malloc deallocated increases
    cb_free(p);

    uint64_t deallocated_1;
    len = sizeof(deallocated_1);
    je_mallctl("thread.deallocated", &deallocated_1, &len, nullptr, 0);
    EXPECT_EQ(deallocated_1, deallocated + 512);
}
#endif