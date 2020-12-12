#pragma once

namespace FileTapeLibrary {
	typedef unsigned long long index_t;
	typedef unsigned long long offset_t;

	static constexpr offset_t NIL_OFFSET = std::numeric_limits<offset_t>::max();
	static constexpr offset_t NIL_INDEX = std::numeric_limits<index_t>::max();
}