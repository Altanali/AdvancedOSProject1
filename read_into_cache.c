#include <stdio.h>
#include <stdlib.h>


int main() {
	char *filename = "../data.txt";
	FILE *fptr = fopen(filename, "rb");

	if((fptr == NULL)) {
		perror("Failed to open data file\n");
		return(-1);
	}
	

	for(int i = 0; i < 10; ++i) {
		int c = fgetc(fptr);
		while(c != EOF) c = fgetc(fptr);
		fseek(fptr, 0, SEEK_SET);
		printf("%d\n", i);
	}
	
	return(0);
}
