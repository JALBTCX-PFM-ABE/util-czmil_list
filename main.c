
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

 /********************************************************************
 *
 * Module Name : main.c
 *
 * Author/Date : Jan C. Depner
 *
 * Description : Lists CZMIL cpf and cwf files in ASCII format.
 *
 ********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <math.h>


/* Local Includes. */

#include "nvutility.h"

#include "czmil.h"

#include "version.h"


void usage ()
{
  fprintf (stderr, "\nUsage: czmil_list [-n RECORD NUMBER] [-t] CZMIL_CPF_OR_CWF_FILENAME\n");
  fprintf (stderr, "\nWhere:\n\n");
  fprintf (stderr, "\t-n  =  list RECORD NUMBER only.\n");
  fprintf (stderr, "\t-t  =  ten second read timing test.\n");
  fprintf (stderr, "\t\tThe t and n options are exclusive.\n\n");
  exit (-1);
}



int32_t main (int32_t argc, char **argv)
{
  char               file[512];
  int32_t            czmil_hnd = -1, rec_num = -1, i, start = 0, end = 0;
  CZMIL_CPF_Header   czmil_cpf_header;
  CZMIL_CPF_Data     czmil_cpf;
  CZMIL_CWF_Header   czmil_cwf_header;
  CZMIL_CWF_Data     czmil_cwf;
  uint8_t            wave = NVFalse, timing = NVFalse;
  char               c;
  extern char        *optarg;
  extern int         optind;


  fprintf (stderr, "\n\n %s \n\n\n", VERSION);


  while ((c = getopt (argc, argv, "n:t")) != EOF)
    {
      switch (c)
        {
        case 'n':
          sscanf (optarg, "%d", &rec_num);
          break;

        case 't':
          timing = NVTrue;
          break;

        default:
          usage ();
          break;
        }
    }


  /* Make sure we got the mandatory file name argument.  */

  if (optind >= argc) usage ();


  strcpy (file, argv[optind]);


  if (strstr (file, ".cwf"))
    {
      if ((czmil_hnd = czmil_open_cwf_file (file, &czmil_cwf_header, CZMIL_READONLY)) < 0)
        {
          czmil_perror ();
          exit (-1);
        }
      wave = NVTrue;
    }
  else if (strstr (file, ".cpf"))
    {
      if ((czmil_hnd = czmil_open_cpf_file (file, &czmil_cpf_header, CZMIL_READONLY)) < 0)
        {
          czmil_perror ();
          exit (-1);
        }
    }
  else
    {
      usage ();
    }


  fprintf (stderr, "\n\n File : %s\n\n", file);


  if (rec_num >= 0)
    {
      if (timing) usage ();

      start = rec_num;
      end = rec_num + 1;
    }
  else
    {
      start = 0;

      if (wave)
        {
          end = czmil_cwf_header.number_of_records;
        }
      else
        {
          end = czmil_cpf_header.number_of_records;
        }

      if (end - start < 100000)
        {
          fprintf (stderr, "\n\nNot enough data to perform ten second timing test.\n\n");
          exit (-1);
        }

      if (timing) end = start + 100000;
    }


  /*  Test block for reading array of CWF records.

  CZMIL_CWF_Data *array;
  array = NULL;

  array = (CZMIL_CWF_Data *) calloc (100000, sizeof (CZMIL_CWF_Data));
  if (array == NULL)
    {
      perror ("Allocating array");
      exit (-1);
    }


  i = czmil_read_cwf_record_array (czmil_hnd, 0, 10000, array);
  fprintf (stderr, "%d\n", i);
  i = czmil_read_cwf_record_array (czmil_hnd, 10000, 10000, array);
  fprintf (stderr, "%d\n", i);
  i = czmil_read_cwf_record_array (czmil_hnd, 20000, 100000, array);
  fprintf (stderr, "%d\n", i);
  i = czmil_read_cwf_record_array (czmil_hnd, 200000, 10000, array);
  fprintf (stderr, "%d\n", i);
  i = czmil_read_cwf_record_array (czmil_hnd, 210000, 100000, array);
  fprintf (stderr, "%d\n", i);

  exit (-1);

*/


  for (i = start ; i < end ; i++)
    {
      if (wave)
        {
          if (czmil_read_cwf_record (czmil_hnd, i, &czmil_cwf) != CZMIL_SUCCESS)
            {
              czmil_perror ();
              exit (-1);
            }

          if (!timing) czmil_dump_cwf_record (czmil_hnd, &czmil_cwf, stderr);
        }
      else
        {
          if (czmil_read_cpf_record (czmil_hnd, i, &czmil_cpf) != CZMIL_SUCCESS)
            {
              czmil_perror ();
              exit (-1);
            }

          if (!timing) czmil_dump_cpf_record (&czmil_cpf, stderr);
        }
    }

  if (wave)
    {
      czmil_close_cwf_file (czmil_hnd);
    }
  else
    {
      czmil_close_cpf_file (czmil_hnd);
    }


  /*  Please ignore the following line.  It is useless.  Except...

      On some versions of Ubuntu, if I compile a program that doesn't use the math
      library but it calls a shared library that does use the math library I get undefined
      references to some of the math library functions even though I have -lm as the last
      library listed on the link line.  This happens whether I use qmake to build the
      Makefile or I have a pre-built Makefile.  Including math.h doesn't fix it either.
      The following line forces the linker to bring in the math library.  If there is a
      better solution please let me know at area.based.editor AT gmail DOT com.  */

  float ubuntu; ubuntu = 4.50 ; ubuntu = fmod (ubuntu, 1.0);


  return (0);
}
