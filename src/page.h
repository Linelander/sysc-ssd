#ifndef SSD_PAGE
#define SSD_PAGE

#include <systemc.h>
#include <stdio.h>

class block;

class page {
	public:
		sc_mutex lock;
		sc_time program{40, SC_US}; 	// must erase block. pages could acquire their block's lock and then program
		sc_time read{10, SC_US};
		block *parent = nullptr;
};

#endif