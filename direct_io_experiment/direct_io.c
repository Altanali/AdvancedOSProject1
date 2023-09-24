#define _GNU_SOURCE //Tell compiler to access GNU/Linux extension functions

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <getopt.h>
#include <sys/mman.h>
#include <string.h>
#include <sched.h>
#include <time.h>
#include <sys/resource.h>

#define	IO_SIZE 4096
#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)


void do_file_io(int fd, char *buf, 
      u_int64_t *offset_array, size_t n, int opt_read);
void shuffle(u_int64_t *array, size_t n);
void read_into_cache(int fd);

int main(int argc, char *argv[]) {

	srand(time(NULL)); //seeding randomization with current time

	cpu_set_t  mask;
	CPU_ZERO(&mask);
	CPU_SET(0, &mask);
	CPU_SET(2, &mask);
	if(sched_setaffinity(0, sizeof(mask), &mask) == -1) {
		handle_error("Failed to bind process to Core 0");
	}

	if(argc == 1) {
		printf("No arguments provided!\n");
		return -1;
	}

	char options[] = "f:r::w::s::d::";
	int opt;
	char *filename;
	//Default Flag Values
	int flags = 0;
	char read_into_cache_true = 0;
	char print_rusage = 0;
	int write_operation = 0; //0 implies read operation
	int sequential = 0; //0 implies random access



	//Argument Parsing
	if(argc > 2) {
		while((opt = getopt(argc, argv, options)) != -1) {	
			switch(opt) {
				case('f'):
					filename = strdup(optarg);
					break;
				case('w'): 
					write_operation = 1;
					break;
				case('r'):
					read_into_cache_true = 1;
					break;
				case('d'):
					print_rusage = 1;
					break;
				case('s'):
					sequential = 1;
					break;
				case(':'):
					printf("Missing arg: %c\n", options[optind]);
				default:
					break;
			}
		}
	}
	// if(write_operation) printf("Performing WRITE operation.\n");
	// else 				printf("Performing READ operation.\n");
	
	// if(sequential) 		printf("Performing operation SEQUENTIALLY.\n");
	// else 				printf("Performing operation RANDOMLY.\n");

	
	int fd = open(filename, write_operation ? O_WRONLY : O_RDONLY, __O_DIRECT);
	if(read_into_cache_true)
		read_into_cache(fd);
	struct stat file_stats;
	fstat(fd, &file_stats);
	size_t file_size = file_stats.st_size;
	size_t num_ofs = (file_size + IO_SIZE - 1)/(IO_SIZE);
	u_int64_t offset_array[num_ofs];
	for(size_t i = 0; i < num_ofs; ++i) {
		offset_array[i] = i*IO_SIZE;
	}

	//shuffle offset indices if performing random access
	if(!sequential) shuffle(offset_array, num_ofs);
	char *buffer = malloc(IO_SIZE);
	if(!buffer) handle_error("malloc of char buffer failed!\n");

	if(write_operation) {
		//If we're performing a write, write random letters to the buffer
		for(size_t i = 0; i < IO_SIZE; ++i) {
			buffer[i] = 'A' + (random() % 26);
		}
	}

	do_file_io(fd, buffer, offset_array, num_ofs, !write_operation);
	free(buffer);

	if(print_rusage) {
		struct rusage *usage = malloc(sizeof(struct rusage));
		
		if(!usage) {
			handle_error("Failled to mallocate memory for an rusage instance.\n");
		}
		if(getrusage(RUSAGE_SELF, usage) == -1) {
			handle_error("Failed to get rusage\n");
		}
		printf("utime (ms): %lu\nstime (ms): %lu\nmaxrss: %lu\nminflt: %lu\nmajflt: %lu\ninblock: %lu\noutblock: %lu\nvcsw: %lu\nivsw: %lu\n\n",\
				usage->ru_utime.tv_usec, usage->ru_stime.tv_usec, usage->ru_maxrss, usage->ru_minflt, usage->ru_majflt, usage->ru_inblock, usage->ru_oublock,\
				usage->ru_nvcsw, usage->ru_nivcsw);
		free(usage);
	}
}

void do_file_io(int fd, char *buf, 
      u_int64_t *offset_array, size_t n, int opt_read)
{
  int ret = 0;
  for (int i = 0; i < n; i++) {
    ret = lseek(fd, offset_array[i], SEEK_SET);
    if (ret == -1) {
      perror("lseek");
      exit(-1);
    }
    if (opt_read)
      ret = read(fd, buf, IO_SIZE);
    else
      ret = write(fd, buf, IO_SIZE);
    if (ret == -1) {
      perror("read/write");
      exit(-1);
    }
  }
}

void shuffle(u_int64_t *array, size_t n)
{
	if (n > 1) {
		size_t i;
		for (i = 0; i < n - 1; i++) {
		size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
		u_int64_t t = array[j];
		array[j] = array[i];
		array[i] = t;
		}
	}
}

void read_into_cache(int fd) {

	FILE *fptr = fdopen(fd, "r");

	if((fptr == NULL)) {
		handle_error("Failed to open data file\n");
	}
	

	for(int i = 0; i < 10; ++i) {
		int c = fgetc(fptr);
		while(c != EOF) c = fgetc(fptr);
		fseek(fptr, 0, SEEK_SET);
		printf("%d\n", i);
	}
	
	return;
}