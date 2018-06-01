#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../jsmn.h"

#define TRUE 1
#define FALSE 0

int keycount;

typedef enum {
	C_SET = 0,
	C_MEAL = 1
} mycategory_t;

typedef struct {
	mycategory_t cat;
	char name[30];
	char size[10];
	int price;
} mymenu_t;

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

static int jsoneq(const char *json, jsmntok_t *tok, char *s) {
	if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
			strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
		return 0;
	}
	return -1;
}

int makemymenu (const char *json, jsmntok_t *t, int tokcount, mymenu_t * m[])
{
	mycategory_t current_cat;
	int prev_tok; // 1: name, 2: size, 3: price
	int menu_count = -1;
	int current_price;
	int i, j;
	char * subString = (char *) malloc(30*sizeof(char));

	for (i = 1; i < tokcount; i ++ ) {
		// 현재 토큰이 JSMN_STRING이나 JSMN_PRIMITIVE가 아니면 스킵
		if (t[i].type != 3 && t[i].type != 4)
			continue;

		// mycategory_t 판별
		if (jsoneq(json, &t[i], "Menu") == 0) {
			current_cat = 0;
			continue;
		}
		else if (jsoneq(json, &t[i], "Meal") == 0) {
			current_cat = 1;
			continue;
		}

		// previous token 판별

		// name 토큰을 읽었을 때는, mymenu_t 메모리 동적 할당도 해줌
		if (jsoneq(json, &t[i], "name") == 0) {
			prev_tok = 1;
			m[++menu_count] = (mymenu_t *)malloc(sizeof(mymenu_t));
			continue;
		}
		else if (jsoneq(json, &t[i], "size") == 0) {
			prev_tok = 2;
			continue;
		}
		else if (jsoneq(json, &t[i], "price") == 0) {
			prev_tok = 3;
			continue;
		}

		// 토큰이 구조체에 저장해야 할 내용이고 String 타입이면, 토큰 내용만 뽑아 'subString'에 따로 저장함
		// We expect the contents of one token is less then 30 chars
		
		// Initialize subString
		for(j = 0; j < 30; j ++)
			subString[j] = '\0';
			
		for(j = t[i].start; j < t[i].end; j ++) 
			subString[j - t[i].start] = json[j];

		subString[j] = '\0';

		// 구조체에 정보 저장

		// name 저장 시, 카테고리도 같이 저장해 줌
		if(prev_tok == 1) {
			strcpy(m[menu_count]->name, subString);
			m[menu_count]->cat = current_cat;
		}
		else if(prev_tok == 2) {
			strcpy(m[menu_count]->size, subString);
		}
		else {
			current_price = atoi(subString);
			m[menu_count]->price = current_price;
		}
	}

	return ++menu_count;
}

void printmenu(mymenu_t *m[], int mcount)
{
	int i;
	char category[2][10] = {"C_SET", "C_MEAL"};

	printf("***** Print Menu *****\n\n");
	for(i = 0; i < mcount; i ++) {
		m[i]->name[29] = '\0';
		printf("[%d. %s] Category: %s, Size: %s, Price: %d\n", i+1, m[i]->name, category[m[i]->cat], m[i]->size, m[i]->price);
	}
}

void printincome(mymenu_t *m[], int mcount)
{
	int menu;
	int order;
	int total_income = 0;
	int menu_income[8] = {0};
	
	FILE *fp;
	fp = fopen("receipts.txt", "rt");

	// 첫 번째 menu / order 쌍은 버림
	fscanf(fp, " %d", menu);
	fscanf(fp, " %d", order);

	while(!feof(fp)) {
		fscanf(fp, " %d", menu);
		fscanf(fp, " %d", order);

		switch (menu) {
			case '1':
				total_income += (3950 * order);
				menu_income[0] = (3950 * order);
				break;
			case '2':
				total_income += (4950 * order);
				menu_income[1] = (4950 * order);
				break;
			case '3':
				total_income += (6950 * order);
				menu_income[2] = (6950 * order);
				break;
			case '4':
				total_income += (3500 * order);
				menu_income[3] = (3500 * order);
				break;
			case '5':
				total_income += (8950 * order);
				menu_income[4] = (8950 * order);
				break;
			case '6':
				total_income += (9950 * order);
				menu_income[5] = (9950 * order);
				break;
			case '7':
				total_income += (10950 * order);
				menu_income[6] = (10590 * order);
				break;
			case '8':
				total_income += (12950 * order);
				menu_income[7] = (12950 * order);
				break;
		}
	}
	
	printf("\n\n*** Print Income *****");
	printf("Total income: %d\n", total_income);
	printf("Income per menu:\n");
	printf("- 1. Vegetable Egg Rolls: %d\n", menu_income[0]);
	printf("- 2. Deep Fried Tofu: %d\n", menu_income[1]);
	printf("- 3. Popcorn Chicken: %d\n", menu_income[2]);
	printf("- 4. Seaweed Salad: %d\n", menu_income[3]);
	printf("- 5. Vegetable Fried Rice bowl: %d\n", menu_income[4]);
	printf("- 6. BBQ Pork Fried Rice: %d\n", menu_income[5]);
	printf("- 7. Seafood Crispy Noodle Bowl: %d\n", menu_income[6]);
	printf("- 8. Pineapple Shrimpt Fried Bowl: %d\n", menu_income[7]);
}

int main() {
	int i;
	int r;
	int menucount;
	jsmn_parser p;
	jsmntok_t t[128]; /* We expect no more than 128 tokens */
	mymenu_t * mymenu[20];
	
	char *JSON_STRING = readjsonfile("mymenu.json");

	printf("Initial JSON String:\n%s\n\n", JSON_STRING);

	jsmn_init(&p);
	r = jsmn_parse(&p, JSON_STRING, strlen(JSON_STRING), t, sizeof(t)/sizeof(t[0]));
	if (r < 0) {
		printf("Failed to parse JSON: %d\n", r);
		return 1;
	}
	menucount = makemymenu(JSON_STRING, t, r, mymenu);

	printmenu(mymenu, menucount);

	printincome(mymenu, menucount);

	return EXIT_SUCCESS;
}
