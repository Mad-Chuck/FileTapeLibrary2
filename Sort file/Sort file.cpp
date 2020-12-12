#include <array>
#include <iostream>
#include <fstream>

#include <FileTapeLibrary.h>

bool sort_policy(FileTapeLibrary::ArrayRecord ar1, FileTapeLibrary::ArrayRecord ar2) {
	return ar1.max() <= ar2.max();
}

int main(int argc, char** argv) {
	auto metadata_filepath = std::string("./metadata.dat");
	auto index_filepath = std::string("./index.dat");
	auto records_filepath = std::string("./records.dat");

	auto database = FileTapeLibrary::BTree(metadata_filepath, index_filepath, records_filepath);

	FileTapeLibrary::ArrayRecord ar({ 1, 2, 3, 4 });

	char c;
	do {
		std::cout << "Menu:" << std::endl;
		std::cout << "c - clear database" << std::endl;
		std::cout << "i - insert record" << std::endl;
		std::cout << "r - read record" << std::endl;
		std::cout << "e - exit" << std::endl;
		
		std::cin >> c;

		FileTapeLibrary::index_t key;
		FileTapeLibrary::ArrayRecord record;
		
		switch(c) {
		case 'c':
			database.clear();
			std::cout << "Database cleared" << std::endl;
			break;
			
		case 'i':
			
			std::cout << "Enter key:" << std::endl;
			std::cin >> key;
			std::cout << "Enter record:" << std::endl;
			try {
				record = FileTapeLibrary::read_user_format_record_from_stream(std::cin);
			}
			catch (std::exception& e) {
				std::cout << e.what() << std::endl;
				break;
			}

			try {
				database.insert_record(key, record);
				std::cout << "Record inserted" << std::endl;
			}
			catch (std::exception& e) {
				std::cout << e.what() << std::endl;
			}
			break;

		case 'r':
			FileTapeLibrary::index_t key;
			std::cout << "Enter key:" << std::endl;
			std::cin >> key;

			try {
				record = database.read_record(key);
				std::cout << record << std::endl;
			}
			catch (std::exception& e) {
				std::cout << e.what() << std::endl;
			}
			break;
			
		default:
			break;
		}
	} while (c != 'e');

	return 0;
}