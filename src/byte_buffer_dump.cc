/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 *     Copyright 2016 Couchbase, Inc.
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
#include <platform/byte_buffer_dump.h>
#include <iomanip>

static void printByte(uint8_t b, std::ostream& out) {
    uint32_t val = b & 0xff;
    if (b < 0x10) {
        out << " 0x0";
    } else {
        out << " 0x";
    }

    out.flags(std::ios::hex);
    out << val;
    out.flags(std::ios::dec);

    if (isprint((int)b)) {
        out << " ('" << (char)b << "')";
    } else {
        out << "      ";
    }
    out << "    |";
}

static void dump(std::ostream& out, const cb::const_byte_buffer buffer) {
    out << std::endl;
    out <<
        "      Byte/     0       |       1       |       2       |       3       |" <<
        std::endl;
    out <<
        "         /              |               |               |               |" <<
        std::endl;
    out <<
        "        |0 1 2 3 4 5 6 7|0 1 2 3 4 5 6 7|0 1 2 3 4 5 6 7|0 1 2 3 4 5 6 7|";

    size_t ii = 0;
    for (; ii < buffer.len; ++ii) {
        if (ii % 4 == 0) {
            out << std::endl;
            out <<
                "        +---------------+---------------+---------------+---------------+" <<
                std::endl;
            out.setf(std::ios::right);
            out << std::setw(8) << ii << "|";
            out.setf(std::ios::fixed);
        }
        printByte(buffer.buf[ii], out);
    }

    out << std::endl;
    if (ii % 4 != 0) {
        out << "        ";
        for (size_t jj = 0; jj < ii % 4; ++jj) {
            out << "+---------------";
        }
        out << "+" << std::endl;
    } else {
        out << "        +---------------+---------------+---------------+---------------+"
            << std::endl;
    }
}

PLATFORM_PUBLIC_API
std::ostream& operator<<(std::ostream& out, const cb::byte_buffer& buffer) {
    cb::const_byte_buffer buf { buffer.buf, buffer.len };
    dump(out, buf);
    return out;
}

PLATFORM_PUBLIC_API
std::ostream& operator<<(std::ostream& out, const cb::const_byte_buffer& buffer) {
    dump(out, buffer);
    return out;
}
