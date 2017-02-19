#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jsmn.h"

#include "../config.h"

/*
 * A small example of jsmn parsing when JSON structure is known and number of
 * tokens is predictable.
 */

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
	if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
			strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
		return 0;
	}
	return -1;
}

int json_parse(const char *jsonStr, char *settingsBuff, size_t *settingsCount, char *encodesBuff, size_t *encodesCount) 
{
	int i, j, k;
	int r;
	jsmn_parser p;
	jsmntok_t t[ENCODES_MAXCOUNT]; // We expect no more than 1024 tokens

	if (!jsonStr || !settingsBuff || !encodesBuff) {
		printf("Invalid input parameters for parse_json\r\n");
		return -1;
	}

	jsmn_init(&p);
	r = jsmn_parse(&p, jsonStr, strlen(jsonStr), t, sizeof(t)/sizeof(t[0]));
	if (r < 0) {
		printf("Failed to parse JSON: %d\n", r);
		return r;
	}

	// Assume the top-level element is an object
	if (r < 1 || t[0].type != JSMN_OBJECT) {
		printf("Object expected\n");
		return r;
	}

	// Loop over all keys of the root object
	*settingsCount = 0;
	*encodesCount = 0;
	for (i = 1; i < r; i++) {
		if (jsoneq(jsonStr, &t[i], "settings") == 0) {
			if (t[i+1].type != JSMN_ARRAY) {
				continue; // We expect groups to be an array of strings
			}
			if (t[i+1].size > SETTINGS_MAXCOUNT) {
				continue; // exceed max count
			}
			
			for (j = 0; j < t[i+1].size; j++) {
				jsmntok_t *g = &t[i+j+2];
				char *setting = &settingsBuff[SETTINGS_UNITSIZE*j];
				size_t len = (g->end-g->start) > SETTINGS_UNITSIZE ? SETTINGS_UNITSIZE : (g->end-g->start);
				memcpy(setting, jsonStr+g->start, len);
			}
			*settingsCount = t[i+1].size;
			i += t[i+1].size + 1;
		} else if (jsoneq(jsonStr, &t[i], "encodes") == 0) {
			if (t[i+1].type != JSMN_ARRAY) {
				continue; // We expect groups to be an array of strings
			}
			if (t[i+1].size > ENCODES_MAXCOUNT) {
				continue; // exceed max count
			}

			for (j = 0; j < t[i+1].size; j++) {
				jsmntok_t *g = &t[i+j+2];
				char *encode = &encodesBuff[ENCODES_UNITSIZE*j];
				size_t len = (g->end-g->start) > ENCODES_UNITSIZE*2 ? ENCODES_UNITSIZE*2 : (g->end-g->start);
				memset(encode, 0, ENCODES_UNITSIZE);

				len &= 0xFFFE; // make as ever
				for (k=0; k<(int)len; k++) {
					char c = jsonStr[g->start+k];
					char num = 0;
					if ((c>=48) && (c<=57)) { // 0~9
						num = c - 48;
					}
					else if ((c>=65) && (c<=70)) { // A~F
						num = c - 65 + 0xA;
					}
					else if ((c>=97) && (c<=102)) { // a~f
						num = c - 97 + 0xa;
					}
					if (k & 1) {
						encode[k/2] <<= 4;
					}
					encode[k/2] &= 0xF0;
					encode[k/2] |= (num & 0xF);
				}
			}
			*encodesCount = t[i+1].size;
			i += t[i+1].size + 1;
		} else {
			printf("Unexpected key: %.*s\n", t[i].end-t[i].start,
				jsonStr + t[i].start);
		}
	}
	return EXIT_SUCCESS;
}

// int main() {
// 	int i;
// 	int r;
// 	jsmn_parser p;
// 	jsmntok_t t[128]; /* We expect no more than 128 tokens */
// 
// 	jsmn_init(&p);
// 	r = jsmn_parse(&p, JSON_STRING, strlen(JSON_STRING), t, sizeof(t)/sizeof(t[0]));
// 	if (r < 0) {
// 		printf("Failed to parse JSON: %d\n", r);
// 		return 1;
// 	}
// 
// 	/* Assume the top-level element is an object */
// 	if (r < 1 || t[0].type != JSMN_OBJECT) {
// 		printf("Object expected\n");
// 		return 1;
// 	}
// 
// 	/* Loop over all keys of the root object */
// 	for (i = 1; i < r; i++) {
// 		if (jsoneq(JSON_STRING, &t[i], "user") == 0) {
// 			/* We may use strndup() to fetch string value */
// 			printf("- User: %.*s\n", t[i+1].end-t[i+1].start,
// 					JSON_STRING + t[i+1].start);
// 			i++;
// 		} else if (jsoneq(JSON_STRING, &t[i], "admin") == 0) {
// 			/* We may additionally check if the value is either "true" or "false" */
// 			printf("- Admin: %.*s\n", t[i+1].end-t[i+1].start,
// 					JSON_STRING + t[i+1].start);
// 			i++;
// 		} else if (jsoneq(JSON_STRING, &t[i], "uid") == 0) {
// 			/* We may want to do strtol() here to get numeric value */
// 			printf("- UID: %.*s\n", t[i+1].end-t[i+1].start,
// 					JSON_STRING + t[i+1].start);
// 			i++;
// 		} else if (jsoneq(JSON_STRING, &t[i], "groups") == 0) {
// 			int j;
// 			printf("- Groups:\n");
// 			if (t[i+1].type != JSMN_ARRAY) {
// 				continue; /* We expect groups to be an array of strings */
// 			}
// 			for (j = 0; j < t[i+1].size; j++) {
// 				jsmntok_t *g = &t[i+j+2];
// 				printf("  * %.*s\n", g->end - g->start, JSON_STRING + g->start);
// 			}
// 			i += t[i+1].size + 1;
// 		} else {
// 			printf("Unexpected key: %.*s\n", t[i].end-t[i].start,
// 					JSON_STRING + t[i].start);
// 		}
// 	}
// 	return EXIT_SUCCESS;
// }
