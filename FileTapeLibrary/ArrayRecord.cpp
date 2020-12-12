#include "ArrayRecord.h"

FileTapeLibrary::ArrayRecord FileTapeLibrary::ArrayRecord::DNEArrayRecord() {
	return ArrayRecord(0);
}

FileTapeLibrary::ArrayRecord::ArrayRecord(std::size_t size) {
	size_ = size;
}

FileTapeLibrary::ArrayRecord::ArrayRecord(std::initializer_list<int> list) {
	size_ = list.size();
	std::copy(list.begin(), list.begin() + std::min(list.size(), MAX_SIZE), data.begin());
}

FileTapeLibrary::ArrayRecord::ArrayRecord(int* array, std::size_t size) {
	size_ = size;
	std::copy(array, array + std::min(size, MAX_SIZE), data.begin());
}

std::size_t FileTapeLibrary::ArrayRecord::size() const {
	return size_;
}

/*void FileTapeLibrary::ArrayRecord::resize(size_t size) {
	data.resize(size);
}*/

/*std::string FileTapeLibrary::ArrayRecord::serialize() {
	auto record_str = std::string();

	// append size to string
	record_str += std::to_string(size());

	// append data to string
	for (auto element : data) {
		record_str += ' ';
		record_str += std::to_string(element);
	}

	return record_str;
}

void FileTapeLibrary::ArrayRecord::deserialize(std::istream& is) {
	// read size
	std::size_t size;
	is >> size;

	// declare data block
	data = std::vector<int>(size);

	// read data
	for (std::size_t i = 0; i < size; ++i) {
		is >> data[i];
	}
}*/

// bool FileTapeLibrary::ArrayRecord::operator<(const ArrayRecord& other) const {
//     return this->max() < other.max();
// }
//
// bool FileTapeLibrary::ArrayRecord::operator<=(const ArrayRecord& other) const {
// 	return this->max() <= other.max();
// }
//
// bool FileTapeLibrary::ArrayRecord::operator>(const ArrayRecord& other) const {
// 	return this->max() > other.max();
// }
//
// bool FileTapeLibrary::ArrayRecord::operator>=(const ArrayRecord& other) const {
// 	return this->max() >= other.max();
// }
//
// bool FileTapeLibrary::ArrayRecord::operator==(const ArrayRecord& other) const {
// 	return this->max() >= other.max();
// }
//
// bool FileTapeLibrary::ArrayRecord::operator!=(const ArrayRecord& other) const {
// 	return this->max() >= other.max();
// }

int FileTapeLibrary::ArrayRecord::max() const {
	if (size() <= 0) {
		throw std::exception("record has no data");
	}
	int max = data[0];
	
	for (std::size_t i = 1; i < size(); ++i) {
		if (data[i] > max) {
			max = data[i];
		}
	}

	return max;
}

bool FileTapeLibrary::ArrayRecord::is_valid() const {
	return size() > 0;
}

std::string FileTapeLibrary::ArrayRecord::short_format() const {
	if (is_valid()) {
		return std::to_string(max());
	}
	return "DNE";
}

int& FileTapeLibrary::ArrayRecord::operator[](std::size_t i) {
	return data[i];
}

namespace FileTapeLibrary {
	std::ostream& operator<<(std::ostream& os, const FileTapeLibrary::ArrayRecord& ar) {
		//os << ar.short_format();
		//return os;
		
		// if record is not valid
		if (!ar.is_valid()) {
			os << "DNE";
			return os;
		}

		// output size
		os << "{ ";

		//write data
		for (std::size_t i = 0; i < ar.size(); ++i) {
			os << ar.data[i];
			if (i != ar.size() - 1) {
				os << ", ";
			}
		}

		os << " }";
		return os;
	}
}
