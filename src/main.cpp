#include <array>
#include <cstring>
#include <iostream>
#include <optional>
#include <sysc/communication/sc_mutex.h>
#include <sysc/kernel/sc_simcontext.h>
#include <sysc/kernel/sc_wait.h>
#include <systemc.h>

constexpr int PAGES_PER_BLOCK = 64;
constexpr int BLOCKS_PER_PLANE = 16;
constexpr int PLANES_PER_DIE = 2;
constexpr int DIES_PER_SSD = 2;
constexpr int MAX_WEAR = 10000000;

// TODO: replace with timings from OFNI manual
const sc_time TRANS_TIME(20, SC_US);
const sc_time PROG_TIME(40, SC_US);
const sc_time SENSE_TIME(10, SC_US);
const sc_time ERS_TIME(1000, SC_US);

enum State {
	VALID,	 // programmed and readable
	INVALID, // programmed and stale
	ERASED	 // programmable
};

struct Guard {
	sc_mutex &m;

	Guard(sc_mutex &mtx) : m(mtx) { m.lock(); }
	~Guard() { m.unlock(); }

	Guard(const Guard &) = delete;		  // copy constructor overwrite
	Guard &operator=(const Guard &) = delete; // reassignment ban
};

struct page {
	State state = ERASED;
};

struct block {
	long wear = 0;
	std::array<page, PAGES_PER_BLOCK> pages;
};

struct plane {
	sc_mutex pl_mut; // for parallel plane reads
	std::array<block, BLOCKS_PER_PLANE> blocks;
};

struct die {
	sc_mutex pin_mut; // for most things
	std::array<plane, PLANES_PER_DIE> planes;
};

struct ssd {
	std::array<die, DIES_PER_SSD> dies;
};

struct ssd drive;

std::optional<sc_time> nand_erase(int d, int pl, int b) {
	sc_time start = sc_time_stamp();
	block &bl = drive.dies[d].planes[pl].blocks[b];

	Guard pin(drive.dies[d].pin_mut);
	wait(ERS_TIME);
	bl.wear++; // TODO scalar. faster for pages but longer for blocks
	for (int i = 0; i < PAGES_PER_BLOCK; i++) {
		bl.pages[i].state = ERASED;
	}

	return sc_time_stamp() - start;
}

/*
	Transfer data from the SSD controller to plane reg
	Program them into the page
*/
std::optional<sc_time> nand_program(int d, int pl, int b, int pa) {
	sc_time start = sc_time_stamp();

	die &di = drive.dies[d];
	plane &pln = di.planes[pl];
	page &pg = pln.blocks[b].pages[pa];

	Guard plg(pln.pl_mut);
	if (pg.state != ERASED) {
		return std::nullopt;
	}

	// Transfer
	{
		Guard pin(di.pin_mut);
		wait(TRANS_TIME);
	}

	// Program
	wait(PROG_TIME);
	pg.state = VALID;

	return sc_time_stamp() - start;
}

/*
	sense plane cells into plane register
	transfer them to SSD controller
*/
std::optional<sc_time> nand_read(int d, int pl, int b, int pa) {
	sc_time start = sc_time_stamp();

	die &di = drive.dies[d];
	plane &pln = di.planes[pl];
	page &pg = pln.blocks[b].pages[pa];

	// Sense
	Guard plg(pln.pl_mut);
	if (pg.state != VALID) {
		return std::nullopt;
	}
	wait(SENSE_TIME);

	// Transfer
	{
		Guard pin(di.pin_mut);
		wait(TRANS_TIME);
	}

	return sc_time_stamp() - start;
}

SC_MODULE(Host) {
	SC_HAS_PROCESS(Host); // allows custom constructor
	// members go here
	FILE *csvin;
	FILE *csvout;

	// constructor
	Host(sc_module_name name, FILE * fin, FILE * fout)
		: sc_module(name), csvin(fin), csvout(fout) {
		SC_THREAD(run);
	}

	// thread
	void run() {
		sc_time start = sc_time_stamp();
		sc_time sumlat(0, SC_US);

		// start reading trace
		char line[256];
		fgets(line, sizeof(line), csvin);
		int line_num = 0;
		while (fgets(line, sizeof line, csvin)) {
			double iss = sc_time_stamp() / sc_time(1, SC_US);

			char op[8];
			int d, pl, b, pa;
			int n = sscanf(line, "%7[^,],%d,%d,%d,%d", op, &d, &pl,
				&b, &pa);

			if (n != 5) {
				cerr << "bad CSVin line " << line_num << ": "
				     << line;
				continue;
			}

			std::optional<sc_time> optime;
			if (strcmp(op, "prog") == 0) {
				optime = nand_program(d, pl, b, pa);
			}
			if (strcmp(op, "read") == 0) {
				optime = nand_read(d, pl, b, pa);
			}
			if (strcmp(op, "eras") == 0) {
				optime = nand_erase(d, pl, b);
			}

			// Write timings to output
			double opt_d = 0;
			double done = sc_time_stamp() / sc_time(1, SC_US);
			if (optime) {
				sumlat += *optime;
				opt_d = *optime / sc_time(1, SC_US);
				fprintf(csvout,
					"%s,%s,%d,%d,%d,%d,%d,%.0f,%.0f,%.0f\n",
					name(), op, 1, d, pl, b, pa, iss, done,
					opt_d);
			} else { // FAILURE
				fprintf(csvout,
					"%s,%s,%d,%d,%d,%d,%d,%.0f,%.0f,\n",
					name(), op, 0, d, pl, b, pa, iss,
					done); // no latency
			}
			line_num++;
		}
		sc_time end = sc_time_stamp();
		cout << endl;
		cout << "Results from " << name() << ":" << endl;
		cout << "Summed Latency: " << sumlat << endl;
		cout << "Total Runtime: " << end - start << endl;
		cout << endl;
	}
};

int sc_main(int argc, char **argv) {
	cout << endl << DIES_PER_SSD << " dies" << endl;
	cout << PLANES_PER_DIE << " planes per die" << endl;
	cout << BLOCKS_PER_PLANE << " blocks per plane" << endl;
	cout << PAGES_PER_BLOCK << " pages per block" << endl;

	if (argc < 4) {
		cerr << "usage: ./main <trace_filename>";
		return 1;
	}

	FILE *fin = fopen(sc_argv()[1], "r");
	if (fin == NULL) {
		perror("bad fopen!");
		return 1;
	}

	FILE *fin2 = fopen(sc_argv()[2], "r");
	if (fin2 == NULL) {
		perror("bad fopen!");
		return 1;
	}

	FILE *fout = fopen(sc_argv()[3], "w");
	if (fout == NULL) {
		perror("bad fopen!");
		return 1;
	}
	fprintf(fout, "host,op,ok,die,plane,block,page,issued_us,completed_"
		      "us,latency_us\n");

	Host host("host-1", fin, fout);
	Host host2("host-2", fin2, fout);

	sc_start();

	fclose(fin);
	fclose(fin2);
	fclose(fout);
	return 0;
}