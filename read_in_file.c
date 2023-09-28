#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)


void read_into_cache(int fd);
int main(int argc, char *argv[]) {
	int fd = open(argv[1], O_RDONLY);
	read_into_cache(fd);
	return 0;
}

void read_into_cache(int fd) {

	FILE *fptr = fdopen(fd, "r");

	if((fptr == NULL)) {
		handle_error("Failed to open data file\n");
	}
	

	for(int i = 0; i < 2; ++i) {
		int c = fgetc(fptr);
		while(c != EOF) c = fgetc(fptr);
		fseek(fptr, 0, SEEK_SET);
		printf("%d\n", i);
	}
	
	return;
}