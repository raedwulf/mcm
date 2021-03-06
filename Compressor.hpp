/*	MCM file compressor

	Copyright (C) 2013, Google Inc.
	Authors: Mathieu Chartier

	LICENSE

    This file is part of the MCM file compressor.

    MCM is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    MCM is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with MCM.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _COMPRESS_HPP_
#define _COMPRESS_HPP_

#include <cmath>
#include <map>
#include <vector>
#include <iostream>
#include <map>
#include <list>
#include <ctime>
#include <iomanip>
#include <malloc.h>

#include "Stream.hpp"
#include "Util.hpp"

class ProgressMeter {
	uint64_t count, prev_size, prev_count;
	clock_t start, prev_time;
	bool encode;
public:
	ProgressMeter(bool encode = true)
		: count(0)
		, prev_size(0)
		, prev_count(0)
		, encode(encode) {
		prev_time = start = clock();
	}

	forceinline uint64_t getCount() const {
		return count;
	}

	forceinline uint64_t addByte() {
		return ++count;
	}

	// Surprisingly expensive to call...
	void printRatio(uint64_t comp_size, const std::string& extra) {
		_mm_empty();
		const auto cur_ratio = double(comp_size - prev_size) / (count - prev_count);
		const auto ratio = double(comp_size) / count;
		auto cur_time = clock();
		auto time_delta = cur_time - start;
		if (!time_delta) ++time_delta;

		const size_t rate = size_t(double(count / KB) / (double(time_delta) / double(CLOCKS_PER_SEC)));
		std::cout
			<< count / KB << "KB " << (encode ? "->" : "<-") << " "
			<< comp_size / KB << "KB " << rate << "KB/s ratio: " << std::setprecision(5) << std::fixed << ratio << extra.c_str() << " cratio: " << cur_ratio << "\t\r";
		prev_size = comp_size;
		prev_count = count;
		prev_time = cur_time;
	}

	forceinline void addBytePrint(uint64_t total, const char* extra = "") {
		if (!(addByte() & 0xFFFF)) {
			// 4 updates per second.
			if (clock() - prev_time > 250) {
				printRatio(total, extra);
			}
		}
	}
};

class Compressor {
public:
	class Factory {
	public:
		virtual Compressor* create() = 0;
	};

	template <typename CompressorType>
	class FactoryOf : public Compressor::Factory {
		virtual Compressor* create() {
			return new CompressorType();
		}
	};

	virtual void compress(ReadStream* in, WriteStream* out) = 0;
	virtual void decompress(ReadStream* in, WriteStream* out) = 0;
};

// Simple uncompressed compressor.
class Store : public Compressor {
public:
	virtual void compress(ReadStream* in, WriteStream* out) {
		BufferedStreamReader<16 * KB> sin(in);
		BufferedStreamWriter<16 * KB> sout(out);
		for (;;) {
			int c = sin.get();
			if (c == EOF) {
				break;
			}
			sout.put(c);
		}
		sout.flush();
	}

	virtual void decompress(ReadStream* in, WriteStream* out) {
		BufferedStreamReader<16 * KB> sin(in);
		BufferedStreamWriter<16 * KB> sout(out);
		for (;;) {
			int c = sin.get();
			if (c == EOF) {
				break;
			}
			sout.put(c);
		}
		sout.flush();
	}
};

#endif
