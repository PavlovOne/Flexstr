#ifndef __JSON_H_
#define __JSON_H_

int json_parse(const char *jsonStr, char *settingsBuff, size_t *settingsCount, char *encodesBuff, size_t *encodesCount);

#endif // __JSON_H_