#include <systemc.h>

#define PAGES_PER_BLOCK 64
#define BLOCKS_PER_PLANE 16
#define PLANES_PER_DIE 2

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

die die;

void program(int pl, int b, int pa) {
	// block &parent_block = die.planes[pl].blocks[b];
	// parent_block.mutex.lock();
	// parent_block.pages[pa].mutex.lock();
	// wait(ERS_TIME);
	// wait(PROG_TIME);
	// parent_block.pages[pa].mutex.unlock();
	// parent_block.mutex.unlock();
}

int sc_main(int argc, char **argv) {
	sc_start();
	return 0;
}