#pragma once
#include "ArrayRecord.h"

namespace FileTapeLibrary {
	class Tape {
	public:
		// tape can be in two states
		typedef int open_mode;
		static constexpr open_mode none = 1 << 0;
		static constexpr open_mode read = 1 << 1;
		static constexpr open_mode write = 1 << 2;

		static constexpr std::size_t BUFFER_SIZE = 4096;

		// default constructor 
		Tape(std::string filepath, open_mode = none);
		/*// copy constructor
		Tape(const Tape& other);
		// copy assignment
		Tape& operator=(const Tape& other);
		// move constructor
		Tape(Tape&&);
		// move assignment
		Tape& operator=(Tape&&);*/
		//destructor
		~Tape();

		// read array record from tape
		ArrayRecord read_next_record();

		// write array record to tape
		void write_next_record(ArrayRecord record);

		// set tape to work in read/write
		void open(std::string filepath, open_mode mode);
		void close();

		ArrayRecord get_current_record();
		ArrayRecord get_last_record();
		// clear last record to assume series is progressing
		void clear_last_record();
		
		/* for read mode only */
		// check if end of tape has been exceeded
		bool is_empty();

		bool is_progressing(bool progressing_policy(ArrayRecord ar1, ArrayRecord ar2)) const;

		unsigned long long get_page_operations() const;
		std::string get_filepath() const;
		
	private:
		// read int from tape
		//int read_int();
		// read byte from tape
		char getc();

		// write int to tape
		//void write_int(int i);
		// write byte to tape
		void putc(char c);

		void end_current_mode();
		void init_mode(open_mode mode);
		// read page from file
		void read_buffer();
		// write page to file
		void write_buffer();

		bool record_read = false;
		ArrayRecord current_record;
		ArrayRecord last_record;
		open_mode mode;
		std::string filepath;
		std::ifstream in;
		std::ofstream out;

		char buffer[BUFFER_SIZE];

		// how many valid bytes there are in buffer
		// when in write mode this means that first {{buffer_data_left}} bytes were written
		// when in read mode this means that last {{buffer_data_left}} bytes are to be read 
		std::size_t buffer_data_left;
		// size of last portion of data in a file
		std::size_t buffer_last_size = 0;

		std::size_t buffer_next_index() const;

		// counter of buffer's outputs or inputs
		unsigned long long page_operations;
	};
}