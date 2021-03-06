/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 *     Copyright 2015 Couchbase, Inc.
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

/*
 * Unit tests for the cb::getopt implementation of getopt / getopt_long.
 */

#include <folly/portability/GTest.h>
#include <platform/cb_malloc.h>
#include <platform/cbassert.h>
#include <platform/getopt.h>
#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

char** vec2array(const std::vector<std::string>& vec) {
    auto** arr = new char*[vec.size()];
    for (unsigned int ii = 0; ii < (unsigned int)vec.size(); ++ii) {
        arr[ii] = cb_strdup(vec[ii].c_str());
    }
    return arr;
}

static void release(char** arr, size_t size) {
    for (size_t ii = 0; ii < size; ++ii) {
        cb_free(arr[ii]);
    }
    delete[] arr;
}

typedef std::vector<std::string> getoptvec;

class GetoptTest : public ::testing::Test {
protected:
    void SetUp() override {
        cb::getopt::mute_stderr();
        cb::getopt::reset();
    }
};

TEST_F(GetoptTest, NormalWithOneUnknownProvided) {
    getoptvec vec;
    vec.push_back("program");
    vec.push_back("-a");
    vec.push_back("-b");
    auto argc = (int)vec.size();
    auto** argv = vec2array(vec);
    ASSERT_EQ(1, cb::getopt::optind);
    ASSERT_EQ('a', cb::getopt::getopt(argc, argv, "a"));
    ASSERT_EQ(2, cb::getopt::optind);
    ASSERT_EQ('?', cb::getopt::getopt(argc, argv, "a"));
    ASSERT_EQ(3, cb::getopt::optind);

    release(argv, vec.size());
}

TEST_F(GetoptTest, NormalWithTermination) {
    getoptvec vec;
    vec.push_back("program");
    vec.push_back("-a");
    vec.push_back("--");
    vec.push_back("-b");
    auto argc = (int)vec.size();
    auto** argv = vec2array(vec);
    ASSERT_EQ('a', cb::getopt::getopt(argc, argv, "a"));
    ASSERT_EQ(-1, cb::getopt::getopt(argc, argv, "a"));
    ASSERT_EQ(3, cb::getopt::optind);

    release(argv, vec.size());
}

TEST_F(GetoptTest, RegressionTestFromEpEngine) {
    getoptvec vec;
    vec.push_back("..\\memcached\\engine_testapp");
    vec.push_back("-E");
    vec.push_back("ep.dll");
    vec.push_back("-T");
    vec.push_back("ep_testsuite.dll");
    vec.push_back("-e");
    vec.push_back("flushall_enabled=true;ht_size=13;ht_locks=7");
    vec.push_back("-v");
    vec.push_back("-C");
    vec.push_back("7");
    vec.push_back("-s");
    vec.push_back("foo");

    auto argc = (int)vec.size();
    auto** argv = vec2array(vec);

    ASSERT_EQ('E', cb::getopt::getopt(argc, argv, "E:T:e:vC:s"));
    ASSERT_STREQ(argv[2], cb::getopt::optarg);
    ASSERT_EQ('T', cb::getopt::getopt(argc, argv, "E:T:e:vC:s"));
    ASSERT_STREQ(argv[4], cb::getopt::optarg);
    ASSERT_EQ('e', cb::getopt::getopt(argc, argv, "E:T:e:vC:s"));
    ASSERT_STREQ(argv[6], cb::getopt::optarg);
    ASSERT_EQ('v', cb::getopt::getopt(argc, argv, "E:T:e:vC:s"));
    ASSERT_EQ('C', cb::getopt::getopt(argc, argv, "E:T:e:vC:s"));
    ASSERT_STREQ(argv[9], cb::getopt::optarg);
    ASSERT_EQ('s', cb::getopt::getopt(argc, argv, "E:T:e:vC:s"));
    ASSERT_EQ(-1, cb::getopt::getopt(argc, argv, "E:T:e:vC:s"));
    ASSERT_EQ(11, cb::getopt::optind);

    release(argv, vec.size());
}

TEST_F(GetoptTest, TestLongOptions) {
    static cb::getopt::option long_options[] = {
            {"first", cb::getopt::no_argument, nullptr, 'f'},
            {"second", cb::getopt::no_argument, nullptr, 's'},
            {"third", cb::getopt::no_argument, nullptr, 't'},
            {nullptr, 0, nullptr, 0}};

    getoptvec vec;

    vec.push_back("getopt_long_test");
    vec.push_back("--first");
    vec.push_back("--wrong");
    vec.push_back("--second");
    vec.push_back("--third");

    auto argc = (int)vec.size();
    auto** argv = vec2array(vec);

    int option_index = 0;
    int c = 1;

    bool first, second, third;
    first = second = third = false;

    while ((c = cb::getopt::getopt_long(
                    argc, argv, "fst", long_options, &option_index)) != -1) {
        switch (c) {
        case 'f':
            first = true;
            break;
        case 's':
            second = true;
            break;
        case 't':
            third = true;
            break;
        }
    }
    EXPECT_TRUE(first) << "--first not found";
    EXPECT_TRUE(second) << "--second not found";
    EXPECT_TRUE(third) << "--third not found";

    release(argv, vec.size());
}

TEST_F(GetoptTest, TestLongOptionsWithArguments) {
    static cb::getopt::option long_options[] = {
            {"host", cb::getopt::required_argument, nullptr, 'h'},
            {"port", cb::getopt::required_argument, nullptr, 'p'},
            {nullptr, 0, nullptr, 0}};

    getoptvec vec;

    vec.push_back("TestLongOptionsWithArguments");
    vec.push_back("--host=localhost");
    vec.push_back("--port");
    vec.push_back("11210");

    auto argc = (int)vec.size();
    auto** argv = vec2array(vec);

    int option_index = 0;
    int c;

    std::string host;
    std::string port;

    while ((c = cb::getopt::getopt_long(
                    argc, argv, "h:p:", long_options, &option_index)) != -1) {
        switch (c) {
        case 'h':
            ASSERT_NE(nullptr, cb::getopt::optarg)
                    << "host should have argument";
            host.assign(cb::getopt::optarg);
            break;
        case 'p':
            ASSERT_NE(nullptr, cb::getopt::optarg)
                    << "port should have argument";
            port.assign(cb::getopt::optarg);
            break;
        default:
            FAIL() << "getopt_long returned " << char(c);
        }
    }

    EXPECT_EQ(host, "localhost");
    EXPECT_EQ(port, "11210");

    release(argv, vec.size());
}

TEST_F(GetoptTest, TestLongOptionsWithMissingLastArguments) {
    static cb::getopt::option long_options[] = {
            {"port", cb::getopt::required_argument, nullptr, 'p'},
            {nullptr, 0, nullptr, 0}};

    getoptvec vec;

    vec.push_back("TestLongOptionsWithMissingLastArguments");
    vec.push_back("--port");

    auto argc = (int)vec.size();
    auto** argv = vec2array(vec);

    int option_index = 0;
    ASSERT_EQ('?',
              cb::getopt::getopt_long(
                      argc, argv, "p:", long_options, &option_index));

    release(argv, vec.size());
}

TEST_F(GetoptTest, TestLongOptionsWithOptionalArguments) {
    static cb::getopt::option long_options[] = {
            {"none", cb::getopt::optional_argument, nullptr, 'n'},
            {"with", cb::getopt::optional_argument, nullptr, 'w'},
            {nullptr, 0, nullptr, 0}};

    getoptvec vec;

    vec.push_back("TestLongOptionsWithOptionalArguments");
    vec.push_back("--none");
    vec.push_back("--with=true");

    auto argc = (int)vec.size();
    auto** argv = vec2array(vec);

    int option_index = 0;
    int c;
    bool none = false;
    bool with = false;

    while ((c = cb::getopt::getopt_long(
                    argc, argv, "n:w:", long_options, &option_index)) != -1) {
        switch (c) {
        case 'n':
            ASSERT_EQ(nullptr, cb::getopt::optarg);
            none = true;
            break;
        case 'w':
            ASSERT_STREQ("true", cb::getopt::optarg);
            with = true;
            break;
        default:
            FAIL() << "getopt_long returned " << char(c);
        }
    }

    EXPECT_TRUE(none);
    EXPECT_TRUE(with);

    release(argv, vec.size());
}
