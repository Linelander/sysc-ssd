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
	page pages[PAGES_PER_BLOCK];
};

struct plane {
	sc_mutex mutex; // for parallel plane reads
	block blocks[BLOCKS_PER_PLANE];
};

struct die {
	sc_mutex mutex; // for most things
	plane planes[PLANES_PER_DIE];
};

struct ssd {
	die dies[DIES_PER_SSD];
}

ssd drive;

void program(int d, int pl, int b, int pa) {
	drive[d].mutex.lock();
	blk = drive[d].dies[d].planes[pl].blocks[b];
	wait(ERS_TIME);
	wait(PROG_TIME);
	drive[d].mutex.unlock;
}

int sc_main(int argc, char **argv) {
	sc_start();
	for (int = 0; i < argc; i++) {
		program(0, 0, 0, 0)
	}
	return 0;
}