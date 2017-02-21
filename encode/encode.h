#ifndef __ENCODE_H_
#define __ENCODE_H_

typedef enum { LINK250KHZ, LINK300KHZ, LINK320KHZ, LINK40KHZ, LINK640KHZ } LINK_FREQUENCY;
typedef enum { TARI25US, TARI12_5US, TARI16_25US } TARI;
typedef enum { FM0, M2, M4, M8 } TAG_ENCODING;
typedef enum { S0, S1, S2, S3 } SESSION;
typedef enum { A, B, AB, BA } TARGET;
typedef enum { 
	DynamicQ = -1, 
	StaticQ0,
	StaticQ1,
	StaticQ2,
	StaticQ3,
	StaticQ4,
	StaticQ5,
	StaticQ6,
	StaticQ7,
	StaticQ8,
	StaticQ9,
	StaticQ10,
	StaticQ11,
	StaticQ12,
	StaticQ13,
	StaticQ14,
	StaticQ15
} Q;

typedef struct _ENCODE_SETTINGS
{
	bool				fastSearch;
	float				ant1ReadPower;
	float				ant1WritePower;
	float				ant2ReadPower;
	float				ant2WritePower;
	LINK_FREQUENCY		linkFrequency;
	TARI				tari;
	TAG_ENCODING		tagEncoding;
	SESSION				session;
	TARGET				target;
	Q					q;
	float				delayEncode;
	float				delayRead;
} ENCODE_SETTINGS;


int init_encode(const char *deviceUri);
void release_encode(void);
int encode_loop(void);//uint8_t *epcData, int epcLength)
void encode_print_status(void);

#endif // __ENCODE_H_