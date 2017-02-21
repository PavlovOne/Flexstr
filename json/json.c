#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "jsmn.h"

#include "../config.h"
#include "../encode/encode.h"

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

int json_parse(const char *jsonStr, ENCODE_SETTINGS *encSettings, char *encodesBuff, size_t *encodesCount) 
{
	int i, j, k;
	int r;
	jsmn_parser p;
	jsmntok_t t[ENCODES_MAXCOUNT*2]; // We expect no more than 1024 tokens

	if (!jsonStr || !encSettings || !encodesBuff) {
		printf("Invalid input parameters for parse_json\r\n");
		return -1;
	}

	jsmn_init(&p);
	r = jsmn_parse(&p, jsonStr, strlen(jsonStr), t, sizeof(t)/sizeof(t[0]));
	if (r < 0) {
//		printf("Failed to parse JSON: %d\n", r);
		return r;
	}

	// Assume the top-level element is an object
	if (r < 1 || t[0].type != JSMN_OBJECT) {
//		printf("Object expected\n");
		return r;
	}

	// Loop over all keys of the root object
	*encodesCount = 0;
	for (i = 1; i < r; i++) {
		if (jsoneq(jsonStr, &t[i], "settings") == 0) {
			if (t[i+1].type != JSMN_OBJECT) {
				continue; // We expect groups to be an object
			}
			k = 0;
			for (j = 0; j < t[i+1].size; j++) {
				jsmntok_t *name = &t[i+j+k+2];
				jsmntok_t *val = &t[i+j+k+3];
				char sr[33];
				size_t len = val->end - val->start;
				if (len > 32) 
					len = 32;
				memcpy(sr, jsonStr + val->start, len);
				sr[len] = 0;
				if (jsoneq(jsonStr, name, "Fast Search") == 0) {
					if (!strcmp(sr, "Yes")) {
						encSettings->fastSearch = true;
					}
					else if (!strcmp(sr, "No")) {
						encSettings->fastSearch = false;
					}
					k++;
				}
				else if (jsoneq(jsonStr, name, "Antenna 1 read power") == 0) {
					encSettings->ant1ReadPower = (float)atof(sr);
					k++;
				}
				else if (jsoneq(jsonStr, name, "Antenna 1 write power") == 0) {
					encSettings->ant1WritePower = (float)atof(sr);
					k++;
				}
				else if (jsoneq(jsonStr, name, "Antenna 2 read power") == 0) {
					encSettings->ant2ReadPower = (float)atof(sr);
					k++;
				}
				else if (jsoneq(jsonStr, name, "Antenna 2 write power") == 0) {
					encSettings->ant2WritePower = (float)atof(sr);
					k++;
				}
				else if (jsoneq(jsonStr, name, "BLF") == 0) {
					if (!strcmp(sr, "LINK250KHZ")) {
						encSettings->linkFrequency = LINK250KHZ;
					}
					else if (!strcmp(sr, "LINK300KHZ")) {
						encSettings->linkFrequency = LINK300KHZ;
					}
					else if (!strcmp(sr, "LINK320KHZ")) {
						encSettings->linkFrequency = LINK320KHZ;
					}
					else if (!strcmp(sr, "LINK40KHZ")) {
						encSettings->linkFrequency = LINK40KHZ;
					}
					else if (!strcmp(sr, "LINK640KHZ")) {
						encSettings->linkFrequency = LINK640KHZ;
					}
					k++;
				}
				else if (jsoneq(jsonStr, name, "Tari") == 0) {
					if (!strcmp(sr, "TARI25US")) {
						encSettings->tari = TARI25US;
					}
					else if (!strcmp(sr, "TARI12_5US")) {
						encSettings->tari = TARI12_5US;
					}
					else if (!strcmp(sr, "TARI16_25US")) {
						encSettings->tari = TARI16_25US;
					}
					k++;
				}
				else if (jsoneq(jsonStr, name, "Tag Encoding") == 0) {
					if (!strcmp(sr, "FM0")) {
						encSettings->tagEncoding = FM0;
					}
					else if (!strcmp(sr, "M2")) {
						encSettings->tagEncoding = M2;
					}
					else if (!strcmp(sr, "M4")) {
						encSettings->tagEncoding = M4;
					}
					else if (!strcmp(sr, "M8")) {
						encSettings->tagEncoding = M8;
					}
					k++;
				}
				else if (jsoneq(jsonStr, name, "Session") == 0) {
					if (!strcmp(sr, "S0")) {
						encSettings->session = S0;
					}
					else if (!strcmp(sr, "S1")) {
						encSettings->session = S1;
					}
					else if (!strcmp(sr, "S2")) {
						encSettings->session = S2;
					}
					else if (!strcmp(sr, "S3")) {
						encSettings->session = S3;
					}
					k++;
				}
				else if (jsoneq(jsonStr, name, "Target") == 0) {
					if (!strcmp(sr, "A")) {
						encSettings->target = A;
					}
					else if (!strcmp(sr, "B")) {
						encSettings->target = B;
					}
					else if (!strcmp(sr, "AB")) {
						encSettings->target = AB;
					}
					else if (!strcmp(sr, "BA")) {
						encSettings->target = BA;
					}
					k++;
				}
				else if (jsoneq(jsonStr, name, "Q") == 0) {
					if (!strcmp(sr, "DynamicQ")) {
						encSettings->q = DynamicQ;
					}
					else if (!strcmp(sr, "StaticQ0")) {
						encSettings->q = StaticQ0;
					}
					else if (!strcmp(sr, "StaticQ1")) {
						encSettings->q = StaticQ1;
					}
					else if (!strcmp(sr, "StaticQ2")) {
						encSettings->q = StaticQ2;
					}
					else if (!strcmp(sr, "StaticQ3")) {
						encSettings->q = StaticQ3;
					}
					else if (!strcmp(sr, "StaticQ4")) {
						encSettings->q = StaticQ4;
					}
					else if (!strcmp(sr, "StaticQ5")) {
						encSettings->q = StaticQ5;
					}
					else if (!strcmp(sr, "StaticQ6")) {
						encSettings->q = StaticQ6;
					}
					else if (!strcmp(sr, "StaticQ7")) {
						encSettings->q = StaticQ7;
					}
					else if (!strcmp(sr, "StaticQ8")) {
						encSettings->q = StaticQ8;
					}
					else if (!strcmp(sr, "StaticQ9")) {
						encSettings->q = StaticQ9;
					}
					else if (!strcmp(sr, "StaticQ10")) {
						encSettings->q = StaticQ10;
					}
					else if (!strcmp(sr, "StaticQ11")) {
						encSettings->q = StaticQ11;
					}
					else if (!strcmp(sr, "StaticQ12")) {
						encSettings->q = StaticQ12;
					}
					else if (!strcmp(sr, "StaticQ13")) {
						encSettings->q = StaticQ13;
					}
					else if (!strcmp(sr, "StaticQ14")) {
						encSettings->q = StaticQ14;
					}
					else if (!strcmp(sr, "StaticQ15")) {
						encSettings->q = StaticQ15;
					}
					k++;
				}
				else if (jsoneq(jsonStr, name, "Delay Encode") == 0) {
					encSettings->delayEncode = (float)atof(sr);
					k++;
				}
				else if (jsoneq(jsonStr, name, "Delay Read") == 0) {
					encSettings->delayRead = (float)atof(sr);
					k++;
				}
			}
			i += t[i+1].size + k + 1;
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
			printf("Unexpected key: %.*s\n", t[i].end-t[i].start, jsonStr + t[i].start);
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
