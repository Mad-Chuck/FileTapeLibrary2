#pragma once
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "typedefs.h"
#include "ArrayRecord.h"
#include "TreePage.h"

namespace FileTapeLibrary {
	class BTree {
	public:
		// storing root_offset in metadata file to be able to tell where the root is in file
		// we must keep offset pointing to root_offset in metadata file
		static constexpr offset_t ROOT_OFFSET_OFFSET = 0;
		static constexpr offset_t DEFAULT_ROOT_OFFSET = 0;
		
		BTree(std::string metadata_filepath, std::string index_filepath, std::string records_filepath);

		void insert_record(index_t index, ArrayRecord& record);
		ArrayRecord read_record(index_t index);
		void print_file();
		// clear database
		void clear();
		
	private:
		offset_t root_offset;
		std::vector<TreePage> page_buffer;

		TreePage& current_page();
		void clear_buffer();
		//TreePage* last_read_page;

		// init empty files
		void initialize_metadata_file(offset_t root_offset = DEFAULT_ROOT_OFFSET);
		void initialize_index_file(offset_t root_offset = DEFAULT_ROOT_OFFSET);
		void initialize_records_file();

		// finds a place for given index in a file
		// leaves current page as last in the buffer
		// returns position of index on page
		// to check if found successfully check if page_buffer.back()[position] == index
		std::size_t try_find_record(index_t index);

		/* operations on index_file */
		void read_page_from_file(offset_t offset);
		void write_page_to_file(TreePage &page);
		void read_root_offset_from_file();
		void write_root_offset_to_file();
		

		/* operations on records_file */
		ArrayRecord read_record_from_file(offset_t offset);
		void write_record_to_file(offset_t offset, ArrayRecord& record);
		offset_t append_record_to_file(ArrayRecord& record);

		std::string metadata_filepath;
		std::string index_filepath;
		std::string records_filepath;
		
		//std::fstream index_file;
		//std::fstream records_file;
		//TreePage root;
	};
}
