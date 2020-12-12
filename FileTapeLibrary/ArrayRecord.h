#pragma once
#include <fstream>
#include <string>
#include <array>

namespace FileTapeLibrary {
	class ArrayRecord {
	public:
		static const std::size_t MAX_SIZE = 15;
		// special type of ArrayRecord which size is 0 - used to indicate that it is empty, not existing record
		static ArrayRecord DNEArrayRecord();
		
		ArrayRecord(std::size_t size = MAX_SIZE);
		ArrayRecord(std::initializer_list<int> list);
		ArrayRecord(int* data, std::size_t size);
		/*// copy constructor
		ArrayRecord(const ArrayRecord& other) = default;
		// copy assignment
		ArrayRecord& operator=(const ArrayRecord& other) = default;
		// move constructor
		ArrayRecord(ArrayRecord&&) = default;
		// move assignment
		ArrayRecord& operator=(ArrayRecord&&) = default;
		//destructor
		~ArrayRecord() = default;*/
		
		std::size_t size() const;
		//void resize(size_t size);
		//std::string serialize();
		//void deserialize(std::istream& is);
		
		// bool operator<(const ArrayRecord& other) const;
		// bool operator<=(const ArrayRecord& other) const;
		// bool operator>(const ArrayRecord& other) const;
		// bool operator>=(const ArrayRecord& other) const;
		// bool operator==(const ArrayRecord& other) const;
		// bool operator!=(const ArrayRecord& other) const;
		
		int max() const;
		virtual bool is_valid() const;
		std::string short_format() const;
		
		// friend std::istream& operator>>(std::istream& is, ArrayRecord& ar);
		friend std::ostream& operator<<(std::ostream& os, const ArrayRecord& ar);

		int& operator[](std::size_t i);
	private:
		std::size_t size_;
		std::array<int, MAX_SIZE> data;
	};

}