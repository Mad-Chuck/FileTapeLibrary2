#pragma once
#include <random>

#include "typedefs.h"
#include "ArrayRecord.h"
#include "Tape.h"
#include "BTree.h"
#include "TreePage.h"

namespace FileTapeLibrary {
	void print_file(std::string filepath);
	void print_file(std::string filepath, std::ostream &logger);
	void initialize_random_tape(std::string filepath, int random_records_number, int seed = std::mt19937::default_seed);
	void convert_to_coded_format(std::string user_format_filepath, std::string coded_format_filepath);
	ArrayRecord read_user_format_record_from_stream(std::istream &in);
	void copy_file(std::string filepath, std::string output_path);
	bool is_sorted(std::string filepath, bool sorting_policy(ArrayRecord ar1, ArrayRecord ar2));
	// returns number of phases and number of disc operations
	std::tuple<unsigned int, unsigned long long> polyphase_merge_sort(
		std::string input_path,
		std::string output_path,
		bool sorting_policy(ArrayRecord ar1, ArrayRecord ar2)
	);
	
	// returns number of phases and number of disc operations
	std::tuple<unsigned int, unsigned long long> polyphase_merge_sort(
		std::string input_path,
		std::string output_path,
		bool sorting_policy(ArrayRecord ar1, ArrayRecord ar2),
		std::ostream& log
	);
}
