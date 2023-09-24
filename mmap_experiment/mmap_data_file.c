
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

#define PAGE_SIZE sysconf(_SC_PAGE_SIZE)
#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)

// Helper Functions
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

	char options[] = "f:s::a::r::p::d::";
	int opt;
	char *filename;
	//Default Flag Values
	int flags = 0;
	char read_into_cache_true = 0;
	char print_rusage = 0;
	int MAP_PRIVACY = MAP_PRIVATE;


	//Argument Parsing
	if(argc > 2) {
		while((opt = getopt(argc, argv, options)) != -1) {	
			switch(opt) {
				case('f'):
					filename = strdup(optarg);
					break;
				case('s'): 
					printf("Setting MAP_SHARED.\n");
					MAP_PRIVACY = MAP_SHARED;
					break;
				case('a'):
					printf("Setting MAP_ANONYMOUS.\n");
					flags |= MAP_ANONYMOUS;
					break;
				case('r'):
					read_into_cache_true = 1;
					break;
				case('p'):
					printf("Setting MAP_POPULATE.\n");
					flags |= MAP_POPULATE;
					break;
				case('d'):
					printf("Printing RUSAGE data.\n");
					print_rusage = 1;
				case(':'):
					printf("Missing arg: %c\n", options[optind]);
				default:
					break;
			}
		}
	}

	if(MAP_PRIVACY == MAP_PRIVATE) printf("Setting MAP_PRIVATE.\n");
	flags |= MAP_PRIVACY;

	struct stat file_stats;
	int fd;
	size_t file_size;
	char *memory_map_addr;
	
	fd = open(filename, O_RDWR);
	if(fd == -1) {
		printf("%s\n", filename);
		handle_error("Failed to open file.\n");
	}
	//Read the file into system page cache
	if(read_into_cache_true) 
		read_into_cache(fd);


	if(fstat(fd, &file_stats) == -1) handle_error("Filed to get file stats.\n");
	file_size = file_stats.st_size;
	
	printf("Attempting to map %zu bytes.\n", file_size);
	memory_map_addr = mmap(NULL, file_size, PROT_READ | PROT_WRITE, flags, fd, 0);

	if(memory_map_addr == MAP_FAILED) {
		handle_error("mmap call failed\n");
	}
	
	size_t num_pages = (file_size + PAGE_SIZE - 1)/(PAGE_SIZE);
	u_int64_t *indices = malloc(num_pages*sizeof(u_int64_t));
	if(!indices) {
		handle_error("Malloc failed when trying to create indices array.");
	}

	//Create a random shuffling of the indices (although I'm seeding shuffle for reproducibility)
	for(size_t i = 0; i < num_pages; ++i) indices[i] = i;
	shuffle(indices, num_pages);
	for(size_t i = 0; i < num_pages; ++i) {
		//Write a random letter to the first page of each page, in the order indicated by the shuffled indices.
		char c = 'A' + (random() % 26);
		memory_map_addr[indices[i]*PAGE_SIZE] = c;
	}

	//flush the changes written in memory to the file
	if(msync(memory_map_addr, file_size, MS_SYNC) == -1) {
		handle_error("Failed to call mysnc and flush changes in memory to disk.\n");
	}

	//free the pages allocated mapped onto my mmap
	if(munmap(memory_map_addr, file_size) == -1) {
		handle_error("Failed to free allocated pages.\n");
	}

	// printf("Successfully unmapped file.\n");
	
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