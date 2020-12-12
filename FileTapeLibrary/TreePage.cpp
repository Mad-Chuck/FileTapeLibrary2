#include "TreePage.h"

#include <iostream>

FileTapeLibrary::TreePage::TreePage(offset_t self_offset) {
	this->self_offset = self_offset;
	parent_offset = NIL_OFFSET;
	std::fill(keys.begin(), keys.end(), NIL_INDEX);
	std::fill(record_offsets.begin(), record_offsets.end(), NIL_OFFSET);
	std::fill(children.begin(), children.end(), NIL_OFFSET);
}

std::size_t FileTapeLibrary::TreePage::find_position_for_key(index_t key) {
	for (std::size_t i = 0; i < keys.size() - 1; ++i) {
		// If keys[i] is place for key (or if is empty)
		if (key <= keys[i] || keys[i] == NIL_INDEX) {
			return i;
		}
	}

	return keys.size();
}

FileTapeLibrary::offset_t FileTapeLibrary::TreePage::get_left_child_offset(std::size_t position) {
	return children[position];
}

void FileTapeLibrary::TreePage::set_left_child_offset(std::size_t position, offset_t offset) {
	children[position] = offset;
}

FileTapeLibrary::offset_t FileTapeLibrary::TreePage::get_right_child_offset(std::size_t position) {
	return children[position + 1];
}

void FileTapeLibrary::TreePage::set_right_child_offset(std::size_t position, offset_t offset) {
	children[position] = offset;
}

bool FileTapeLibrary::TreePage::is_root() {
	return parent_offset == NIL_OFFSET;
}

FileTapeLibrary::index_t FileTapeLibrary::TreePage::get_key(std::size_t position) {
	return keys[position];
}

void FileTapeLibrary::TreePage::set_key(std::size_t position, index_t key) {
	keys[position] = key;
}

FileTapeLibrary::offset_t FileTapeLibrary::TreePage::get_record_offset(std::size_t position) {
	return record_offsets[position];
}

void FileTapeLibrary::TreePage::set_record_offset(std::size_t position, offset_t key) {
	record_offsets[position] = key;
}

bool FileTapeLibrary::TreePage::is_empty_key(std::size_t position) {
	return keys[position] == NIL_INDEX;
}

/*
void FileTapeLibrary::TreePage::read_from_file(offset_t offset, std::string& filepath) {
	self_offset = offset;
	
	auto file = std::ifstream(filepath, std::ios::binary | std::ios::out | std::ios::in);
	file.seekg(offset);
	file.read(
		reinterpret_cast<char*>(&parent_offset),
		sizeof(parent_offset)
	);
	file.read(
		reinterpret_cast<char*>(keys.data()),
		static_cast<unsigned long long>(sizeof(index_t)) * keys.size()
	);
	file.read(
		reinterpret_cast<char*>(record_offsets.data()),
		static_cast <unsigned long long>(sizeof(offset_t)) * keys.size()
	);
	file.read(
		reinterpret_cast<char*>(children.data()),
		static_cast <unsigned long long>(sizeof(offset_t)) * children.size()
	);
}*/

void FileTapeLibrary::TreePage::read_from_file(std::string& filepath) {
	auto file = std::ifstream(filepath, std::ios::binary | std::ios::out | std::ios::in);
	file.seekg(self_offset);
	file.read(
		reinterpret_cast<char*>(&parent_offset),
		sizeof(parent_offset)
	);
	file.read(
		reinterpret_cast<char*>(keys.data()),
		static_cast<unsigned long long>(sizeof(index_t)) * keys.size()
	);
	file.read(
		reinterpret_cast<char*>(record_offsets.data()),
		static_cast <unsigned long long>(sizeof(offset_t)) * keys.size()
	);
	file.read(
		reinterpret_cast<char*>(children.data()),
		static_cast <unsigned long long>(sizeof(offset_t)) * children.size()
	);
}

void FileTapeLibrary::TreePage::write_to_file(std::string& filepath) {
	auto file = std::ofstream(filepath, std::ios::binary | std::ios::out | std::ios::in);
	
	file.seekp(self_offset);
	file.write(
		reinterpret_cast<const char*>(&parent_offset), 
		sizeof(parent_offset)
	);
	file.write(
		reinterpret_cast<const char*>(keys.data()),
		static_cast<unsigned long long>(sizeof(index_t)) * keys.size()
	);
	file.write(
		reinterpret_cast<const char*>(record_offsets.data()),
		static_cast <unsigned long long>(sizeof(offset_t)) * keys.size()
	);
	file.write(
		reinterpret_cast<const char*>(children.data()),
		static_cast <unsigned long long>(sizeof(offset_t)) * children.size()
	);
}

/*
FileTapeLibrary::index_t& FileTapeLibrary::TreePage::operator[](std::size_t i) {
	return keys[i];
}
*/
