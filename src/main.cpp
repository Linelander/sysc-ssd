	#include <iostream>
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

	sc_time nand_program(int d, int pl, int b, int pa) {
		sc_time start = sc_time_stamp();

		drive.dies[d].mutex.lock();
		wait(PROG_TIME);
		drive.dies[d].mutex.unlock();

		return sc_time_stamp() - start;
	}

	sc_time nand_erase(int d, int pl, int b) {
		sc_time start = sc_time_stamp();

		drive.dies[d].mutex.lock();
		wait(ERS_TIME);
		drive.dies[d].planes[pl].blocks[b].wear++; // TODO scalar. faster for pages but longer for blocks
		drive.dies[d].mutex.unlock();

		return sc_time_stamp() - start;
	}

	sc_time nand_read(int d, int pl, int b, int pa) {
		sc_time start = sc_time_stamp();

		drive.dies[d].mutex.lock();
		wait(READ_TIME);
		drive.dies[d].mutex.unlock();

		return sc_time_stamp() - start;
	}

	SC_MODULE(Host) {
		SC_CTOR(Host) {
			SC_THREAD(run);
		}
		void run() {
			// CSV: P/E, d, pl, b, pa?
		}
	};

	int sc_main(int argc, char **argv) {
		cout << endl << DIES_PER_SSD << " dies" << endl;
		cout << PLANES_PER_DIE << " planes per die" << endl;
		cout << BLOCKS_PER_PLANE << " blocks per plane" << endl;
		cout << PAGES_PER_BLOCK << " pages per block" << endl;

		// might want to configure everything based on the args here

		// one arg should be the file being processed

		Host host("host");
		sc_start();

		return 0;
	}