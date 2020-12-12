#pragma once
#include <array>
#include <fstream>
#include <limits>

#include "typedefs.h"

namespace FileTapeLibrary {
	class TreePage {
	public:
		static constexpr std::size_t D = 4;
		static constexpr std::size_t SIZE_IN_FILE = sizeof(offset_t) + 2 * D * sizeof(index_t) + 2 * D * sizeof(offset_t) + (2 * D + 1) * sizeof(offset_t);
		TreePage(offset_t self_offset);

		std::size_t find_position_for_key(index_t key);
		offset_t get_left_child_offset(std::size_t position);
		void set_left_child_offset(std::size_t position, offset_t offset);
		offset_t get_right_child_offset(std::size_t position);
		void set_right_child_offset(std::size_t position, offset_t offset);

		bool is_root();

		//index_t& operator[](std::size_t i);

		index_t get_key(std::size_t position);
		void set_key(std::size_t position, index_t key);

		offset_t get_record_offset(std::size_t position);
		void set_record_offset(std::size_t position, offset_t key);

		bool is_empty_key(std::size_t position);

		//void read_from_file(offset_t offset, std::string& filepath);
		void read_from_file(std::string& filepath);
		void write_to_file(std::string& filepaths);
	private:

		offset_t self_offset;
		// if parent_offset is set to max offset_t value - that means current page is root
		offset_t parent_offset;
		std::array<index_t, 2 * D> keys;
		std::array<offset_t, 2 * D> record_offsets;
		std::array<offset_t, 2 * D + 1> children;
	};
}
