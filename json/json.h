#ifndef __JSON_H_
#define __JSON_H_

#include "jsmn.h"

int json_parse(const char *jsonStr, ENCODE_SETTINGS *encSettings, char *encodesBuff, size_t *encodesCount);

#endif // __JSON_H_