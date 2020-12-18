
/*********************************************************************************************

    This is public domain software that was developed by or for the U.S. Naval Oceanographic
    Office and/or the U.S. Army Corps of Engineers.

    This is a work of the U.S. Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the U.S. Government.

    Neither the United States Government, nor any employees of the United States Government,
    nor the author, makes any warranty, express or implied, without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes any liability or
    responsibility for the accuracy, completeness, or usefulness of any information,
    apparatus, product, or process disclosed, or represents that its use would not infringe
    privately-owned rights. Reference herein to any specific commercial products, process,
    or service by trade name, trademark, manufacturer, or otherwise, does not necessarily
    constitute or imply its endorsement, recommendation, or favoring by the United States
    Government. The views and opinions of authors expressed herein do not necessarily state
    or reflect those of the United States Government, and shall not be used for advertising
    or product endorsement purposes.

*********************************************************************************************/


/*********************************************************************************************

    This program is public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This is a work of the US Government. In accordance with 17 USC 105,
    copyright protection is not available for any work of the US Government.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

*********************************************************************************************/

#ifndef VERSION

#define     VERSION     "PFM Software - pfm_beamstats V2.38 - 07/29/14"

#endif

/*

    Version 1.0
    Jan C. Depner
    01/06/00


    Version 2.0
    Jan C. Depner
    02/15/00

    Went to version 2.0 of the PFM library.


    Version 2.1
    Jan C. Depner
    03/29/00

    Uses version 2.1 of the pfm library.


    Version 2.2
    Jan C. Depner
    09/04/00

    Replaced call to read_depth_record_index with read_depth_array_index.


    Version 2.21
    Jan C. Depner
    02/07/01

    Kicked the max number of beams up to 1024 to support em120 (191 beams).


    Version 2.22
    Jan C. Depner
    02/21/01

    Passing scale to open_pfm_file as a pointer.


    Version 2.23
    Jan C. Depner
    06/20/01

    Passing structure args to open_pfm_file.


    Version 2.24
    Jan C. Depner
    07/19/01
 
    4.0 PFM library changes.


    Version 2.25
    Jan C. Depner
    10/03/01
 
    Disregard deleted files.  Precede all comments with # for gnuplot.


    Version 2.26
    Jan C. Depner
    12/04/03
 
    Added computation for square kilometers and square nautical miles.


    Version 2.27
    Jan C. Depner
    10/15/04
 
    Added computation for square kilometers and square nautical miles at
    200% or better.


    Version 2.28
    Jan C. Depner
    02/25/05

    Switched to open_existing_pfm_file from open_pfm_file.


    Version 2.29
    Jan C. Depner
    03/04/05

    Fix return from open_existing_pfm_file.


    Version 2.30
    Jan C. Depner
    10/26/05

    Changed usage for PFM 4.6 handle file use.


    Version 2.31
    Jan C. Depner
    10/22/07

    Added fflush calls after prints to stderr since flush is not automatic in Windows.


    Version 2.32
    Stacy Johnson
    02/25/08

    Stacy Johnson's changes for Sue Sebastian.


    Version 2.33
    Stacy Johnson
    03/17/08

    More changes for Sue by Stacy.


    Version 2.34
    Jan C. Depner
    04/07/08

    Replaced single .h files from utility library with include of nvutility.h


    Version 2.35
    Jan C. Depner
    01/29/09

    Set checkpoint to 0 prior to calling open_existing_pfm_file.


    Version 2.36
    Jan C. Depner (PFM Software)
    02/26/14

    Cleaned up "Set but not used" variables that show up using the 4.8.2 version of gcc.


    Version 2.37
    Jan C. Depner (PFM Software)
    07/23/14

    - Switched from using the old NV_INT64 and NV_U_INT32 type definitions to the C99 standard stdint.h and
      inttypes.h sized data types (e.g. int64_t and uint32_t).


    Version 2.38
    Jan C. Depner (PFM Software)
    07/29/14

    - Fixed errors discovered by cppcheck.

*/
