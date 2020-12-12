#include "Tape.h"

FileTapeLibrary::Tape::Tape(std::string filepath, open_mode mode) {
	this->filepath = filepath;
	page_operations = 0;

	init_mode(mode);
}

FileTapeLibrary::Tape::~Tape() {
	end_current_mode();
}

FileTapeLibrary::ArrayRecord FileTapeLibrary::Tape::read_next_record() {
	if (mode != read) {
		throw std::exception("tape is not in read mode");
	}
	
	last_record = current_record;

	if (is_empty()) {
		current_record = ArrayRecord::DNEArrayRecord();
	}
	else {
		for (std::size_t i = 0; i < sizeof(current_record); ++i) {
			reinterpret_cast<char*>(&current_record)[i] = getc();
		}
				
		// auto size = read_int();
		// current_record = ArrayRecord(size);
		//
		// for (int i = 0; i < ArrayRecord::MAX_SIZE; ++i) {
		// 	if (i < size) {
		// 		current_record[i] = read_int();
		// 	}
		// 	else {
		// 		read_int();
		// 	}
		// }
	}

	return current_record;
}

/*int FileTapeLibrary::Tape::read_int() {
	int i;
	// read 4 bytes
	for (std::size_t k = 0; k < sizeof(int); ++k) {
		reinterpret_cast<char*>(&i)[k] = getc();
	}
	return i;
}*/

char FileTapeLibrary::Tape::getc() {
	// check if tape is empty and try to load more data to buffer
	if (is_empty()) {
		throw std::exception("tape is empty");
	}
	/*// if buffer is empty
	if (buffer_data_left == 0) {		// no valid data in buffer
		// fill buffer from file
		read_buffer();
	}*/
	
	// read byte from buffer
	char c = buffer[buffer_next_index()];
	// decrement buffer 
	--buffer_data_left;
	
	return c;
}

void FileTapeLibrary::Tape::write_next_record(ArrayRecord record) {
	if (mode != write) {
		throw std::exception("tape is not in write mode");
	}

	last_record = current_record;
	current_record = record;

	char* data = reinterpret_cast<char*>(&record);
	for (std::size_t i = 0; i < sizeof(record); ++i) {
		putc(data[i]);
	}
	
	/*// write size
	write_int(record.size());
	// write all data
	for (std::size_t i = 0; i < ArrayRecord::MAX_SIZE; ++i) {
		if (i < record.size()) {
			write_int(record[i]);
		}
		else {
			write_int(0);
		}
	}*/
}
/*

void FileTapeLibrary::Tape::write_int(int i) {
	// cast int to 4 bytes
	const char* istr = reinterpret_cast<const char*>(&i);

	// put each byte in buffer
	for (std::size_t k = 0; k < sizeof(int); ++k) {
		putc(istr[k]);
	}
}

*/

void FileTapeLibrary::Tape::putc(char c) {
	if (mode != write) {
		throw std::exception("tape is not in write mode");
	}
	
	// if buffer is full
	if (buffer_data_left >= BUFFER_SIZE) {					// should never be more than BUFFER_SIZE
		// empty buffer to file
		write_buffer();
	}

	// write byte to the buffer
	buffer[buffer_next_index()] = c;
	
	// increase valid buffer data size
	++buffer_data_left;
}


void FileTapeLibrary::Tape::open(std::string filepath, open_mode mode) {
	this->filepath = filepath;
	// end current mode
	end_current_mode();

	// start new mode
	init_mode(mode);
}

void FileTapeLibrary::Tape::close() {
	end_current_mode();
	this->mode = none;
}

void FileTapeLibrary::Tape::end_current_mode() {
	// no record has been read
	last_record = ArrayRecord::DNEArrayRecord();
	current_record = ArrayRecord::DNEArrayRecord();
	
	if (mode == read) {
		// close input file
		in.close();
		// current buffer may be forgotten (should be empty if tape is used correctly)
	}
	else if (mode == write) {
		// if buffer is not empty
		if (buffer_data_left > 0) {
			// write rest of buffer to file
			write_buffer();
		}
		// can close file
		out.close();
	}

	this->mode = none;
}

void FileTapeLibrary::Tape::init_mode(open_mode mode) {
	this->mode = mode;
	buffer_last_size = 0;

	// no record has been read or written
	last_record = ArrayRecord::DNEArrayRecord();
	current_record = ArrayRecord::DNEArrayRecord();
	
	if (mode == read) {
		// buffer is empty
		record_read = false;
		buffer_data_left = 0;
		in.open(filepath, std::fstream::binary);
	}
	else if (mode == write) {
		// buffer is empty
		buffer_data_left = 0;
		out.open(filepath, std::fstream::binary);
	}
}

bool FileTapeLibrary::Tape::is_empty() {
	if (mode != read) {
		throw std::exception("tape is not in read mode");
	}
	
	// if there is nothing left in a buffer
	if (buffer_data_left == 0) {
		// if file has been read completely
		if (in.eof()) {
			// no more data neither in file nor buffer
			return true;
		}

		// file has not been read completely, but there is nothing left in a buffer
		
		// try to read buffer
		read_buffer();
		// if last portion of data was of size 0 - tape is empty
		return buffer_data_left == 0;
	}
	// there is still something left in a buffer
	return false;
}

FileTapeLibrary::ArrayRecord FileTapeLibrary::Tape::get_current_record() {	
	return current_record;
}

FileTapeLibrary::ArrayRecord FileTapeLibrary::Tape::get_last_record() {
	return last_record;
}

void FileTapeLibrary::Tape::clear_last_record() {
	last_record = ArrayRecord::DNEArrayRecord();
}

bool FileTapeLibrary::Tape::is_progressing(bool progressing_policy(ArrayRecord ar1, ArrayRecord ar2)) const {
	// if one or no record has been read - series is progressing
	if (!last_record.is_valid()) {
		return true;
	}

	if (!current_record.is_valid()) {
		return false;
	}
	
	return progressing_policy(last_record, current_record);
}

unsigned long long FileTapeLibrary::Tape::get_page_operations() const {
	return page_operations;
}

std::string FileTapeLibrary::Tape::get_filepath() const {
	return filepath;
}

// read portion of data to the buffer
void FileTapeLibrary::Tape::read_buffer() {	
	in.read(buffer, BUFFER_SIZE);
	++page_operations;

	if (!in.eof()) {
		// all 'BUFFER_SIZE' bytes read successfully
		//std::cout << "read " << BUFFER_SIZE << " bytes" << std::endl;
		// buffer is full
		buffer_data_left = BUFFER_SIZE;
	}
	else {
		//std::cout << "only " << in.gcount() << " could have been read" << std::endl;
		// cast is safe since we already know BUFFER_SIZE > gcount()
		buffer_data_left = static_cast<std::size_t>(in.gcount());
		// current buffer size is same as buffer_data_left
		buffer_last_size = buffer_data_left;
		// in.close(); // redundant?
	}
}

void FileTapeLibrary::Tape::write_buffer() {
	out.write(buffer, buffer_data_left);
	++page_operations;
	// buffer is now empty
	buffer_data_left = 0;
}

std::size_t FileTapeLibrary::Tape::buffer_next_index() const {
	// in read mode next index is position of first unread byte
	if (mode == read) {
		// last portion of data can be shorter than buffer
		if (in.eof()) {
			return buffer_last_size - buffer_data_left;
		}
		return BUFFER_SIZE - buffer_data_left;
	}
	// in write mode next index is place for next byte
	if (mode == write) {
		return buffer_data_left;
	}
	return 0;
}
