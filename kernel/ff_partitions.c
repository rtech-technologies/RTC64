#include "ff.h"
#include "diskio.h"

/* Mandatory Partition Table for FatFs FF_MULTI_PARTITION */
PARTITION VolToPart[FF_VOLUMES] = {
    {0, 1}, /* Volume 0: Physical drive 0, Partition 1 */
    {1, 1}, /* Volume 1: Physical drive 1, Partition 1 */
    {2, 1},
    {3, 1},
    {4, 1},
    {5, 1},
    {6, 1},
    {7, 1},
    {8, 1},
    {9, 1}
};
