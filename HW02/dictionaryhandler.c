#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include "dictionaryhandler.h"

char** dict;
int num_words = 0;

int dictionary_setup(char* filename, int mwordlen) {
	FILE* fp;
	fp = fopen(filename, "r");
	if(fp == NULL) {
		return -1;
	}
	dict = (char**)malloc(100*sizeof(char**));
	int curr_max=100;
	size_t len = mwordlen;
	ssize_t read;
	while((read = getline(&dict[num_words], &len, fp)) != -1) {
		strtok(dict[num_words], "\n");
		num_words++;
		if(num_words==curr_max) {
			curr_max*=2;
			dict = (char**)realloc(dict, curr_max*sizeof(char**));
			
			if(dict==NULL) {
				printf("Realloc failure\n");
				return -1;
			}
		}

	}
	fclose(fp);
	return 0;
}


int getWord(char* word) {
	
	int i = rand()%num_words;
	strcpy(word, dict[i]);
	if(word==NULL) {
		perror("Error getting word");
		return -1;
	}
	return strlen(word)-1;
}

void dictionary_close() {
	for(int i=0; i<num_words; ++i) {
		free(dict[i]);
	}
	free(dict);
}