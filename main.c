
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

/***************************************************************************\
*                                                                           *
*   Module Name:        pfm_beamstats                                       *
*                                                                           *
*   Programmer(s):      Jan Depner                                          *
*                                                                           *
*   Date Written:       01/06/00                                            *
*                                                                           *
*   Purpose:            Calculates statistics on an edited PFM file.        *
*                       Invalid beams are excluded from all totals.         *
*                                                                           *
\***************************************************************************/
  

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>

#include "nvutility.h"

#include "pfm.h"

#include "gsf.h"

#include "version.h"


#define  MAX_BEAMS  1024


typedef struct
{ 
    int32_t bad[MAX_BEAMS];          /* number of edits per beam            */
    int32_t good[MAX_BEAMS];         /* number of good data points per beam */
    int32_t manual[MAX_BEAMS];       /* number of manual edits              */
    int32_t filter[MAX_BEAMS];       /* number of filter edits              */
    int32_t select[MAX_BEAMS];       /* number of selected soundings        */
    int32_t pfm[MAX_BEAMS];          /* number of manual edits              */
    int32_t total_depths[MAX_BEAMS]; /* total number of depths per beam     */
} BeamCount;


int32_t main (int32_t argc, char **argv)
{
    BeamCount  beam_count;       /* beam count record for statistical counts */
    int32_t    i, j,             /* general-purpose counters                 */
               percent = 0,      /* percent processed counter                */
               old_percent = -1, /* previous percent processed               */
               grand_total,      /* total number of depths for all beams     */
               total_filter = 0, /* total filter edited                      */
               total_manual = 0, /* total manual edited                      */
               total_pfm = 0,    /* total PFM bit set                        */
               total_bad = 0,    /* total number of edits for all beams      */
               total_good = 0,   /* total number of good depths for all beams*/
               total_select = 0; /* total number of selected soundings       */
    float bad_percent,      /* % of total depths that have been edited  */
               good_percent,     /* % of total depths that are good          */
               total_bad_percent = 0,
                                 /* % of total depths that have been edited  */
      total_good_percent = 0;
                                 /* % of total depths that are good         */

    PFM_OPEN_ARGS           open_args;
    NV_I32_COORD2           coord, prev_coord = {-1, -1};
    BIN_RECORD              bin_record;
    DEPTH_RECORD            *depth;
    int32_t                 pfm_handle, total_bins, recnum, bin_count, 
                            bin2_count;
    uint8_t                 bin_data;

    int32_t                 k, m, neg_count[MAX_BEAMS], pos_count[MAX_BEAMS],
                            min_beams = MAX_BEAMS + 1, max_beams = -1;
    float                   rms[MAX_BEAMS], max_val[MAX_BEAMS], 
                            min_val[MAX_BEAMS], neg_percent[MAX_BEAMS], 
                            pos_percent[MAX_BEAMS], depthtot[MAX_BEAMS], 
                            min_depth[MAX_BEAMS], max_depth[MAX_BEAMS], diff, 
                            dep, square_kilometers, square_nmiles, s2_kilos,
                            s2_nmiles, start_line_no;
    double                  sum[MAX_BEAMS], sum2[MAX_BEAMS],
                            meandiff[MAX_BEAMS], meandepth[MAX_BEAMS],
                            ss[MAX_BEAMS], var[MAX_BEAMS], stddev[MAX_BEAMS], 
                            sddepth[MAX_BEAMS];
    FILE                    *fp;


    fprintf (stderr, "\n\n %s \n\n", VERSION);
    fflush (stderr);


    if (argc < 2)
    {
        fprintf(stderr, "USAGE: pfm_beamstats <PFM_HANDLE_FILE or PFM_LIST_FILE> [output filespec]\n");
        exit (-1);
    }

    strcpy (open_args.list_path, argv[1]);


    if (argc == 3)
    {
        if ((fp = fopen (argv[2], "w")) == NULL)
        {
            perror (argv[2]);
            exit (-1);
        }
    }
    else
    {
        fp = stdout;
    }
   

    /* zero out beam count record */

    memset(&beam_count, 0, sizeof(beam_count));


    for (i = 0 ; i < MAX_BEAMS ; i++)
    {
        neg_count[i] = 0;
        pos_count[i] = 0;
        min_val[i] = 99999.0;
        max_val[i] = -99999.0;
        min_depth[i] = 99999.0;
        max_depth[i] = -99999.0;
        sum[i] = 0.0;
        sum2[i] = 0.0;
        depthtot[i] = 0.0;
    }


    /* Process the input file on the command line. */

    open_args.checkpoint = 0;
    if ((pfm_handle = open_existing_pfm_file (&open_args)) < 0) 
        pfm_error_exit (pfm_error);


    percent = 0;
    bin_count = 0;
    bin2_count = 0;

    fprintf(stderr,"\n\n");
    fflush (stderr);


    total_bins = open_args.head.bin_height * open_args.head.bin_width;


    /* Process all records in the PFM index file */

    for (i = 0 ; i < open_args.head.bin_height ; i++)
    {
        coord.y = i;
        
        for (j = 0 ; j < open_args.head.bin_width ; j++)
        {
            coord.x = j;

            bin_data = NVFalse;

            if (!read_depth_array_index (pfm_handle, coord, &depth, &recnum))
            {
                start_line_no = -1;
                for (m = 0 ; m < recnum ; m++)
                {
                    k = depth[m].beam_number;
                    dep = depth[m].xyz.z;

                    if (!(depth[m].validity & PFM_DELETED))
                    {
                        beam_count.total_depths[k]++;

                        if (depth[m].validity & PFM_MODIFIED)
                        {
                            beam_count.pfm[k]++;                 /* PFM bit */
                            total_pfm++;
                        }

                        if (depth[m].validity & PFM_MANUALLY_INVAL)
                        {
                            beam_count.manual[k]++;              /* Manual */
                            total_manual++;
                            beam_count.bad[k]++;                 /* edited */
                            total_bad++;
                        }
                        else if (depth[m].validity & PFM_FILTER_INVAL || 
                            depth[m].xyz.z >= open_args.head.null_depth)
                        {
                            beam_count.filter[k]++;              /* Filter */
                            total_filter++;
                            beam_count.bad[k]++;                 /* edited */
                            total_bad++;
                        }
                        else
                        {
                            /*  Looking for 200% or better coverage.  */

                            if (start_line_no == -1) 
                              {
                                start_line_no = depth[m].line_number;
                              }
                            else
                              {
                                if (start_line_no != -2)
                                  {
                                    if (depth[m].line_number != start_line_no) 
                                      {
                                        bin2_count++;
                                        start_line_no = -2;
                                      }
                                  }
                              }


                            beam_count.good[k]++;                /* good */
                            total_good++;

                            bin_data = NVTrue;


                            if (depth[m].validity & PFM_SELECTED_SOUNDING) 
                            {
                                beam_count.select[k]++;          /* Selected */
                                total_select++;
                            }


                            /*  Compute repeatability statistics.  */

                            if (prev_coord.x != coord.x || 
                                prev_coord.y != coord.y)
                            {
                                read_bin_record_index (pfm_handle, coord, 
                                    &bin_record);
                                prev_coord = coord;
                            }

                            if (bin_record.validity & PFM_DATA)
                            {
                                diff = bin_record.avg_filtered_depth - dep;

                                if (dep < min_depth[k]) min_depth[k] = dep;

                                if (dep > max_depth[k]) max_depth[k] = dep;

                                depthtot[k] += dep;
                                sum[k] += diff;
                                sum2[k] += diff * diff;

                                if (diff < 0.0)
                                {
                                    neg_count[k]++;
                                }
                                else
                                {
                                    pos_count[k]++;
                                }

                                if (fabs((double) diff) < min_val[k])
                                    min_val[k] = fabs((double) diff);

                                if (fabs((double) diff) > max_val[k])
                                  max_val[k] = fabs((double) diff);

                                if (k > max_beams) max_beams = k;
                                if (k < min_beams) min_beams = k;
                            }
                        }
                    }
                }
                free (depth);

                if (bin_data) bin_count++;
            }
        }

        percent = ((float) (i * open_args.head.bin_width + j) / 
            (float) total_bins) * 100.0;
        if (old_percent != percent)
        {
            fprintf (stderr, "%03d%% processed     \r", percent);
            old_percent = percent;
            fflush (stderr);
        }
    }

    percent = 100;

    fprintf(stderr,"%03d%% processed        \n\n", percent);
    fflush (stderr);



    /* calculate totals */

    grand_total = total_bad + total_good;
    square_kilometers = (bin_count * (open_args.head.bin_size_xy * 
        open_args.head.bin_size_xy)) / (1000.0 * 1000.0);
    square_nmiles = square_kilometers / (1.852 * 1.852);
    s2_kilos = (bin2_count * (open_args.head.bin_size_xy * 
        open_args.head.bin_size_xy)) / (1000.0 * 1000.0);
    s2_nmiles = s2_kilos / (1.852 * 1.852);


    /* print results */

    fprintf(fp, "#\n#Filename:  %s\n", open_args.list_path);
    fprintf(fp, "#\n#Square kilometers covered:  %f\n", square_kilometers);
    fprintf(fp, "#\n#Square nautical miles covered:  %f\n", square_nmiles);
    fprintf(fp, "#\n#Square kilometers covered at 200%% or better:  %f  (%.1f%%)\n", 
            s2_kilos, s2_kilos / square_kilometers * 100.0);
    fprintf(fp, "#\n#Square nautical miles covered at 200%% or better:  %f  (%.1f%%)\n", 
            s2_nmiles, s2_nmiles / square_nmiles * 100.0);
   
    fprintf(fp, "#\n#\n");
    fprintf(fp, "#Beam Stats\n");
    fprintf(fp, "#---------------------------\n\n");


    fprintf(fp, "#\t\t  %%BAD   %%GOOD    #BAD       #GOOD     #MANUAL    #FILTER      #PFM      #SELECTED\n");
    fprintf(fp, "#\t\t------  ------  ---------  ---------  ---------  ---------  ---------    ---------\n");
    for (i = 0 ; i < MAX_BEAMS ; i++)
    {
        if (beam_count.total_depths[i] > 0)
        {
            bad_percent = (float)beam_count.bad[i] / 
                (float)beam_count.total_depths[i] * 100.0;

            good_percent = (float)beam_count.good[i] / 
                (float)beam_count.total_depths[i] * 100.0;

            fprintf(fp, "Beam %3d\t%5.1f   %5.1f   %9d  %9d  %9d  %9d  %9d  %9d\n", 
                i + 1, bad_percent, good_percent, beam_count.bad[i], 
                beam_count.good[i], beam_count.manual[i], beam_count.filter[i],
                beam_count.pfm[i], beam_count.select[i]);
        }
    }


    if (grand_total > 0)
    {
        fprintf(fp, 
            "#------------------------------------------------------------------------------------------------\n");
        total_bad_percent = (float)total_bad / (float)grand_total * 100.0;
        total_good_percent = (float)total_good / (float)grand_total * 100.0;
	
    }

    fprintf(fp, "#All Beams\t%5.1f   %5.1f   %9d  %9d  %9d  %9d  %9d  %9d\n", 
        total_bad_percent, total_good_percent, total_bad, total_good,
        total_manual, total_filter, total_pfm, total_select); 
 

    fprintf(fp, "#\n#\n");
    fprintf(fp, "#Total number of non-dropped beams: %d\n", total_good);
    fprintf(fp, "#Total number of beams: %d\n", grand_total);
    fprintf(fp, "#Total number of edited beams: %d\n", (total_manual+total_filter));
    fprintf(fp, "#Total percent of edited beams: %5.2f\n", ((float)(total_manual+total_filter)/(float)total_good *100));
    fprintf(fp, "#\n#\n#\n#\n#\n");



    fprintf (fp, 
        "# BEAM #     RMS       MEAN DIFF          STD             STD%%    NEG%%   POS%%      MAX RESID    MEAN DEPTH    # POINTS\n#\n");

    for (i = min_beams ; i < max_beams ; i++)
    {
        meandiff[i] = sum[i] / (float) (neg_count[i] + pos_count[i]);
        meandepth[i] = depthtot[i] / (float) (neg_count[i] + pos_count[i]);
        ss[i] = sum2[i] - (sum[i] * meandiff[i]);
        var[i] = ss[i] / ((neg_count[i] + pos_count[i]) - 1);
        stddev[i] = sqrt (var[i]);
        sddepth[i] = (stddev[i] / meandepth[i]) * 100;
        rms[i] = sqrt((double) (sum2[i] / (float) (neg_count[i] + 
            pos_count[i])));
        neg_percent[i] = ((float) neg_count[i] / (float) (neg_count[i] + 
            pos_count[i])) * 100.0;
        pos_percent[i] = ((float) pos_count[i] / (float) (neg_count[i] + 
            pos_count[i])) * 100.0;

        if (sum[i] != 0.0)
        {
          fprintf(fp, 
                " %3d   %10.3f   %10.3f      %10.3f      %10.4f    %03d    %03d   %10.3f    %10.3f  %12d\n", 
                i + 1, rms[i], meandiff[i], stddev[i], sddepth[i], 
                NINT (neg_percent[i]), NINT (pos_percent[i]), 
                max_val[i], meandepth[i], (neg_count[i] + pos_count[i]));
        }
    }
    fprintf (fp, 
        "#\n#The above represents the reference average filtered bin value\n");
    fprintf (fp, 
        "#from the PFM file minus the real depth values from the PFM file.\n");
    fprintf (fp, 
        "#Negatives indicate the depth values are deeper than the averages.\n");

    fclose (fp);

    return (0);
}
