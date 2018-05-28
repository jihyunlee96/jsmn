#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../jsmn.h"
#include "../jsmn.c"

#define TRUE 1
#define FALSE 0

int keycount;

/*
 * A small example of jsmn parsing when JSON structure is known and number of
 * tokens is predictable.
 */

char * readjsonfile(const char * filename)
{
	char input[500]; // can read up to 500 characters at a time
	char *result;
	
	FILE *fp;
	fp = fopen(filename, "rt");

	// first allocation
	fgets(input, sizeof(input), fp);

	result = (char *) malloc(strlen(input));
	strncpy(result, input, strlen(input));

	// second ~ nth allocation
	while(!feof(fp)) {
		fgets(input, sizeof(input), fp);
		result = (char *) realloc(result, sizeof(result) + sizeof(input));
		strcat(result, input);
	}
	
	result[strlen(result)] = '\0';

	return result;
}

void printall(const char *json, jsmntok_t *t, int tokcount)
{
	int i;

	printf("***** All Tokens *****\n");

	for (i = 1; i < tokcount; i ++ ) {
		char * type = malloc(20 * sizeof(char));
		int j, k;

		if (t[i].type == 1)
			type = "JSMN_OBJECT";
		else if (t[i].type == 2)
			type = "JSMN_ARRAY";
		else if (t[i].type == 3)
			type = "JSMN_STRING";
		else if (t[i].type == 4)
			type = "JSMN_PRIMITIVE";
		else
			type = "UNDEFINED";

		// We expect the contents of one token is less then 200 chars
		char * subString = (char *) malloc(200*sizeof(char));
		// Initialize subString
		for(j = 0; j < 200; j ++)
			subString[j] = '\0';
		
		for(j = t[i].start; j < t[i].end; j ++) 
			subString[j - t[i].start] = json[j];

		subString[j] = '\0';

		printf("[%2d] %s (size=%d, %d~%d, %s)\n", i, subString, t[i].size, t[i].start, t[i].end, type); 
		
		free(subString);
	}
	printf("\n");
}

void printkeys(const char *json, jsmntok_t *t, int tokcount)
{
	int i;
	int count = 1;

	printf("***** All Keys *****\n");

	for (i = 1; i < tokcount; i ++ ) {
		char * type = malloc(20 * sizeof(char));
		int j, k;

		if(t[i].size == 0)
			continue;
		else if(t[i].type != 3)
			continue;

		if (t[i].type == 1)
			type = "JSMN_OBJECT";
		else if (t[i].type == 2)
			type = "JSMN_ARRAY";
		else if (t[i].type == 3)
			type = "JSMN_STRING";
		else if (t[i].type == 4)
			type = "JSMN_PRIMITIVE";
		else
			type = "UNDEFINED";

		// We expect the contents of one token is less then 200 chars
		char * subString = (char *) malloc(200*sizeof(char));
		// Initialize subString
		for(j = 0; j < 200; j ++)
			subString[j] = '\0';
		
		for(j = t[i].start; j < t[i].end; j ++) 
			subString[j - t[i].start] = json[j];

		subString[j] = '\0';

		printf("[%2d] %s (%d)\n", count++, subString, i); 
		
		free(subString);
	}
	printf("\n");
}

int findkeys(const char *json, jsmntok_t *t, int tokcount, int *keys)
{
	int i;
	int count = 0;

	for (i = 1; i < tokcount; i ++ ) {
		if(t[i].size == 0)
			continue;
		else if(t[i].type != 3)
			continue;

		keys[count++] = i;
	}
	return count;
}

void printvalues(const char *json, jsmntok_t *t, int tokcount, int *keys) {
	int i, j;
	int isKey;

	printf("***** Print values ******\n");

	for (i = 1; i < tokcount; i++) {
		isKey = FALSE;
		for(j = 0; j < keycount; j++) {

			if(i == keys[j]) {
				isKey = TRUE;
				break;
			}
		}

		if(isKey == TRUE) {
			printf("- %.*s: ", t[i].end-t[i].start,
					json + t[i].start);
		}
		else {
			printf("%.*s\n", t[i].end-t[i].start,
					json + t[i].start);
		}
	}
}

static int jsoneq(char *json, jsmntok_t *tok, char *s) {
	if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
			strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
		return 0;
	}
	return -1;
}

int main() {
	int i;
	int r;
	jsmn_parser p;
	jsmntok_t t[128]; /* We expect no more than 128 tokens */
	
	char *JSON_STRING = readjsonfile("data.json");

	printf("Initial JSON String:\n%s\n\n", JSON_STRING);

	jsmn_init(&p);
	r = jsmn_parse(&p, JSON_STRING, strlen(JSON_STRING), t, sizeof(t)/sizeof(t[0]));
	if (r < 0) {
		printf("Failed to parse JSON: %d\n", r);
		return 1;
	}

	printall(JSON_STRING, t, r);

	printkeys(JSON_STRING, t, r);

	int keyarray[100];
	keycount = findkeys(JSON_STRING, t, r, keyarray);

	printvalues(JSON_STRING, t, r, keyarray);

	return EXIT_SUCCESS;
}
