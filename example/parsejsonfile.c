#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../jsmn.h"
#include "../jsmn.c"

/*
 * A small example of jsmn parsing when JSON structure is known and number of
 * tokens is predictable.
 */

const char * readjsonfile(const char * filename)
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

static int jsoneq(const char *json, jsmntok_t *tok, char *s) {
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
	
	const char *JSON_STRING = readjsonfile("data.json");

	printf("Initial JSON String:\n%s\n\n", JSON_STRING);

	jsmn_init(&p);
	r = jsmn_parse(&p, JSON_STRING, strlen(JSON_STRING), t, sizeof(t)/sizeof(t[0]));
	if (r < 0) {
		printf("Failed to parse JSON: %d\n", r);
		return 1;
	}
	
	printall(JSON_STRING, t, r);

	printkeys(JSON_STRING, t, r);
	
	/* Assume the top-level element is an object */
	if (r < 1 || t[0].type != JSMN_OBJECT) {
		printf("Object expected\n");
		return 1;
	}

	/* Loop over all keys of the root object */
	for (i = 1; i < r; i++) {
		if (jsoneq(JSON_STRING, &t[i], "user") == 0) {
			/* We may use strndup() to fetch string value */
			printf("- User: %.*s\n", t[i+1].end-t[i+1].start,
					JSON_STRING + t[i+1].start);
			i++;
		} else if (jsoneq(JSON_STRING, &t[i], "admin") == 0) {
			/* We may additionally check if the value is either "true" or "false" */
			printf("- Admin: %.*s\n", t[i+1].end-t[i+1].start,
					JSON_STRING + t[i+1].start);
			i++;
		} else if (jsoneq(JSON_STRING, &t[i], "uid") == 0) {
			/* We may want to do strtol() here to get numeric value */
			printf("- UID: %.*s\n", t[i+1].end-t[i+1].start,
					JSON_STRING + t[i+1].start);
			i++;
		} else if (jsoneq(JSON_STRING, &t[i], "groups") == 0) {
			int j;
			printf("- Groups:\n");
			if (t[i+1].type != JSMN_ARRAY) {
				continue; /* We expect groups to be an array of strings */
			}
			for (j = 0; j < t[i+1].size; j++) {
				jsmntok_t *g = &t[i+j+2];
				printf("  * %.*s\n", g->end - g->start, JSON_STRING + g->start);
			}
			i += t[i+1].size + 1;
		} else { /* Managing unexpected keys (Must be in key / value pair) */

			// Print key with its value
			printf("- %.*s:", t[i].end-t[i].start,
					JSON_STRING + t[i].start);
			printf(" %.*s\n", t[i+1].end-t[i+1].start,
					JSON_STRING + t[i+1].start);
			i ++;
		}
	}
	return EXIT_SUCCESS;
}
