#ifndef SSD_BLOCK
#define SSD_BLOCK

#include <systemc.h>
#include <stdio.h>
#include "page.h"

class block {
	public:
		block() {
			for (int i = 0; i < PAGES_PER_BLOCK; i++) {
				pages[i].parent = this;
			}
		}

		static constexpr int PAGES_PER_BLOCK = 64; // placeholder
		sc_mutex lock; // pages will acquire their blocks' lock when a program happens. might want to get rid of oop...
		sc_time erase{1000, SC_US}; // placeholder
		page pages[PAGES_PER_BLOCK];

};

#endif