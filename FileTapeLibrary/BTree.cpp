#include "BTree.h"

FileTapeLibrary::BTree::BTree(std::string metadata_filepath, std::string index_filepath, std::string records_filepath) {
	this->metadata_filepath = metadata_filepath;
	this->index_filepath = index_filepath;
	this->records_filepath = records_filepath;

	// make sure metadata file exists and is big enough to give us root_offset
	if (!std::filesystem::exists(metadata_filepath) || std::filesystem::file_size(metadata_filepath) < sizeof(root_offset)) {
		// create empty metadata file
		initialize_metadata_file();
	}
	else {
		// read root_offset from metadata file
		read_root_offset_from_file();
	}
	
	// make sure index file exists and is big enough to contain root_page
	if (!std::filesystem::exists(index_filepath) || std::filesystem::file_size(index_filepath) < root_offset + TreePage::SIZE_IN_FILE) {
		// file either doesn't exist or is not big enough - create new one
		initialize_index_file(root_offset);
	}

	// make sure records file exists
	if (!std::filesystem::exists(records_filepath)) {
		// file does not exist - create empty file
		initialize_records_file();
	}

	return;
}

void FileTapeLibrary::BTree::insert_record(index_t index, ArrayRecord& record) {
	auto position = try_find_record(index);

	// if found
	if (current_page().get_key(position) == index) {
		clear_buffer();
		throw std::exception("Already exists");
	}

	// if position is empty
	if (current_page().is_empty_key(position)) {
		// insert here

		// insert record to records_file
		auto offset = append_record_to_file(record);

		// insert offset to current page
		current_page().set_key(position, index);
		current_page().set_record_offset(position, offset);

		// update page in index_file
		write_page_to_file(current_page());
		clear_buffer();
		
		return;
	}

	/* OVERFLOW */
	
	//std::get<>();
	
	/*
	  1. Search for x (using the Searching algorithm).
	  2. If found, then RETURN (Already_Exists).
	  3. On the current page check if m < 2d. If YES,
		then insert (x,a) into this page, RETURN (OK).
					<-- OVERFLOW! -->
	  4. Try compensation(description will follow).
	  5. If compensation possible, then RETURN(OK).
				<-- COMPENSATION IMPOSSIBLE! -->
	  6. Make split of the overflown page(description will follow).
	  7. Make the ancestor page the current page.Go to step 3.
	 */

	
}

FileTapeLibrary::ArrayRecord FileTapeLibrary::BTree::read_record(index_t index) {
	auto position = try_find_record(index);

	// if found
	if (page_buffer.back().get_key(position) == index) {
		// read record from file
		auto record_offset = current_page().get_record_offset(position);
		// clear buffer
		clear_buffer();
		// return record
		return read_record_from_file(record_offset);
	}
	
	clear_buffer();
	throw std::exception("Not found");
}

void FileTapeLibrary::BTree::print_file() {
}

FileTapeLibrary::TreePage& FileTapeLibrary::BTree::current_page() {
	return page_buffer.back();
}

void FileTapeLibrary::BTree::clear_buffer() {
	page_buffer.clear();
}

void FileTapeLibrary::BTree::initialize_metadata_file(offset_t root_offset) {
	// file create empty file
	auto index_file = std::fstream(metadata_filepath, std::ios::out | std::ios::trunc);
	index_file.close();

	// put default root page in a file
	this->root_offset = root_offset;
	// put root_offset in file
	write_root_offset_to_file();		
}

void FileTapeLibrary::BTree::initialize_index_file(offset_t root_offset) {
	// discard current content in file or create empty
	auto index_file = std::fstream(index_filepath, std::ios::out | std::ios::trunc);
	index_file.close();

	// put empty root page in file
	page_buffer.emplace_back(TreePage(root_offset));
	write_page_to_file(current_page());
	clear_buffer();
}

void FileTapeLibrary::BTree::initialize_records_file() {
	auto records_file = std::fstream(records_filepath, std::ios::out | std::ios::trunc);
	records_file.close();
}

void FileTapeLibrary::BTree::clear() {
	initialize_metadata_file();
	initialize_index_file();
	initialize_records_file();
}

std::size_t FileTapeLibrary::BTree::try_find_record(index_t index) {
	auto s = root_offset;
	auto position = std::size_t();
	
	/*
	  1. Let s be the address of the root page.
	  2. If s = NIL, then RETURN(Not_Found).
	  3. Fetch the page indicated by s to the main memory.
	  4. Search for x in this page(eg. using bisection).
	  5. If found xi = x, then RETURN(xi, ai)
	  6. If x < x1, then s = p0. Go to step 2.
	  7. If x > xm, then s = pm. Go to step 2.
	  8. Let s = pi, where xi < x < xi+1. Go to step 2.
	 */
	
	do {
		read_page_from_file(s);

		// find_position_for_key position for next item
		position = current_page().find_position_for_key(index);

		// if either is empty or the one we looking for
		if (current_page().is_empty_key(position) || current_page().get_key(position) == index) {
			// found
			break;
		}

		if (current_page().get_key(position) < index) {
			s = current_page().get_left_child_offset(position);
		}
		else if (current_page().get_key(position) > index) {
			s = current_page().get_right_child_offset(position);
		}
	}
	while (s != NIL_OFFSET);
	
	return position;
}

void FileTapeLibrary::BTree::read_page_from_file(offset_t offset) {
	page_buffer.emplace_back(TreePage(offset));
	current_page().read_from_file(index_filepath);
}

void FileTapeLibrary::BTree::write_page_to_file(TreePage& page) {
	page.write_to_file(index_filepath);
}

void FileTapeLibrary::BTree::read_root_offset_from_file() {
	auto index_file = std::ifstream(metadata_filepath, std::ios::binary | std::ios::out | std::ios::in);
	index_file.seekg(ROOT_OFFSET_OFFSET);
	index_file.read(reinterpret_cast<char*>(&root_offset), sizeof(root_offset));
	index_file.close();
}

void FileTapeLibrary::BTree::write_root_offset_to_file() {
	auto index_file = std::ofstream(metadata_filepath, std::ios::binary | std::ios::out | std::ios::in);
	index_file.seekp(ROOT_OFFSET_OFFSET);
	index_file.write(reinterpret_cast<const char*>(&root_offset), sizeof(root_offset));
	index_file.close();
}

FileTapeLibrary::ArrayRecord FileTapeLibrary::BTree::read_record_from_file(offset_t offset) {
	auto records_file = std::ifstream(records_filepath, std::ios::binary | std::ios::out | std::ios::in);
	records_file.seekg(offset);

	auto size = std::size_t();
	records_file.read(reinterpret_cast<char*>(&size), sizeof(size));
	auto record = ArrayRecord(size);

	for (std::size_t i = 0; i < ArrayRecord::MAX_SIZE; i++) {
		records_file.read(reinterpret_cast<char*>(&record[i]), sizeof(record[i]));
	}
	
	records_file.close();
	
	return record;
}

void FileTapeLibrary::BTree::write_record_to_file(offset_t offset, ArrayRecord& record) {
	auto records_file = std::ofstream(records_filepath, std::ios::binary | std::ios::out | std::ios::in);
	records_file.seekp(offset);
	
	auto size = record.size();
	records_file.write(reinterpret_cast<char*>(&size), sizeof(size));

	for (int i = 0; i < ArrayRecord::MAX_SIZE; ++i) {
		records_file.write(reinterpret_cast<const char*>(&record[i]), sizeof(record[i]));
	}

	records_file.close();
}

FileTapeLibrary::offset_t FileTapeLibrary::BTree::append_record_to_file(ArrayRecord& record) {
	auto records_file = std::ofstream(records_filepath, std::ios::binary | std::ios::out | std::ios::in | std::ios::ate);

	auto offset = records_file.tellp();
	auto size = record.size();
	records_file.write(reinterpret_cast<char*>(&size), sizeof(size));

	for (std::size_t i = 0; i < ArrayRecord::MAX_SIZE; ++i) {
		records_file.write(reinterpret_cast<const char*>(&record[i]), sizeof(record[i]));
	}
	
	records_file.close();
	
	return offset;
}
