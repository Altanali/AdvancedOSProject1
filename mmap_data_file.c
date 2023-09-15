#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <math.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define handle_error(msg) \
	do { perror(msg); exit(EXIT_FAILURE); } while (0);

#define PAGE_SIZE sysconf(_SC_PAGE_SIZE)

// Helper Functions
void shuffle(u_int64_t *array, size_t n);

int main(int argc, char *argv[]) {
	struct stat file_stats;
	int fd;
	size_t file_size;
	char *memory_map_addr;

	fd = open(argv[1], O_RDWR);
	if(fd == -1) {
		handle_error("Failed to open file.\n");
	}
	fstat(fd, &file_stats);
	file_size = file_stats.st_size;

	memory_map_addr = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

	if(memory_map_addr == MAP_FAILED) {
		handle_error("mmap call failed\n");
	}

	size_t num_pages = (file_size + PAGE_SIZE - 1)/(PAGE_SIZE);

	u_int64_t *indices = malloc(num_pages*sizeof(u_int64_t));

	if(!indices) {
		handle_error("Malloc failed when trying to create indices array.");
	}

	for(size_t i = 0; i < num_pages; ++i) indices[i] = i;
	shuffle(indices, num_pages);

	for(size_t i = 0; i < num_pages; ++i) {
		//Write 0 to the first page of each page
		memory_map_addr[indices[i]*PAGE_SIZE] = 0;
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