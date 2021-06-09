#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#include "types.h"
#include "bgh.h"
#include "line_list.h"
#include "stream.h"
#include "generator.h"
#ifdef _COMPRESS_IMAGE_
#include "zlib.h"
#endif

/*
 * external variables
 */

extern stBGHGLOB	stBgh;

/*
 * external functions
 */

extern tostBuf *fpstFileMap(char *spsnzHeaderPath);
extern int PutImageIntoDatabase(
                                char *bill_image_string, /* image-information */
                                int bill_image_string_len,
                                char *bill_image_header_string, /* image-header-information */
                                int bill_image_header_string_len,
                                TYPEID type_id,        /* type of document */
                                stDBCOL *stCol 		     /* add. columns */
                                );

/*
 * static variables
 */

static tostImageStream *dpstStream;
static tostBuf *dpstBuf;
static char szTemp[PATH_MAX];

/*
 * static functions
 */

static toenBool foenSaveImage(tostImageStream *ppstStream, tostBuf *ppstBuf);

/**********************************************************************************************************************
 *
 * fpstImageStream_New
 *
 **********************************************************************************************************************
 */

tostImageStream *fpstImageStream_New()
{
  char lasnzPath[MAX_PATH_NAME_LEN];
  char *lpsnzDes;
  tostImageStream *lpstStream;

  /*
   * Get header file if necessary
   */
  
  if (dpstBuf == NULL)
    {
      strcpy(lasnzPath, stBgh.szLayoutDir);
      strcat(lasnzPath, "header_" GEN_VER "_" BGH_VER ".ps" );  
      if ((dpstBuf = fpstFileMap(lasnzPath)) == NULL)
        {
          sprintf (szTemp, "Can't map file: %s\n", lasnzPath);
          macErrorMessage (STREAM_MAP_FILE, WARNING, szTemp);      
          return NULL;
        }
    }

  /*
   * Alloc memory 
   */
  
  if ((lpstStream = (tostImageStream *)calloc(1, sizeof(tostImageStream))) == NULL)
    {
      sprintf (szTemp, "Can't malloc memory\n");
      macErrorMessage (STREAM_MALLOC, WARNING, szTemp);      
      return NULL;
    }
  
  /*
   * Fill the structure
   */

  strcpy(lpstStream->sasnzPathName, stBgh.szImageDir);
  if ((lpstStream->spstLineList = fpstLineList_New()) == NULL)
    {
      sprintf (szTemp, "Can't create line list for stream\n");
      macErrorMessage (STREAM_CREATE_LINE_LIST, WARNING, szTemp);      
      return NULL;
    }
  
  return lpstStream;
}

/**********************************************************************************************************************
 *
 * foenImageStream_Next
 *
 **********************************************************************************************************************
 */

toenBool foenImageStream_Next(tostImageStream *ppstStream, int poiCustomerId, toenBool poenInitializeLineList)
{
    toenBool loenStatus;

    /*
     * Customer id for next document
     */

    ppstStream->soiCustomerId = poiCustomerId;

    /*
     * List must be deleted before next customer processing is started, except the dunning the same customer
     */
    if(TRUE == poenInitializeLineList)
    {
/* Initializing the list without freeing it probably is a mistake
        if ((loenStatus = foenLineList_Init(ppstStream->spstLineList)) == FALSE)
*/
        if ((loenStatus = foenLineList_Delete(ppstStream->spstLineList)) == FALSE)
        {
            sprintf (szTemp, "Can't init line list for customer: %d\n", poiCustomerId);
            macErrorMessage (STREAM_INIT_LINE_LIST, WARNING, szTemp);      
            return FALSE;
        }
    }
    return TRUE;
}

/**********************************************************************************************************************
 *
 * foenImageStream_Print
 *
 **********************************************************************************************************************
 */

toenBool foenImageStream_Print(tostImageStream *ppstStream, char *ppsnzLine)
{
  toenBool loenStatus;
  
  if ((loenStatus = foenLineList_Append(ppstStream->spstLineList, ppsnzLine)) == FALSE)
    {
      sprintf (szTemp, "Can't append line to the line list: %\n", ppsnzLine);
      macErrorMessage (STREAM_APPEND_LINE_LIST, WARNING, szTemp);             
      return FALSE;
    }
  
  return TRUE;
}

/**********************************************************************************************************************
 *
 * foenImageStream_Flush
 *
 **********************************************************************************************************************
 */

toenBool foenImageStream_Flush(tostImageStream *ppstStream)
{
  char lasnzFile[MAX_PATH_NAME_LEN];
  char lasnzDes[MAX_BILL_MEDIUM_DES  + 8];
  char *lpsnzDes, *lpsnzType;
  toenBool loenStatus;
  int rc;

#ifndef _CLEAN_IMAGE_

  /*
   * Create type dependent file name
   */
  
  sprintf(lasnzFile, 
          "%s/image.%08d.ps", 
          ppstStream->sasnzPathName, ppstStream->soiCustomerId);

  /*
   * Open output file
   */
  
  fovdPrintLog (LOG_DEBUG, "fpstImageStream_Flush: File is: %s\n", lasnzFile);  
  if ((ppstStream->sofilFile = open(lasnzFile, O_TRUNC | O_CREAT | O_WRONLY, STREAM_FILE_ACCESS_RIGHTS)) == -1)
    {
      sprintf (szTemp, "Can't open output file: %s, errno = %d\n", lasnzFile, errno);
      macErrorMessage (STREAM_OPEN_OUTPUT_FILE, WARNING, szTemp);      
      return FALSE;
    }

  /*
   * Add header file
   */
  
  fovdPrintLog (LOG_DEBUG, "fpstImageStream_Flush: Adding header\n");
  if ((rc = write(ppstStream->sofilFile, dpstBuf->spsnVal, dpstBuf->soiLen)) == -1)
    {
      sprintf (szTemp, "Can't write header to the output file: %\n", lasnzFile);
      macErrorMessage (STREAM_WRITE_OUTPUT_FILE, WARNING, szTemp); 
      return FALSE;
    }
  
  /*
   * Write all lines from the queue
   */
  
  fovdPrintLog (LOG_DEBUG, "fpstImageStream_Flush: Writing document\n");
  if ((loenStatus = foenLineList_Write(ppstStream->spstLineList, ppstStream->sofilFile)) == FALSE)
    {
      sprintf (szTemp, "Can't write document to the output file: %\n", lasnzFile);
      macErrorMessage (STREAM_WRITE_OUTPUT_FILE, WARNING, szTemp); 
      return FALSE;
    }

  /*
   * close output file
   */

  if ((rc = close(ppstStream->sofilFile)) == -1)
    {
      sprintf (szTemp, "Can't close the output file: %\n", lasnzFile);
      macErrorMessage (STREAM_CLOSE_OUTPUT_FILE, WARNING, szTemp); 
      return FALSE;
    }

#endif
  
  /*
   * Clean module memory
   */

  fovdPrintLog (LOG_DEBUG, "fpstImageStream_Flush: Deleting line list\n");
  if ((loenStatus = foenLineList_Delete(ppstStream->spstLineList)) == FALSE)
    {
      sprintf (szTemp, "Can't delete the line list for the output file: %\n", lasnzFile);
      macErrorMessage (STREAM_DELETE_LINE_LIST, WARNING, szTemp);       
      return FALSE;
    } 

  return TRUE;
}

/**********************************************************************************************************************
 *
 * foenImageStream_Insert
 *
 **********************************************************************************************************************
 */

toenBool foenImageStream_Insert(tostImageStream *ppstStream, int poiImgTypeId)
{
  int rc;
  toenBool loenStatus;
  tostBuf lostBuf;
  tostBuf lostCompBuf;
  stDBCOL lostCol;
#ifdef _COMPRESS_IMAGE_
  uLongf len;
#endif

  /*
   * Buffer must be filled with the image of bill
   */

  fovdPrintLog (LOG_DEBUG, "foenImageStream_Insert: Filling memory with image\n");
  if ((loenStatus = foenLineList_FillBuf(ppstStream->spstLineList, &lostBuf)) == FALSE)
    {
      sprintf (szTemp, "Can't convert line list to buffer\n");
      macErrorMessage (STREAM_CONVERT_LINE_LIST, WARNING, szTemp);             
      return FALSE;
    }

#ifdef _COMPRESS_IMAGE_

  /*
   * Image may be compressed before insert
   */

  if (stBgh.bCompressImage == TRUE)
    {
      /*
       * Set up input compression data
       */

      lostCompBuf.soiLen = lostBuf.soiLen + (int)((double)lostBuf.soiLen * 0.1) + 12;
      fovdPrintLog (LOG_DEBUG, "foenImageStream_Insert: Comp. buffer size: %d\n", lostCompBuf.soiLen);
      if ((lostCompBuf.spsnVal = calloc(lostCompBuf.soiLen, sizeof(char))) == NULL)
        {
          sprintf (szTemp, "Can't malloc memory\n");
          macErrorMessage (STREAM_MALLOC, WARNING, szTemp);      
          return FALSE;
        }

      /*
       * Stat compression of the image
       */
      
      len = lostCompBuf.soiLen; 
      rc = compress((Bytef *)lostCompBuf.spsnVal, 
                    &len,
                    (Bytef *)lostBuf.spsnVal, 
                    (uLongf)lostBuf.soiLen); 
      
      if (rc == Z_MEM_ERROR)
        {
          sprintf (szTemp, "Z_MEM_ERROR: Not enough memory during compression\n");
          macErrorMessage (STREAM_Z_MEM_ERROR, WARNING, szTemp);             
          return FALSE;          
        }
      else if (rc == Z_BUF_ERROR)
        {
          sprintf (szTemp, "Z_BUF_ERROR: Not enough memory in the output buffer\n");
          macErrorMessage (STREAM_Z_BUF_ERROR, WARNING, szTemp);             
          return FALSE;          
        }
      else if (rc == Z_OK)
        {}
      else
        {
          sprintf (szTemp, "Compression status not Z_OK\n");
          macErrorMessage (STREAM_COMPRESSION_ERROR, WARNING, szTemp);             
          return FALSE;          
        }

        free(lostBuf.spsnVal);
        
        /*
         * Use compressed buffer
         */

        lostBuf.spsnVal = lostCompBuf.spsnVal;
        lostBuf.soiLen = len;

        /*
         * Save compressed image
         */

        if ((loenStatus = foenSaveImage(ppstStream, &lostBuf)) == FALSE)
          {
            sprintf (szTemp, "Can't save image\n");
            macErrorMessage (STREAM_SAVE_BILL_IMAGE, WARNING, szTemp);                         
            return FALSE;
          }
    }

#endif
  
  /*
   * Inserting image to BILL_IMAGES ising standard interface
   */

  fovdPrintLog (LOG_DEBUG, "foenImageStream_Insert: Inserting image with: %d bytes\n", lostBuf.soiLen);
  if ((rc = PutImageIntoDatabase(lostBuf.spsnVal, lostBuf.soiLen, 
                                 dpstBuf->spsnVal, dpstBuf->soiLen, 
                                 poiImgTypeId, &lostCol)) != 0)
    {
      sprintf (szTemp, "Can't insert document to table BILL_IMAGES\n");
      macErrorMessage (STREAM_INSERT_BILL_IMAGE, WARNING, szTemp);             
      return FALSE;
    }

  fovdPrintLog (LOG_TIMM, "Inserted image with: %d bytes\n", lostBuf.soiLen);

  /*
   * Memory used for the image must be deallocated
   */

  free(lostBuf.spsnVal);

  return TRUE;
}

static toenBool foenSaveImage(tostImageStream *ppstStream, tostBuf *ppstBuf)
{
  int rc;
  char lasnzFileName[MAX_PATH_NAME_LEN];
  char lofilFile;
  
  rc = sprintf(lasnzFileName, 
               "%s%s%.08d.ps.gz",
               stBgh.szImageDir, 
               "image.",
               ppstStream->soiCustomerId);  
  
  if ((lofilFile = open(lasnzFileName, O_CREAT | O_WRONLY, S_IRUSR | S_IRGRP)) == -1)
    {
      sprintf (szTemp, "Can't open output file: %\n", lasnzFileName);
      macErrorMessage (STREAM_OPEN_OUTPUT_FILE, WARNING, szTemp);      
      return FALSE;
    }
  
  if ((rc = write(lofilFile, ppstBuf->spsnVal, ppstBuf->soiLen)) == -1)
    {
      sprintf (szTemp, "Can't write image to the output file: %s\n", lasnzFileName);
      macErrorMessage (STREAM_WRITE_OUTPUT_FILE, WARNING, szTemp); 
      return FALSE;
    }
  
  if ((rc = close(lofilFile)) == -1)
    {
      sprintf (szTemp, "Can't close the output file: %\n", lasnzFileName);
      macErrorMessage (STREAM_CLOSE_OUTPUT_FILE, WARNING, szTemp);       
      return FALSE;
    }

  return TRUE;
}



  

