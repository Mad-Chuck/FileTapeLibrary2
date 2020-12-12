#include <fstream>
#include <iostream>
#include <tuple>

#include <FileTapeLibrary.h>

bool sort_policy(FileTapeLibrary::ArrayRecord ar1, FileTapeLibrary::ArrayRecord ar2) {
	return ar1.max() <= ar2.max();
}

int main(int argc, char** argv) {
	auto filepath = std::string("./data/random.dat");
	auto output_path = std::string("./data/sorted.dat");
	auto data_path = std::string("./data/data.csv");
	auto data = std::ofstream(data_path, std::ios_base::binary);

	auto max_number_of_records = int(100000);

	for (int i = 10; i <= max_number_of_records; i *= 10) {
		FileTapeLibrary::initialize_random_tape(filepath, i);

		auto numbers = FileTapeLibrary::polyphase_merge_sort(filepath, output_path, sort_policy);
		
		if (!FileTapeLibrary::is_sorted(output_path, sort_policy)) {
			std::cout << "File is not sorted" << std::endl;
		}

		data << i << "," << std::get<0>(numbers) << "," << std::get<1>(numbers) << std::endl;
	}
}
