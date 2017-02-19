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

#define numberof(x) (sizeof((x))/sizeof((x)[0]))

#define usage() {errx(1, "Please provide reader URL, such as:\n"\
                         "tmr:///com4 or tmr:///com4 --ant 1,2\n"\
                         "tmr://my-reader.example.com or tmr://my-reader.example.com --ant 1,2\n");}

extern char *gEncodesBuff;
extern size_t gEncodesCount;
extern size_t gEncodesPtr;

void errx(int exitval, const char *fmt, ...)
{
	va_list ap;
	
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	
//	exit(exitval);
}

void checkerr(TMR_Reader* rp, TMR_Status ret, int exitval, const char *msg)
{
	if (TMR_SUCCESS != ret)
	{
		errx(exitval, "Error %s: %s\n", msg, TMR_strerr(rp, ret));
	}
}

int get_gpi(const char *deviceUri, int pinNum)
{
	TMR_Reader r, *rp;
	TMR_Status ret;
	TMR_Region region;
	TMR_GpioPin state[16];
	uint8_t i, stateCount = numberof(state);

	rp = &r;
	ret = TMR_create(rp, deviceUri);
	if (TMR_SUCCESS != ret) {
		errx(1, "Error creating reader: %s\n", TMR_strerr(rp, ret));
	}

	ret = TMR_connect(rp);
	if (TMR_SUCCESS != ret) {
		switch (ret) {
		case TMR_ERROR_BL_INVALID_IMAGE_CRC:
		case TMR_ERROR_BL_INVALID_APP_END_ADDR:
			fprintf(stderr, "Error: App image corrupt.  Call firmware load\n");
			break;
		default:
			errx(1, "Error connecting reader: %s\n", TMR_strerr(rp, ret));
			break;
		}
		return -1;
	}

	region = TMR_REGION_NONE;
	ret = TMR_paramGet(rp, TMR_PARAM_REGION_ID, &region);
	if (TMR_SUCCESS != ret)	{
		errx(1, "Error getting region: %s\n", TMR_strerr(rp, ret));
	}

	if (TMR_REGION_NONE == region) {
		TMR_RegionList regions;
		TMR_Region _regionStore[32];
		regions.list = _regionStore;
		regions.max = sizeof(_regionStore)/sizeof(_regionStore[0]);
		regions.len = 0;

		ret = TMR_paramGet(rp, TMR_PARAM_REGION_SUPPORTEDREGIONS, &regions);
		if (TMR_SUCCESS != ret) {
			errx(1, "Error getting supported region: %s\n", TMR_strerr(rp, __LINE__));
		}

		if (regions.len < 1) {
			errx(1, "Reader doesn't supportany regions", TMR_strerr(rp, __LINE__));      
		}
		region = regions.list[0];
		ret = TMR_paramSet(rp, TMR_PARAM_REGION_ID, &region);
		if (TMR_SUCCESS != ret) {
			errx(1, "Error setting region: %s\n", TMR_strerr(rp, ret));
		}
	}

	ret = TMR_gpiGet(rp, &stateCount, state);
	if (TMR_SUCCESS != ret)	{
		printf("Error reading GPIO pins: %s\n", TMR_strerr(rp, ret));
		return -1;
	}
	
	printf("stateCount: %d\n", stateCount);
	for (i = 0 ; i < stateCount ; i++) {
		printf("Pin %d: %s\n", state[i].id, state[i].high ? "High" : "Low"); 
	}

	TMR_destroy(rp);

	if (pinNum < stateCount) {
		if (state[pinNum].high){
			return 1;
		}
	}
	
	return 0;
}

int read_tag(const char *deviceUri, const uint8_t antNum)
{
	TMR_Reader r, *rp;
	TMR_Status ret;
	TMR_Region region;
	TMR_ReadPlan plan;
	uint8_t antennaList[1];
	uint8_t antennaCount = 0x0;
	TMR_String model;
	char str[64];

	if (!deviceUri || antNum<1 || antNum>2) {
		printf("Invalid input params for read_tag!\r\n");
		return -1;
	}

	antennaList[0] = antNum;
	antennaCount = 1;
	
	rp = &r;
	ret = TMR_create(rp, deviceUri);
	checkerr(rp, ret, 1, "creating reader");
	
	ret = TMR_connect(rp);
	checkerr(rp, ret, 1, "connecting reader");
	
	region = TMR_REGION_NONE;
	ret = TMR_paramGet(rp, TMR_PARAM_REGION_ID, &region);
	checkerr(rp, ret, 1, "getting region");
	
	if (TMR_REGION_NONE == region)
	{
		TMR_RegionList regions;
		TMR_Region _regionStore[32];
		regions.list = _regionStore;
		regions.max = sizeof(_regionStore)/sizeof(_regionStore[0]);
		regions.len = 0;
		
		ret = TMR_paramGet(rp, TMR_PARAM_REGION_SUPPORTEDREGIONS, &regions);
		checkerr(rp, ret, 1, "getting supported regions");
		
		if (regions.len < 1) {
			checkerr(rp, TMR_ERROR_INVALID_REGION, __LINE__, "Reader doesn't supportany regions");
		}
		region = regions.list[0];
		ret = TMR_paramSet(rp, TMR_PARAM_REGION_ID, &region);
		checkerr(rp, ret, 1, "setting region");  
	}
	
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
	checkerr(rp, ret, 1, "initializing the  read plan");
	
	/* Commit read plan */
	ret = TMR_paramSet(rp, TMR_PARAM_READ_PLAN, &plan);
	checkerr(rp, ret, 1, "setting read plan");
	
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
		checkerr(rp, ret, 1, "reading tags");
	}
	
	while (TMR_SUCCESS == TMR_hasMoreTags(rp))
	{
		TMR_TagReadData trd;
		char epcStr[128];

		ret = TMR_getNextTag(rp, &trd);
		checkerr(rp, ret, 1, "fetching tag");
		
		TMR_bytesToHex(trd.tag.epc, trd.tag.epcByteCount, epcStr);
		
		printf("EPC:%s ant:%d count:%d\n", epcStr, trd.antenna, trd.readCount);
	}
	
	TMR_destroy(rp);
	return 0;
}


int write_tag(const char *deviceUri, const uint8_t antNum, const uint8_t *epcData, int epcLen)
{
	TMR_Reader r, *rp;
	TMR_Status ret;
	TMR_Region region;
	TMR_String model;
	char str[64];
#if USE_TRANSPORT_LISTENER
	TMR_TransportListenerBlock tb;
#endif
	
	if (!deviceUri || antNum<1 || antNum>2 || !epcData || epcLen<=0) {
		printf("Invalid Antenna string!\r\n");
		return -1;
	}
	
	rp = &r;
	ret = TMR_create(rp, deviceUri);
	checkerr(rp, ret, 1, "creating reader");

	ret = TMR_connect(rp);
	checkerr(rp, ret, 1, "connecting reader");

	region = TMR_REGION_NONE;
	ret = TMR_paramGet(rp, TMR_PARAM_REGION_ID, &region);
	checkerr(rp, ret, 1, "getting region");

	if (TMR_REGION_NONE == region) {
		TMR_RegionList regions;
		TMR_Region _regionStore[32];
		regions.list = _regionStore;
		regions.max = sizeof(_regionStore)/sizeof(_regionStore[0]);
		regions.len = 0;

		ret = TMR_paramGet(rp, TMR_PARAM_REGION_SUPPORTEDREGIONS, &regions);
		checkerr(rp, ret, __LINE__, "getting supported regions");
		
		if (regions.len < 1) {
			checkerr(rp, TMR_ERROR_INVALID_REGION, __LINE__, "Reader doesn't supportany regions");
		}
		region = regions.list[0];
		ret = TMR_paramSet(rp, TMR_PARAM_REGION_ID, &region);
		checkerr(rp, ret, 1, "setting region");  
	}
	
	model.value = str;
	model.max = 64;
	TMR_paramGet(rp, TMR_PARAM_VERSION_MODEL, &model);
	if (((0 == strcmp("Sargas", model.value)) || (0 == strcmp("M6e Micro", model.value)) ||(0 == strcmp("M6e Nano", model.value)))
		&& (antNum<1 || antNum>2)) {
		fprintf(stdout, "Module doesn't has antenna detection support please provide antenna list\n");
		usage();
	}
	//Use first antenna for operation
	if (antNum<1 || antNum>2) {
		ret = TMR_paramSet(rp, TMR_PARAM_TAGOP_ANTENNA, &antNum);
		checkerr(rp, ret, 1, "setting tagop antenna");  
	}
	
	{
		TMR_TagData epc;
		TMR_TagOp tagop;

		/* Set the tag EPC to a known value*/
		epc.epcByteCount = epcLen;
		memcpy(epc.epc, epcData, epc.epcByteCount * sizeof(uint8_t));
		ret = TMR_TagOp_init_GEN2_WriteTag(&tagop, &epc);
		checkerr(rp, ret, 1, "initializing GEN2_WriteTag");
		
		ret = TMR_executeTagOp(rp, &tagop, NULL, NULL);
		checkerr(rp, ret, 1, "executing GEN2_WriteTag");
	}
	
	TMR_destroy(rp);
	return 0;
}

int encode_loop(void)
{
	int i;
	static int statusFlag;
	uint8_t epcData[ENCODES_UNITSIZE];

	if (gEncodesCount < gEncodesPtr)
		return -1;
	
	for (i=0; i<ENCODES_UNITSIZE; i++){
		epcData[i] = gEncodesBuff[gEncodesPtr*ENCODES_UNITSIZE+i];
		printf("%02x", epcData[i]&0xFF);
	}
	gEncodesPtr++;
	printf("\n");
/*
	statusFlag = 0;
	if (get_gpi("tmr://localhost", 0) > 0)
	{
		if (statusFlag == 0)
		{
			statusFlag = 1;
			read_tag("tmr://localhost", 1);
			sleepcp(100);

			printf("---------GPIO Triggered, writing tag\r\n");
			i = write_tag("tmr://localhost", 1, epcData, ENCODES_UNITSIZE);
			if (i == 0){
				gEncodesPtr++;
			}
			
			sleepcp(100);
			read_tag("tmr://localhost", 1);
		}
	}
	else {
		statusFlag = 0;
	}
	sleepcp(100);
*/	
	return 0;
}