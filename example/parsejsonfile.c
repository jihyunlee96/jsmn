#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../jsmn.h"
#include "../jsmn.c"

/*
 * A small example of jsmn parsing when JSON structure is known and number of
 * tokens is predictable.
 */

char * readjsonfile()
{
	char input[200]; // can read up to 200 characters at a time
	char *result;
	
	FILE *fp;
	fp = fopen("JSON_STRING.txt", "rt");

	// first allocation
	fgets(input, sizeof(input), fp);

	result = (char *) malloc(strlen(input));
	strncpy(result, input, strlen(input));

	// second ~ nth allocation
	while(!feof(fp)) {
		fgets(input, sizeof(input), fp);
		result = (char *) realloc(result, strlen(input));
		strncpy(result + strlen(result), input, strlen(input));

		puts(input);
		printf("\n");
	}
	
	result[strlen(result)] = '\0';

	return result;
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
	
	char *JSON_STRING = readjsonfile();

	printf("Initial JSON String:\n%s\n\n", JSON_STRING);

	jsmn_init(&p);
	r = jsmn_parse(&p, JSON_STRING, strlen(JSON_STRING), t, sizeof(t)/sizeof(t[0]));
	if (r < 0) {
		printf("Failed to parse JSON: %d\n", r);
		return 1;
	}

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

			// Make sure the first letter of key is capital
			if('a' < *(JSON_STRING + t[i].start) && *(JSON_STRING + t[i].start) < 'z')
				*(JSON_STRING + t[i].start) -= 32;

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
