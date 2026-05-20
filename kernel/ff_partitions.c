#include "ff.h"

/* Mandatory Partition Table for FatFs FF_MULTI_PARTITION */
/* Volume - Physical location resolution table */
PARTITION VolToPart[FF_VOLUMES] = {
    {0, 1}, /* "0:" -> Physical drive 0, 1st partition */
    {0, 2}, /* "1:" -> Physical drive 0, 2nd partition */
    {1, 0}, /* "2:" -> Physical drive 1, auto-detect */
    {2, 0}, /* "3:" -> Physical drive 2, auto-detect */
    {3, 0}, /* "4:" -> Physical drive 3, auto-detect */
    {4, 0}, /* "5:" -> Physical drive 4, auto-detect */
    {5, 0}, /* "6:" -> Physical drive 5, auto-detect */
    {6, 0}, /* "7:" -> Physical drive 6, auto-detect */
    {7, 0}, /* "8:" -> Physical drive 7, auto-detect */
    {8, 0}  /* "9:" -> Physical drive 8, auto-detect */
};
