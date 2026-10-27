#include <chrono>
#include <iostream>
#include <sysc/kernel/sc_externs.h>
#include <sysc/kernel/sc_module.h>
#include <systemc.h>

#define PAGES_PER_BLOCK 64
#define BLOCKS_PER_PLANE 16
#define PLANES_PER_DIE 2
#define DIES_PER_SSD 2

const sc_time PROG_TIME(40, SC_US);
const sc_time READ_TIME(10, SC_US);
const sc_time ERS_TIME(1000, SC_US);

struct page {
	sc_mutex mutex;
};

struct block {
	long wear = 0;
	struct page pages[PAGES_PER_BLOCK];
};

struct plane {
	sc_mutex mutex; // for parallel plane reads
	struct block blocks[BLOCKS_PER_PLANE];
};

struct die {
	sc_mutex mutex; // for most things
	struct plane planes[PLANES_PER_DIE];
};

struct ssd {
	struct die dies[DIES_PER_SSD];
};

struct ssd drive;

void program(int d, int pl, int b, int pa) {
	struct block &blk = drive.dies[d].planes[pl].blocks[b];
	blk.wear++; // how much time should wear add? see OFNI manual?
	wait(ERS_TIME);
	wait(PROG_TIME);
	drive.dies[d].mutex.unlock();
}

SC_MODULE(Host) {
	SC_CTOR(Host) {
		SC_THREAD(run);
	}
	void run() {
		if (sc_argc() < 6) {
			std::cerr << "use like: ./main <num programs>\n";
			return;
		}
		for (int i = 0; i < std::atoi(sc_argv()[1]); i++) {
			int d = std::atoi(sc_argv()[2]);
			int pl = std::atoi(sc_argv()[3]);
			int b = std::atoi(sc_argv()[4]);
			int pa = std::atoi(sc_argv()[5]);
			program(d, pl, b, pa);
		}
	}
};


int sc_main(int argc, char **argv) {
	cout << endl << DIES_PER_SSD << " dies" << endl;
	cout << PLANES_PER_DIE << " planes per die" << endl;
	cout << BLOCKS_PER_PLANE << " blocks per plane" << endl;
	cout << PAGES_PER_BLOCK << " pages per block" << endl;

	int d = std::atoi(argv[2]);
	int pl = std::atoi(argv[3]);
	int b = std::atoi(argv[4]);
	int pa = std::atoi(argv[5]);

	drive.dies[d].mutex.lock();
	cout << endl << "LOCKING die " << d << endl;
	cout << "WEARING and ERASING " << "block " << b << " of its plane " << pl << endl;
	cout << "PROGRAMING its page " << pa << endl;
	cout << "Repeating the above " << argv[1] << " times" << endl << endl;

	Host host("host");
	auto start = std::chrono::steady_clock::now();
	sc_start();
	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double, std::milli> elapsed = end - start;

	cout << "TIME SPENT: " << elapsed.count() << " MILLISECONDS" << endl;
	return 0;
}