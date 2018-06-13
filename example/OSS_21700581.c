#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../jsmn.h"
#include "../jsmn.c"

/*
 * A small example of jsmn parsing when JSON structure is known and number of
 * tokens is predictable.
 */

typedef struct {
	char model[20];
	char maker[20];
	int year;
	char gastype[20];
} car_t;

char * readjsonfile(const char * filename)
{
	char input[1000]; // can read up to 1000 characters at a time
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

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
	if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
			strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
		return 0;
	}
	return -1;
}

int makecarlist (const char *json, jsmntok_t *t, int tokcount, car_t * list[])
{
	int prev_tok; // 1: model, 2: maker, 3: year, 4: gas
	int car_count = -1;
	int i, j;
	int current_year;
	char * subString = (char *) malloc(21 * sizeof(char));

	for (i = 1; i < tokcount; i ++ ) {
		
		// 현재 토큰이 JSMN_STRING이나 JSMN_PRIMITIVE가 아니면 스킵
		if (t[i].type != 3 && t[i].type != 4)
			continue;

		// previous token 판별
		// name 토큰을 읽었을 때는, mycar_t 메모리 동적 할당도 해줌
		if (jsoneq(json, &t[i], "cars") == 0) {
			continue;
		}
		else if (jsoneq(json, &t[i], "model") == 0) {
			prev_tok = 1;			
			list[++car_count] = (car_t *)malloc(sizeof(car_t));
			continue;
		}
		else if (jsoneq(json, &t[i], "maker") == 0) {
			prev_tok = 2;
			continue;
		}
		else if (jsoneq(json, &t[i], "year") == 0) {
			prev_tok = 3;
			continue;
		}
		else if (jsoneq(json, &t[i], "gas") == 0) {
			prev_tok = 4;
			continue;
		}

		// 토큰이 구조체에 저장해야 할 내용이고 String 타입이면, 토큰 내용만 뽑아 'subString'에 따로 저장함
		// We expect the contents of one token is less then 30 chars
		
		// Initialize subString
		for(j = 0; j < 20; j ++)
			subString[j] = '\0';
			
		for(j = t[i].start; j < t[i].end; j ++) 
			subString[j - t[i].start] = json[j];

		subString[j] = '\0';

		// 구조체에 정보 저장
		if(prev_tok == 1) {
			strcpy(list[car_count]->model, subString);
		}
		else if(prev_tok == 2) {
			strcpy(list[car_count]->maker, subString);
		}
		else if(prev_tok == 3) {
			current_year = atoi(subString);
			list[car_count]->year = current_year;
		}
		else {
			strcpy(list[car_count]->gastype, subString);
		}
	}
	return ++car_count;
}

void printcarlist(car_t * list[], int carcount)
{
	int i;

	// 저장된 구조체 배열을 알맞게 출력
	printf("번호	모델명	제조사	제조년도	연료타입\n");
	for(i = 0; i < carcount; i ++) {
		printf("%d	%s	%s	%d		%s\n", i+1, list[i]->model, list[i]->maker, list[i]->year, list[i]->gastype);
	}
}

int main() {
	int i;
	int r;
	jsmn_parser p;
	jsmntok_t t[128]; /* We expect no more than 128 tokens */
	car_t * carlist[5];
	int carcount;

	char *JSON_STRING = readjsonfile("cars.json");

	jsmn_init(&p);
	r = jsmn_parse(&p, JSON_STRING, strlen(JSON_STRING), t, sizeof(t)/sizeof(t[0]));

	carcount = makecarlist(JSON_STRING, t, r, carlist);

	printcarlist(carlist, carcount);
	
	return EXIT_SUCCESS;
}
