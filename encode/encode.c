/**
 * Sample program that writes an EPC to a tag
 * @file writetag.c
 */

#include <tm_reader.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <inttypes.h>
#ifndef WIN32
#include <string.h>
#include <unistd.h>
#endif

#include "../config.h"
#include "../utils.h"
#include "encode.h"

#define numberof(x) (sizeof((x))/sizeof((x)[0]))

#define usage() {errx(1, "Please provide reader URL, such as:\n"\
                         "tmr:///com4 or tmr:///com4 --ant 1,2\n"\
                         "tmr://my-reader.example.com or tmr://my-reader.example.com --ant 1,2\n");}

extern ENCODE_SETTINGS gEncodeSettings;
extern char *gEncodesBuff;
extern size_t gEncodesCount;
extern size_t gEncodesPtr;

static TMR_Reader r, *rp;
static TMR_Region region;

static const char *regions[] = {"UNSPEC", "NA", "EU", "KR", "IN", "JP", "PRC", "EU2", "EU3", "KR2", "PRC2", "AU", "NZ", "REDUCED_FCC"};
static const char *powerModes[] = {"FULL", "MINSAVE", "MEDSAVE", "MAXSAVE", "SLEEP"};
static const char *userModes[] = {"NONE", "PRINTER", NULL, "PORTAL"};

static const char *tagEncodingNames[] = {"FM0", "M2", "M4", "M8"};
static const char *sessionNames[] = {"S0", "S1", "S2", "S3"};
static const char *targetNames[] = {"A", "B", "AB", "BA"};
static const char *gen2LinkFrequencyNames[] = {"250kHz", "300kHz", "320kHz", "40kHz", "640KHz"};
static const char *tariNames[] = {"25us", "12.5us", "6.25us"};

static const char *iso180006bLinkFrequencyNames[] = {"40kHz", "160kHz"};

static const char *iso180006bModulationDepthNames[] = {"99 percent", "11 percent"};

static const char *iso180006bDelimiterNames[] = {"", "Delimiter1", "", "", "Delimiter4"};
static const char *protocolNames[] = {NULL, NULL, NULL, "ISO180006B", NULL, "GEN2", "UCODE", "IPX64", "IPX256"};

static const char *bankNames[] = {"Reserved", "EPC", "TID", "User"};
static const char *selectOptionNames[] = {"EQ", "NE", "GT", "LT"};

static const char *tagopNames[] = {"Gen2.Read","Gen2.Write","Gen2.Lock", "Gen2.Kill"};

int init_encode(const char *deviceUri)
{
/*
	TMR_Status ret;

	rp = &r;
	ret = TMR_create(rp, deviceUri);
	if (TMR_SUCCESS != ret) {
		printf("Error creating reader: %s\n", TMR_strerr(rp, ret));
		return -1;
	}

	ret = TMR_connect(rp);
	if (TMR_SUCCESS != ret) {
		switch (ret) {
		case TMR_ERROR_BL_INVALID_IMAGE_CRC:
		case TMR_ERROR_BL_INVALID_APP_END_ADDR:
			printf("Error: App image corrupt.  Call firmware load\n");
			break;
		default:
			printf("Error connecting reader: %s\n", TMR_strerr(rp, ret));
			break;
		}
		return -1;
	}

	region = TMR_REGION_NONE;
	ret = TMR_paramGet(rp, TMR_PARAM_REGION_ID, &region);
	if (TMR_SUCCESS != ret)	{
		printf("Error getting region: %s\n", TMR_strerr(rp, ret));
		return -1;
	}

	if (TMR_REGION_NONE == region) {
		TMR_RegionList regions;
		TMR_Region _regionStore[32];
		regions.list = _regionStore;
		regions.max = sizeof(_regionStore)/sizeof(_regionStore[0]);
		regions.len = 0;

		ret = TMR_paramGet(rp, TMR_PARAM_REGION_SUPPORTEDREGIONS, &regions);
		if (TMR_SUCCESS != ret) {
			printf("Error getting supported region: %s\n", TMR_strerr(rp, __LINE__));
			return -1;
		}

		if (regions.len < 1) {
			printf("Reader doesn't supportany regions", TMR_strerr(rp, __LINE__));      
			return -1;
		}
		region = regions.list[0];
		ret = TMR_paramSet(rp, TMR_PARAM_REGION_ID, &region);
		if (TMR_SUCCESS != ret) {
			printf("Error setting region: %s\n", TMR_strerr(rp, ret));
			return -1;
		}
	}
*/
	// init settings
	gEncodeSettings.fastSearch = true;
	gEncodeSettings.ant1ReadPower = 16.0;
	gEncodeSettings.ant1WritePower = 16.0;
	gEncodeSettings.ant2ReadPower = 16.0;
	gEncodeSettings.ant2WritePower = 16.0;
	gEncodeSettings.linkFrequency = LINK640KHZ;
	gEncodeSettings.tari = TARI16_25US;
	gEncodeSettings.tagEncoding = FM0;
	gEncodeSettings.session = S0;
	gEncodeSettings.target = AB;
	gEncodeSettings.q = StaticQ2;
	gEncodeSettings.delayEncode = 1.0;
	gEncodeSettings.delayRead = 0.5;

	return 0;
}

void encode_print_status(void)
{
	size_t i, j;

	printf("----- Current Settings -----\r\n");
	printf("%-25s : %s\r\n", "Fast Search", gEncodeSettings.fastSearch ? "Yes" : "No");
	printf("%-25s : %4.2ff\r\n", "Antenna 1 read power", gEncodeSettings.ant1ReadPower);
	printf("%-25s : %4.2ff\r\n", "Antenna 1 write power", gEncodeSettings.ant1WritePower);
	printf("%-25s : %4.2ff\r\n", "Antenna 2 read power", gEncodeSettings.ant2ReadPower);
	printf("%-25s : %4.2ff\r\n", "Antenna 2 write power", gEncodeSettings.ant2WritePower);
	printf("%-25s : %s\r\n", "BLF", gen2LinkFrequencyNames[gEncodeSettings.linkFrequency]);
	printf("%-25s : %s\r\n", "Tari", tariNames[gEncodeSettings.tari]);
	printf("%-25s : %s\r\n", "Tag Encoding", tagEncodingNames[gEncodeSettings.tagEncoding]);
	printf("%-25s : %s\r\n", "Session", sessionNames[gEncodeSettings.session]);
	printf("%-25s : %d\r\n", "Q", gEncodeSettings.q);
	printf("%-25s : %4.2ff\r\n", "Delay Encode", gEncodeSettings.delayEncode);
	printf("%-25s : %4.2ff\r\n", "Delay Read", gEncodeSettings.delayRead);
	printf("\n");

	printf("----- Current Encodes -----\r\n");
	printf("encodes = %lu\n", gEncodesCount);
	for (i=0; i<gEncodesCount; i++){
		for (j=0; j<ENCODES_UNITSIZE; j++){
			char c = gEncodesBuff[i*ENCODES_UNITSIZE+j];
			printf("%02x", c&0xFF);
		}
		printf(", ");
	}
	printf("\n");
}

void release_encode(void)
{
	if (rp) TMR_destroy(rp);
	rp = NULL;
}

int get_gpi(int pinNum)
{
	TMR_Status ret;
	TMR_GpioPin state[16];
	uint8_t stateCount = numberof(state);

	if (!rp || pinNum<0 || pinNum>1) {
		printf("Invalid input params for get_gpi!\r\n");
		return -1;
	}

	ret = TMR_gpiGet(rp, &stateCount, state);
	if (TMR_SUCCESS != ret)	{
		printf("Error reading GPIO pins: %s\n", TMR_strerr(rp, ret));
		return -1;
	}
/*	
	printf("stateCount: %d\n", stateCount);
	for (i = 0 ; i < stateCount ; i++) {
		printf("Pin %d: %s\n", state[i].id, state[i].high ? "High" : "Low"); 
	}
*/
	if (pinNum < stateCount) {
		if (state[pinNum].high){
			return 1;
		}
	}
	
	return 0;
}

int read_tag(const uint8_t antNum)
{
	TMR_Status ret;
	TMR_ReadPlan plan;
	uint8_t antennaList[1];
	uint8_t antennaCount = 0x0;
	TMR_String model;
	char str[64];

	if (!rp || antNum<1 || antNum>2) {
		printf("Invalid input params for read_tag!\r\n");
		return -1;
	}

	antennaList[0] = antNum;
	antennaCount = 1;
	
	model.value = str;
	model.max = 64;
	TMR_paramGet(rp, TMR_PARAM_VERSION_MODEL, &model);
	
	/**
	* for antenna configuration we need two parameters
	* 1. antennaCount : specifies the no of antennas should
	*    be included in the read plan, out of the provided antenna list.
	* 2. antennaList  : specifies  a list of antennas for the read plan.
	**/ 
	
	// initialize the read plan 
	ret = TMR_RP_init_simple(&plan, antennaCount, antennaList, TMR_TAG_PROTOCOL_GEN2, 1000);
	if (TMR_SUCCESS != ret)
	{
		printf("Error %s: %s\n", "initializing the  read plan", TMR_strerr(rp, ret));
		return -1;
	}
	
	/* Commit read plan */
	ret = TMR_paramSet(rp, TMR_PARAM_READ_PLAN, &plan);
	if (TMR_SUCCESS != ret)
	{
		printf("Error %s: %s\n", "setting read plan", TMR_strerr(rp, ret));
		return -1;
	}
	
	ret = TMR_read(rp, 500, NULL);
	if (TMR_ERROR_TAG_ID_BUFFER_FULL == ret)
	{
		/* In case of TAG ID Buffer Full, extract the tags present
		* in buffer.
		*/
		fprintf(stdout, "reading tags:%s\n", TMR_strerr(rp, ret));
	}
	else
	{
		if (TMR_SUCCESS != ret)
		{
			printf("Error %s: %s\n", "reading tags", TMR_strerr(rp, ret));
			return -1;
		}
	}
	
	while (TMR_SUCCESS == TMR_hasMoreTags(rp))
	{
		TMR_TagReadData trd;
		char epcStr[128];

		ret = TMR_getNextTag(rp, &trd);
		if (TMR_SUCCESS != ret)
		{
			printf("Error %s: %s\n", "fetching tag", TMR_strerr(rp, ret));
			return -1;
		}

		TMR_bytesToHex(trd.tag.epc, trd.tag.epcByteCount, epcStr);
		
		printf("EPC:%s ant:%d count:%d\n", epcStr, trd.antenna, trd.readCount);
	}
	
	return 0;
}


int write_tag(const uint8_t antNum, const uint8_t *epcData, int epcLen)
{
	TMR_Status ret;
	
	if (!rp || antNum<1 || antNum>2 || !epcData || epcLen<=0) {
		printf("Invalid Antenna string!\r\n");
		return -1;
	}

	//Use first antenna for operation
	if (antNum<1 || antNum>2) {
		ret = TMR_paramSet(rp, TMR_PARAM_TAGOP_ANTENNA, &antNum);
		if (TMR_SUCCESS != ret)
		{
			printf("Error %s: %s\n", "setting tagop antenna", TMR_strerr(rp, ret));
			return -1;
		}
	}
	
	{
		TMR_TagData epc;
		TMR_TagOp tagop;

		/* Set the tag EPC to a known value*/
		epc.epcByteCount = epcLen;
		memcpy(epc.epc, epcData, epc.epcByteCount * sizeof(uint8_t));
		ret = TMR_TagOp_init_GEN2_WriteTag(&tagop, &epc);
		if (TMR_SUCCESS != ret)
		{
			printf("Error %s: %s\n", "initializing GEN2_WriteTag", TMR_strerr(rp, ret));
			return -1;
		}
		
		ret = TMR_executeTagOp(rp, &tagop, NULL, NULL);
		if (TMR_SUCCESS != ret)
		{
			printf("Error %s: %s\n", "executing GEN2_WriteTag", TMR_strerr(rp, ret));
			return -1;
		}
	}
	
	return 0;
}

static int statusFlag;
int encode_loop(void)
{
	int i;
	uint8_t epcData[ENCODES_UNITSIZE];

	if (gEncodesCount <= gEncodesPtr)
		return -1;
	
	if (get_gpi(0) > 0)
	{
		if (statusFlag == 0)
		{
			statusFlag = 1;
			read_tag(1);

			for (i=0; i<ENCODES_UNITSIZE; i++){
				epcData[i] = gEncodesBuff[gEncodesPtr*ENCODES_UNITSIZE+i];
				printf("%02x", epcData[i]&0xFF);
			}

			printf("---------GPIO Triggered, writing tag\r\n");
			i = write_tag(1, epcData, ENCODES_UNITSIZE);
			if (i == 0){
				gEncodesPtr++;
			}
			
			read_tag(1);
		}
	}
	else {
		statusFlag = 0;
	}

	return 0;
}

int main_(void)
{
	int i;
	char buff[32*ENCODES_UNITSIZE];
	gEncodesBuff = buff;
	gEncodesCount = 32;
	gEncodesPtr = 0;

	for (i=0; i<(32*ENCODES_UNITSIZE); i++)
	{
		gEncodesBuff[i] = i&0xFF;
	}
	
	init_encode("tmr://localhost");
	while (1)
	{
		encode_loop();
		sleepcp(2000);
	}
	
	release_encode();
}