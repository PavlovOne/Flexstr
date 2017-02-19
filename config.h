#ifndef __CONFIG_H_
#define __CONFIG_H_

#define NET_PORT 8888
#define BACK_LOG 10

#define SETTINGS_MAXCOUNT 16
#define SETTINGS_UNITSIZE 16
#define SETTINGS_BUFSIZE (SETTINGS_MAXCOUNT*SETTINGS_UNITSIZE)

#define ENCODES_MAXCOUNT 2048
#define ENCODES_UNITSIZE 12
#define ENCODES_BUFSIZE (ENCODES_MAXCOUNT*ENCODES_UNITSIZE)

#define NET_BUFSIZE (SETTINGS_BUFSIZE + ENCODES_BUFSIZE*3)

#define STATUS_BUFSIZE 32

#define SOCK_ERROR	-1

#endif // __CONFIG_H_