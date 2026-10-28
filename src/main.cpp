	#include <cstring>
#include <sysc/kernel/sc_simcontext.h>
	#include <iostream>
	#include <systemc.h>
	#include <optional>

	#define PAGES_PER_BLOCK 64
	#define BLOCKS_PER_PLANE 16
	#define PLANES_PER_DIE 2
	#define DIES_PER_SSD 2
	#define MAX_WEAR 10000000

	const sc_time PROG_TIME(40, SC_US);
	const sc_time READ_TIME(10, SC_US);
	const sc_time ERS_TIME(1000, SC_US);

	enum State {
		VALID, 		// programmed and readable 
		INVALID,	// programmed and stale
		ERASED 		// programmable
	};

	struct page {
		sc_mutex mutex;
		State state = ERASED;
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

	std::optional<sc_time> nand_erase(int d, int pl, int b) {
		sc_time start = sc_time_stamp();
		block &bl = drive.dies[d].planes[pl].blocks[b];

		drive.dies[d].mutex.lock();
		wait(ERS_TIME);
		bl.wear++; // TODO scalar. faster for pages but longer for blocks
		for (int i = 0; i < PAGES_PER_BLOCK; i++) {
			bl.pages[i].state = ERASED;
		}
		drive.dies[d].mutex.unlock();

		return sc_time_stamp() - start;
	}

	std::optional<sc_time> nand_program(int d, int pl, int b, int pa) {
		sc_time start = sc_time_stamp();
		page &pg = drive.dies[d].planes[pl].blocks[b].pages[pa];

		drive.dies[d].mutex.lock();
		if (pg.state != ERASED) {
			drive.dies[d].mutex.unlock();
			return std::nullopt;
		}
		wait(PROG_TIME);
		pg.state = VALID;
		drive.dies[d].mutex.unlock();

		return sc_time_stamp() - start;
	}

	std::optional<sc_time> nand_read(int d, int pl, int b, int pa) {
		sc_time start = sc_time_stamp();
		page &pg = drive.dies[d].planes[pl].blocks[b].pages[pa];

		drive.dies[d].mutex.lock();
		if (pg.state != VALID) {
			drive.dies[d].mutex.unlock();
			return std::nullopt;
		}
		wait(READ_TIME);
		drive.dies[d].mutex.unlock();

		return sc_time_stamp() - start;
	}

	SC_MODULE(Host) {
		SC_HAS_PROCESS(Host);
		// members go here
		FILE *csvin;
		FILE *csvout;

		// constructor
		Host(sc_module_name name, FILE *fin, FILE *fout) : sc_module(name), csvin(fin), csvout(fout) {
			SC_THREAD(run);
		}

		// thread
		void run() {
			sc_time start = sc_time_stamp();
			sc_time sumlat(0, SC_US);

			char line[256];
			fgets(line, sizeof(line), csvin);
			
			fprintf(csvout, "op,die,plane,block,page,issued_us,completed_us,latency_us\n");

			int line_num = 0;
			while (fgets(line, sizeof line, csvin)) {
				double iss = sc_time_stamp() / sc_time(1, SC_US);
				
				char op[8];
				int d, pl, b, pa;
				int n = sscanf(line, "%7[^,],%d,%d,%d,%d", op, &d, &pl, &b, &pa);

				if (n != 5) {
					cerr << "bad CSVin line " << line_num << ": " << line;	
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
				double opt_d = 0;
				double done = sc_time_stamp() / sc_time(1, SC_US);
				if (optime) {
					sumlat += *optime;
					opt_d = *optime / sc_time(1, SC_US);
					fprintf(csvout, "%s,%d,%d,%d,%d,%.0f,%.0f,%.0f\n", op, d, pl, b, pa, iss, done, opt_d);
				} else { // FAILURE
					fprintf(csvout, "%s,%d,%d,%d,%d,%.0f,%.0f,\n", op, d, pl, b, pa, iss, done);
				}
				line_num++;
			}
			sc_time end = sc_time_stamp();
			cout << "Summed Latency: " << sumlat << endl;
			cout << "Total Runtime: " << end - start << endl;
		}
	};

	int sc_main(int argc, char **argv) {
		cout << endl << DIES_PER_SSD << " dies" << endl;
		cout << PLANES_PER_DIE << " planes per die" << endl;
		cout << BLOCKS_PER_PLANE << " blocks per plane" << endl;
		cout << PAGES_PER_BLOCK << " pages per block" << endl;

		if (argc < 2) {
			cerr << "usage: ./main <trace_filename>";
			return 1;
		}

		FILE *fin = fopen(sc_argv()[1], "r");
		if (fin == NULL) {
			perror("bad fopen!");
			return 1;
		}

		FILE *fout = fopen(sc_argv()[2], "w");
		if (fout == NULL) {
			perror("bad fopen!");
			return 1;
		}

		Host host("host", fin, fout);
		sc_start();
		
		fclose(fin);
		fclose(fout);
		return 0;
	}