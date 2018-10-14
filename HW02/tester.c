#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "dictionaryhandler.h"




int main() {
	char* file = "dict.txt";
	if(dictionary_setup(file, 1024)<0) {
		printf("Error setting up dict\n");
		exit(1);
	}
	char* word = (char*)malloc(sizeof(char*));
	srand(time(NULL));
	for(int i=0; i<10; ++i) {
		int len = getWord(word);
		printf("%d: %s\n", len, word);
	}
	dictionary_close();
	free(word);

	return 0;
}