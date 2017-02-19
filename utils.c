/**
 * Sample program that writes an EPC to a tag
 * @file writetag.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void sleepcp(int milliseconds)
{
#ifdef WIN32
        Sleep(milliseconds);
#else
        usleep(milliseconds * 1000);
#endif // win32
}