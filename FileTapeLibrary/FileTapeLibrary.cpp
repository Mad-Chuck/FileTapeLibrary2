#include <fstream>
#include <functional>
#include <iostream>
#include <random>
#include <sstream>
#include <tuple>

#include "Tape.h"
#include "FileTapeLibrary.h"

void FileTapeLibrary::print_file(std::string filepath) {
	print_file(filepath, std::cout);
}

void FileTapeLibrary::print_file(std::string filepath, std::ostream& logger) {
	auto tape = Tape(filepath, Tape::read);

	while (!tape.is_empty()) {
		logger << tape.read_next_record() << std::endl;
	}
}

void FileTapeLibrary::initialize_random_tape(std::string filepath, int random_records_number, int seed) {
	auto tape = Tape(filepath, Tape::write);
	auto engine = std::mt19937(seed);
	auto size_distribution = std::uniform_int_distribution<std::size_t>(1, ArrayRecord::MAX_SIZE);
	auto data_distribution = std::uniform_int_distribution<int>(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());

	auto size_generator = [&]() { return size_distribution(engine); };
	auto data_generator = [&]() { return data_distribution(engine); };

	int bytes = 0;

	for (int i = 0; i < random_records_number; ++i) {
		auto vector = std::vector<int>(size_generator());

		bytes += 4 * (1 + vector.size());
		std::generate_n(vector.begin(), vector.size(), data_generator);
		auto random_record = ArrayRecord(vector.data(), vector.size());

		//std::cout << random_record << std::endl;
		tape.write_next_record(random_record);
	}

	//std::cout << bytes << std::endl;
}

void FileTapeLibrary::convert_to_coded_format(std::string user_format_filepath, std::string coded_format_filepath) {
	auto in = std::ifstream(user_format_filepath, std::fstream::binary);
	auto tape = Tape(coded_format_filepath, Tape::write);

	auto record_line = std::string();
	while (std::getline(in, record_line, '}') && !in.eof()) {
		// remove all '{' characters
		record_line.erase(std::remove(record_line.begin(), record_line.end(), '{'), record_line.end());
		// remove all ' ' characters
		//record_line.erase(std::remove(record_line.begin(), record_line.end(), ' '), record_line.end());

		// create stringstream from current line
		auto record_stream = std::istringstream();
		record_stream.str(record_line);

		// data vector
		auto data = std::vector<int>();
		data.reserve(ArrayRecord::MAX_SIZE);

		auto number = std::string();
		while (std::getline(record_stream, number, ',')) {
			// add number to data vector
			data.push_back(std::stoi(number));
		}

		auto record = ArrayRecord(data.data(), data.size());
		//std::cout << record << std::endl;

		// put new record on tape
		tape.write_next_record(record);
	}

	tape.close();
}

FileTapeLibrary::ArrayRecord FileTapeLibrary::read_user_format_record_from_stream(std::istream& in) {
	auto record_line = std::string();
	
	if (std::getline(in, record_line, '}') && !in.eof()) {
		// remove all '{' characters
		record_line.erase(std::remove(record_line.begin(), record_line.end(), '{'), record_line.end());
		// remove all ' ' characters
		//record_line.erase(std::remove(record_line.begin(), record_line.end(), ' '), record_line.end());

		// create stringstream from current line
		auto record_stream = std::istringstream();
		record_stream.str(record_line);

		// data vector
		auto data = std::vector<int>();
		data.reserve(ArrayRecord::MAX_SIZE);

		auto number = std::string();
		while (std::getline(record_stream, number, ',')) {
			// add number to data vector
			data.push_back(std::stoi(number));
		}

		return ArrayRecord(data.data(), data.size());
	} 

	throw std::exception("Wrong input");
}

void FileTapeLibrary::copy_file(std::string filepath, std::string output_path) {
	auto input = Tape(filepath, Tape::read);
	auto output = Tape(output_path, Tape::write);

	while (!input.is_empty()) {
		output.write_next_record(input.read_next_record());
	}
}

bool FileTapeLibrary::is_sorted(std::string filepath, bool sorting_policy(ArrayRecord ar1, ArrayRecord ar2)) {
	auto tape = Tape(filepath, Tape::read);
	
	while (!tape.is_empty()) {
		tape.read_next_record();

		if (!tape.is_progressing(sorting_policy)) {
			return false;
		}
	}

	return true;
}


std::tuple<unsigned int, unsigned long long> FileTapeLibrary::polyphase_merge_sort(
	std::string input_path, std::string output_path,
    bool sorting_policy(ArrayRecord ar1, ArrayRecord ar2)
) {
	Tape tapes[] = {
		Tape(input_path, Tape::read),
		Tape("./data/tape1.dat", Tape::write),
		Tape("./data/tape2.dat", Tape::write)
	};
	int dummy_runs = 0;
	int series_written;

	int fib_last = 0,
		fib = 1;

	/*
	 * call this function only if:
	 * - data_tape has current_record (not DNE)
	 * - output_tape has current_record (can be DNE - no checking for joining then)
	 *
	 * logs will be correct if:
	 * - fib_last is number of series on output_tape
	 *
	 * remember:
	 * - after calling this function data_tape's current_record is not stored on any file (so only iff it is DNE - tape is actually empty)
	 */
	auto write_n_series = [&](std::size_t data_tape_id, std::size_t output_tape_id, int n) {
		auto& data_tape = tapes[data_tape_id];
		auto& output_tape = tapes[output_tape_id];
		bool series_joined = false;

		// repeat n times
		for (int i = 0; i < n; ++i) {
			// When writing first series check for series joining
			// Don't do it if output_tape is empty (current record is DNE)
			if (i == 0 && !series_joined && tapes[output_tape_id].get_current_record().is_valid()) {
				// check for series joining
				// if current record on data_tape (which we want to put on output_tape) is in correct order after current record on output_tape depending on sorting_policy
				if (sorting_policy(tapes[output_tape_id].get_current_record(), data_tape.get_current_record())) {
					// last element of last series was properly sorted before first element of next series
					// series are joined - need to get one more series
					series_joined = true;
					--i;
				}
			}

			do {
				// put data_tape's current record on output_tape (as current record)
				tapes[output_tape_id].write_next_record(data_tape.get_current_record());

				// get new record
				data_tape.read_next_record();
				
				// if current record is DNE
				if (!data_tape.get_current_record().is_valid()) {
					// data is over
					// dummy runs
					dummy_runs = n - i - 1;

					// if detected dummy runs, are equal to another file's series count, they are not needed at all
					if (dummy_runs == n) {
						dummy_runs = 0;
					}

					return i + 1;
				}
			}
			// while series on data_tape is progressing
			while (data_tape.is_progressing(sorting_policy));
		}
		return n;
	};

	/*
	 * call this function only when:
	 * - both tapes' current record is the record which begins the series
	 *
	 * calling this function does:
	 * - merge 2 tapes into output tape
	 * - the data on bigger tape will remain
	 * - no data will remain on both tapes if they have equal number of series
	 * - after this function current record on both functions is guaranteed not to be written to the output yet
	 *   if tape has been saved completely, it will have DNE as current record
	 *
	 * logs will be correct only if:
	 * - id1 tape has more series than id2 tape
	 */
	auto merge = [&](std::size_t id1, std::size_t id2, std::size_t output_tape_id) {		
		// id of tape, which record will be saved first - on beginning it can be either id1 or id2
		// since we assume series on both tapes is progressing (we cleared last record)
		std::size_t save_tape_id;
		
		auto& output_tape = tapes[output_tape_id];

		// don't care about last records, we start merging assuming current record is first of the tape
		tapes[id1].clear_last_record();
		tapes[id2].clear_last_record();

		// merge series until there is no more data on one of the tapes
		do {
			// merge records from one tape with second tape until the series on one of them is progressing
			do {
				// choose which tape's record goes first
				if (sorting_policy(tapes[id1].get_current_record(), tapes[id2].get_current_record())) {
					save_tape_id = id1;
				}
				else {
					save_tape_id = id2;
				}
				
				// save chosen record
				output_tape.write_next_record(tapes[save_tape_id].get_current_record());

				// get next record, since we just saved one
				tapes[save_tape_id].read_next_record();
				
				// if there is still data on tape
				if (!tapes[save_tape_id].get_current_record().is_valid()) {
					// no more data on tape - series ended for sure
					break;
				}
			}
			// continue if series on save_tape is still progressing
			while (tapes[save_tape_id].is_progressing(sorting_policy));

			// if tape of id1 ended first, finish tape of id2
			save_tape_id = save_tape_id == id1 ? id2 : id1;

			// finish series on the other tape
			do {
				// save current record from the tape
				output_tape.write_next_record(tapes[save_tape_id].get_current_record());

				// get next record, since we just saved one
				tapes[save_tape_id].read_next_record();

				// if there is still data on tape
				if (!tapes[save_tape_id].get_current_record().is_valid()) {
					// no more data on tape - series ended for sure
					break;
				}
			}
			// continue if series on is still progressing
			while (tapes[save_tape_id].is_progressing(sorting_policy));
		}
		// continue if both tapes are not empty
		while (tapes[id1].get_current_record().is_valid() && tapes[id2].get_current_record().is_valid());
	};

	/* distribution phase */

	auto output_tape_id = std::size_t(1);
	
	// read first record of file
	if (!tapes[0].is_empty()) {
		tapes[0].read_next_record();
	}
	else {
		// file has no data - tape sorted
		copy_file(input_path, output_path);
		return std::make_tuple(0, tapes[0].get_page_operations() + tapes[1].get_page_operations() + tapes[2].get_page_operations());;
	}

	// write 1 series to tape1 (no need to worry about series joining yet, however, need to keep last written record)
	series_written = write_n_series(0, output_tape_id, 1);

	// first series is on tape1

	// repeat process until all series has been distributed
	while (tapes[0].get_current_record().is_valid()) {
		// change active_output_tape
		output_tape_id = output_tape_id == 1 ? 2 : 1;

		// write fib series from input to active_tape (take series joining into account)
		series_written = write_n_series(0, output_tape_id, fib);

		// next fibonacci number
		std::tie(fib_last, fib) = std::make_tuple(fib, fib_last + fib);
	}

	// tape contained 1 series only (we didn't enter while loop above)
	if (fib_last == 0) {
		copy_file(input_path, output_path);
		return std::make_tuple(0, tapes[0].get_page_operations() + tapes[1].get_page_operations() + tapes[2].get_page_operations());
	}

	// close all tapes
	for (int i = 0; i < 3; ++i) {
		tapes[i].close();
	}
	/* end of distribution phase */

	/* merge phase */
	tapes[0].open(output_path, Tape::write);		// change path to prevent overriding input file
	tapes[1].open(tapes[1].get_filepath(), Tape::read);
	tapes[2].open(tapes[2].get_filepath(), Tape::read);

	// output_tape is last being written to, so it is "bigger" - it may contains dummy 
	auto bigger_tape_id = output_tape_id;
	auto smaller_tape_id = std::size_t(bigger_tape_id == 1 ? 2 : 1);
	output_tape_id = 0;
	
	// if in last write there were no actual series written in this case output tape was actually "smaller"
	if (series_written == 0) {
		// swap tapes
		std::swap(bigger_tape_id, smaller_tape_id);
	}

	// counter of phases
	auto phases_count = std::size_t(0);

	// read record of data tapes - bigger and smaller
	if (!tapes[bigger_tape_id].is_empty() && !tapes[smaller_tape_id].is_empty()) {
		tapes[smaller_tape_id].read_next_record();
		tapes[bigger_tape_id].read_next_record();
	}
	else {
		// something is very, very, very bad
		throw std::exception("Unknown very, very, very bad error");
	}

	do {
		// if dummy runs should be resolved, rewrite 'dummy_runs' series from "smaller" tape to output tape
		if (dummy_runs > 0) {
			// rewrite dummy runs to bigger tape
			series_written = write_n_series(smaller_tape_id, output_tape_id, dummy_runs);

			if (dummy_runs != series_written) {
				// who knows what that means, but in case it happens
				throw std::exception("Unknown error, dummy runs cannot have been written");
			}
			
			// no more dummy runs will ever be resolved again
			dummy_runs = 0;

			// dummy runs ended file
			if (!tapes[smaller_tape_id].get_current_record().is_valid()) {
				throw std::exception("Dummy runs ended tape");

				// not implemented yet - should not happen with current optimization
			}
		}

		/*
		for (int i = 0; i < 3; ++i) {
			tapes[i].close();

			log << "-----------------tape" << i << "------------------" << std::endl;
			print_file(tapes[i].get_filepath(), log);
		}*/
		
		// merge 2 tapes
		merge(bigger_tape_id, smaller_tape_id, output_tape_id);

		// switch smaller tape (which is empty now) to write mode
		tapes[smaller_tape_id].close();
		tapes[smaller_tape_id].open(tapes[smaller_tape_id].get_filepath(), Tape::write);

		// switch output tape to read mode
		tapes[output_tape_id].close();
		tapes[output_tape_id].open(tapes[output_tape_id].get_filepath(), Tape::read);
		// we should read first record here
		tapes[output_tape_id].read_next_record();

		// bigger tape is still opened

		// switch tapes' ids properly
		std::tie(smaller_tape_id, output_tape_id, bigger_tape_id) = std::make_tuple(bigger_tape_id, smaller_tape_id, output_tape_id);

		++phases_count;
	}
	// continue until bigger tape (smaller after switch) has data
	while (tapes[smaller_tape_id].get_current_record().is_valid());

	// close all tapes
	for (int i = 0; i < 3; ++i) {
		tapes[i].close();
	}

	// if sorted file is not on output_path
	if (tapes[bigger_tape_id].get_filepath() != output_path) {
		copy_file(tapes[bigger_tape_id].get_filepath(), output_path);
	}

	return std::make_tuple(phases_count, tapes[0].get_page_operations() + tapes[1].get_page_operations() + tapes[2].get_page_operations());
}

std::tuple<unsigned int, unsigned long long> FileTapeLibrary::polyphase_merge_sort(
	std::string input_path, std::string output_path,
	bool sorting_policy(ArrayRecord ar1, ArrayRecord ar2),
	std::ostream& log
) {
	log << "------------file before sort------------" << std::endl;
	print_file(input_path, log);

	Tape tapes[] = {
		Tape(input_path, Tape::read),
		Tape("./data/tape1.dat", Tape::write),
		Tape("./data/tape2.dat", Tape::write)
	};
	int dummy_runs = 0;
	int series_written;

	int fib_last = 0,
		fib = 1;

	/*
	 * call this function only if:
	 * - data_tape has current_record (not DNE)
	 * - output_tape has current_record (can be DNE - no checking for joining then)
	 *
	 * logs will be correct if:
	 * - fib_last is number of series on output_tape
	 *
	 * remember:
	 * - after calling this function data_tape's current_record is not stored on any file (so only iff it is DNE - tape is actually empty)
	 */
	auto write_n_series = [&](std::size_t data_tape_id, std::size_t output_tape_id, int n) {
		auto& data_tape = tapes[data_tape_id];
		auto& output_tape = tapes[output_tape_id];

		log << "writing " << n << " series to tape " << output_tape_id << " from tape " << data_tape_id << std::endl;
		log << "current record on data tape before write: " << data_tape.get_current_record() << std::endl;

		bool series_joined = false;

		// repeat n times
		for (int i = 0; i < n; ++i) {
			// When writing first series check for series joining
			// Don't do it if output_tape is empty (current record is DNE)
			if (i == 0 && !series_joined && tapes[output_tape_id].get_current_record().is_valid()) {
				// check for series joining
				// if current record on data_tape (which we want to put on output_tape) is in correct order after current record on output_tape depending on sorting_policy
				if (sorting_policy(tapes[output_tape_id].get_current_record(), data_tape.get_current_record())) {
					// last element of last series was properly sorted before first element of next series
					// series are joined - need to get one more series
					log << "series are joining" << std::endl;
					series_joined = true;
					--i;
				}
			}

			do {
				// put data_tape's current record on output_tape (as current record)
				tapes[output_tape_id].write_next_record(data_tape.get_current_record());

				// get new record
				data_tape.read_next_record();

				// if current record is DNE
				if (!data_tape.get_current_record().is_valid()) {
					// data is over
					// dummy runs
					dummy_runs = n - i - 1;

					// if detected dummy runs, are equal to another file's series count, they are not needed at all
					if (dummy_runs == n) {
						log << "no actual series has been written - resetting dummy runs" << std::endl;
						dummy_runs = 0;
					}

					log << i + 1 << " series were written " << (series_joined ? "(+1 joined) " : "(no joining)") << std::endl;
					log << dummy_runs << " dummy runs needed" << std::endl;
					log << "there are " << fib_last + i + 1 << " series currently on a tape" << std::endl;
					log << "current record on data tape after write: " << data_tape.get_current_record() << std::endl;
					log << std::endl;

					return i + 1;
				}
			}
			// while series on data_tape is progressing
			while (data_tape.is_progressing(sorting_policy));
		}

		log << n << " series were written " << (series_joined ? "(+1 joined) " : "(no joining)") << std::endl;
		log << "there are " << fib_last + n << " series currently on a tape" << std::endl;
		log << "current record on data tape after write: " << data_tape.get_current_record() << std::endl;
		log << std::endl;

		return n;
	};

	/*
	 * call this function only when:
	 * - both tapes' current record is the record which begins the series
	 *
	 * calling this function does:
	 * - merge 2 tapes into output tape
	 * - the data on bigger tape will remain
	 * - no data will remain on both tapes if they have equal number of series
	 * - after this function current record on both functions is guaranteed not to be written to the output yet
	 *   if tape has been saved completely, it will have DNE as current record
	 *
	 * logs will be correct only if:
	 * - id1 tape has more series than id2 tape
	 */
	auto merge = [&](std::size_t id1, std::size_t id2, std::size_t output_tape_id) {
		log << "merging tapes " << id1 << " and " << id2 << std::endl;

		// id of tape, which record will be saved first - on beginning it can be either id1 or id2
		// since we assume series on both tapes is progressing (we cleared last record)
		std::size_t save_tape_id;

		auto& output_tape = tapes[output_tape_id];

		// don't care about last records, we start merging assuming current record is first of the tape
		tapes[id1].clear_last_record();
		tapes[id2].clear_last_record();

		// merge series until there is no more data on one of the tapes
		do {
			// merge records from one tape with second tape until the series on one of them is progressing
			do {
				log << "comparing record " << tapes[id1].get_current_record() << " with " << tapes[id2].get_current_record() << std::endl;

				// choose which tape's record goes first
				if (sorting_policy(tapes[id1].get_current_record(), tapes[id2].get_current_record())) {
					save_tape_id = id1;
				}
				else {
					save_tape_id = id2;
				}
				log << "save tape is: " << save_tape_id << std::endl;

				// save chosen record
				log << "saving record " << tapes[save_tape_id].get_current_record() << std::endl;
				output_tape.write_next_record(tapes[save_tape_id].get_current_record());

				// get next record, since we just saved one
				tapes[save_tape_id].read_next_record();

				// if there is still data on tape
				if (!tapes[save_tape_id].get_current_record().is_valid()) {
					// no more data on tape - series ended for sure
					break;
				}
			}
			// continue if series on save_tape is still progressing
			while (tapes[save_tape_id].is_progressing(sorting_policy));

			log << "series on tape " << save_tape_id << " ended - ";

			// if tape of id1 ended first, finish tape of id2
			save_tape_id = save_tape_id == id1 ? id2 : id1;

			log << "finishing series of tape " << save_tape_id << std::endl;

			// finish series on the other tape
			do {
				// save current record from the tape
				log << "saving record " << tapes[save_tape_id].get_current_record() << std::endl;
				output_tape.write_next_record(tapes[save_tape_id].get_current_record());

				// get next record, since we just saved one
				tapes[save_tape_id].read_next_record();

				// if there is still data on tape
				if (!tapes[save_tape_id].get_current_record().is_valid()) {
					// no more data on tape - series ended for sure
					break;
				}
			}
			// continue if series on is still progressing
			while (tapes[save_tape_id].is_progressing(sorting_policy));

			log << std::endl;
		}
		// continue if both tapes are not empty
		while (tapes[id1].get_current_record().is_valid() && tapes[id2].get_current_record().is_valid());

		log << "no more data on tape " << id2 << " - merge ended" << std::endl;
		log << "current record of tape " << id1 << " after merge: " << tapes[id1].get_current_record() << std::endl;
	};

	/* distribution phase */

	auto output_tape_id = std::size_t(1);

	// read first record of file
	if (!tapes[0].is_empty()) {
		tapes[0].read_next_record();
	}
	else {
		// file has no data - tape sorted
		copy_file(input_path, output_path);
		return std::make_tuple(0, tapes[0].get_page_operations() + tapes[1].get_page_operations() + tapes[2].get_page_operations());;
	}

	// write 1 series to tape1 (no need to worry about series joining yet, however, need to keep last written record)
	series_written = write_n_series(0, output_tape_id, 1);

	// first series is on tape1

	// repeat process until all series has been distributed
	while (tapes[0].get_current_record().is_valid()) {
		// change active_output_tape
		output_tape_id = output_tape_id == 1 ? 2 : 1;

		// write fib series from input to active_tape (take series joining into account)
		series_written = write_n_series(0, output_tape_id, fib);

		// next fibonacci number
		std::tie(fib_last, fib) = std::make_tuple(fib, fib_last + fib);
	}

	// tape contained 1 series only (we didn't enter while loop above)
	if (fib_last == 0) {
		log << "Only one series was on a file" << std::endl;
		copy_file(input_path, output_path);
		return std::make_tuple(0, tapes[0].get_page_operations() + tapes[1].get_page_operations() + tapes[2].get_page_operations());
	}

	log << "tapes after distribution phase" << std::endl;
	// close all tapes
	for (int i = 0; i < 3; ++i) {
		tapes[i].close();

		log << "-----------------tape" << i << "------------------" << std::endl;
		print_file(tapes[i].get_filepath(), log);
	}
	log << std::endl;
	/* end of distribution phase */

	/* merge phase */
	tapes[0].open(output_path, Tape::write);		// change path to prevent overriding input file
	tapes[1].open(tapes[1].get_filepath(), Tape::read);
	tapes[2].open(tapes[2].get_filepath(), Tape::read);

	// output_tape is last being written to, so it is "bigger" - it may contains dummy 
	auto bigger_tape_id = output_tape_id;
	auto smaller_tape_id = std::size_t(bigger_tape_id == 1 ? 2 : 1);
	output_tape_id = 0;

	// if in last write there were no actual series written in this case output tape was actually "smaller"
	if (series_written == 0) {
		// swap tapes
		std::swap(bigger_tape_id, smaller_tape_id);
	}

	// counter of phases
	auto phases_count = std::size_t(0);

	// read record of data tapes - bigger and smaller
	if (!tapes[bigger_tape_id].is_empty() && !tapes[smaller_tape_id].is_empty()) {
		tapes[smaller_tape_id].read_next_record();
		tapes[bigger_tape_id].read_next_record();
	}
	else {
		// something is very, very, very bad
		throw std::exception("Unknown very, very, very bad error");
	}

	do {
		// if dummy runs should be resolved, rewrite 'dummy_runs' series from "smaller" tape to output tape
		if (dummy_runs > 0) {
			log << dummy_runs << " dummy runs detected" << std::endl;

			// rewrite dummy runs to bigger tape
			series_written = write_n_series(smaller_tape_id, output_tape_id, dummy_runs);

			if (dummy_runs != series_written) {
				// who knows what that means, but in case it happens
				throw std::exception("Unknown error, dummy runs cannot have been written");
			}

			// no more dummy runs will ever be resolved again
			dummy_runs = 0;

			// dummy runs ended file
			if (!tapes[smaller_tape_id].get_current_record().is_valid()) {
				throw std::exception("Dummy runs ended tape");

				// not implemented yet - should not happen with current optimization
			}
		}

		/*
		for (int i = 0; i < 3; ++i) {
			tapes[i].close();

			log << "-----------------tape" << i << "------------------" << std::endl;
			print_file(tapes[i].get_filepath(), log);
		}*/

		// merge 2 tapes
		merge(bigger_tape_id, smaller_tape_id, output_tape_id);

		log << std::endl;
		log << "tapes after " << phases_count << " phase" << std::endl;
		log << "-----------------tape" << smaller_tape_id << "------------------" << std::endl;
		log << "-----------------empty------------------" << std::endl;
		// switch smaller tape (which is empty now) to write mode
		tapes[smaller_tape_id].close();
		tapes[smaller_tape_id].open(tapes[smaller_tape_id].get_filepath(), Tape::write);

		// switch output tape to read mode
		tapes[output_tape_id].close();
		tapes[output_tape_id].open(tapes[output_tape_id].get_filepath(), Tape::read);
		// we should read first record here
		tapes[output_tape_id].read_next_record();

		log << "-----------------tape" << output_tape_id << "------------------" << std::endl;
		print_file(tapes[output_tape_id].get_filepath(), log);

		// bigger tape is still opened
		log << "-----------------tape" << bigger_tape_id << "------------------" << std::endl;
		log << "-----current record: " << tapes[bigger_tape_id].get_current_record() << "-----" << std::endl;
		print_file(tapes[bigger_tape_id].get_filepath(), log);

		log << std::endl;

		// switch tapes' ids properly
		std::tie(smaller_tape_id, output_tape_id, bigger_tape_id) = std::make_tuple(bigger_tape_id, smaller_tape_id, output_tape_id);

		++phases_count;
	}
	// continue until bigger tape (smaller after switch) has data
	while (tapes[smaller_tape_id].get_current_record().is_valid());

	// close all tapes
	for (int i = 0; i < 3; ++i) {
		tapes[i].close();
	}

	// if sorted file is not on output_path
	if (tapes[bigger_tape_id].get_filepath() != output_path) {
		copy_file(tapes[bigger_tape_id].get_filepath(), output_path);
	}

	log << std::endl;
	log << "sorted file after " << phases_count << " phases" << std::endl;
	log << "-----------------tape" << bigger_tape_id << "------------------" << std::endl;
	print_file(output_path, log);

	return std::make_tuple(phases_count, tapes[0].get_page_operations() + tapes[1].get_page_operations() + tapes[2].get_page_operations());
}