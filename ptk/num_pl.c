/**************************************************************************/
/*  MODULE : Translator of numbers                                        */
/*                                                                        */
/*  AUTHOR : N.Bondarczuk                                                 */
/*                                                                        */
/*  CREATION DATE : 18.11.97                                              */
/*                                                                        */
/*  DESCRIPTION : Exports  functions translating numbers into  words in   */
/*                polish                                                  */
/**************************************************************************/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "strutl.h"

#if 0	/* just for version.sh */
static char *SCCS_VERSION = "4.1.1";
#endif

#define SPACE " "
#define EOS '\0'
#define DOT '.'
#define INDEX(a) (a - '0')

#include "strings_pl.h"

void init() 
{}

int split(char *src, char *prefix, char *suffix) 
{
  int i, j, dest;

  dest = 0;
  for (i = 0, j = 0; i < strlen(src); i++)
    {
      if (isdigit(src[i])) 
        {
          if (dest == 0)
            {
              prefix[j++] = src[i];
            }
          else
            {
              suffix[j++] = src[i];
            }          
        }
      else if (src[i] == '.' || src[i] == ',')
        {
          prefix[j] = '\0';
          j = 0;
          dest = 1;
        }
      else
        {
          /* */
        }
    }
  
  return 1;
}

int moa2str_pl(char *moa_buf, int moa_buf_len, char *out, int out_buf_len) 
{
  int sign, j, i, iRet, n;
  static char moa[MAX_BUFFER];
  static char prefix[MAX_BUFFER], suffix[MAX_BUFFER];
  char *str_a, *str_b;
  char *in_words(char *, int); 
  double lofConv;

  memset(moa, 0, MAX_BUFFER);
  memset(prefix, 0, MAX_BUFFER);
  memset(suffix, 0, MAX_BUFFER);

  sign = 1;
  for (i = 0, j = 0; i < strlen(moa_buf) + 1; i++)
    {
      if (moa_buf[i] == '-')
        {
          sign = -1;
        }
      else
        {
          moa[j++] = moa_buf[i];
        }
    }

  iRet = split(moa, prefix, suffix);
  if (iRet == 0)
    {
      return 0;
    }

  if (sign == -1)
    {
      strcpy(out, STR_MINUS_PL);
    }
  else
    {
      out[0] = '\0';
    }

  n = atoi(prefix);
  if (n == 0)
    {
      strcat(out, zz);
    }
  else
    {
      str_a = in_words(prefix, 1);
      strcat(out, str_a+1);
    }

  strcat(out, SEP);

  n = atoi(suffix);
  if (n == 0)
    {
      strcat(out, zg);
    }
  else
    {
      str_b = in_words(suffix, 0);
      strcat(out, str_b+1);
    }

  return 1;
}

#define JEDYNKI 0
#define DZIESIATKI 1
#define SETKI 2
#define ASCII2NUM(a) (a - '0')

int number(char *, int, int, int);
char *shape(int, int, int);
void mirror(char *);

char *in_words(char *input_pattern, int zlote) 
{
  static char pattern[MAX_BUFFER];
  static char output[512];
  int i, j, k, p, n;
  char *str;
  char *suf;
  int num, last, next, plast;
  signed int val;

  sprintf(pattern, "%s", input_pattern);
  output[0] = '\0';
  n = sscanf(pattern, "%d", &val);
  if (val < 0)
    {
      val = (-1) * val;
    }

  n = strlen(pattern);
  for (i = 0; i < n; i++)
    {
      j = n - i - 1;
      switch(j%3)
        {
        case JEDYNKI:
          next = 0;
          k = ASCII2NUM(pattern[i]);
          str = shape(next, k, j);
          break;

        case DZIESIATKI:
          k = ASCII2NUM(pattern[i]);
          if (k == 1 && (i+1) < n)
            {
              next = ASCII2NUM(pattern[i+1]);
            }
          else
            {
              next = 0;
            }
          
          str = shape(next, k, j);
          if (k == 1 && next != 0)
            {
              i++;
              j = n - i - 1;
            }
          break;
     
        case SETKI:
          k = ASCII2NUM(pattern[i]);
          next = 0;
          str = shape(next, k, j);
          break;	 
        }
      strcat(output, " ");
      strcat(output, str);

      switch (j)
        {
        case 0:
          num =  number(pattern, i - 2, i - 1, i);
          last = ASCII2NUM(pattern[i]);
          plast = ASCII2NUM(pattern[i-1]);
          if (val == 0)
            {
              if (zlote)
                {
                  suf = STR_ZLOTYCH_PL;
                }
              else
                {
                  suf = STR_GROSZY_PL;
                }
            }
          else if (val == 1)
            {
              if (zlote)
                {
                  suf = STR_ZLOTY_PL;
                }
              else
                {
                  suf = STR_GROSZ_PL;
                }
            }
          else if ((last == 2 || last == 3 || last == 4) && (plast != 1))
            {
              if (zlote)
                {
                  suf = STR_ZLOTE_PL;
                }
              else
                {
                  suf = STR_GROSZE_PL;
                }
            }
          else
            {
              if (zlote)
                {
                  suf = STR_ZLOTYCH_PL;
                }
              else
                {
                  suf = STR_GROSZY_PL;
                }
            }

          strcat(output, " ");
          strcat(output, suf);
          break;

        case 3:
          num =  number(pattern, i%3 - 2, i%3 - 1, i%3);
          last = ASCII2NUM(pattern[i]);
          plast = ASCII2NUM(pattern[i-1]);
          if (num == 0)
            {
              suf = STR_TYSIECY_PL;
            }
          else if (num == 1)
            {
              suf = STR_TYSIAC_PL;
            }
          else if ((last == 2 || last == 3 || last == 4) && (plast != 1))
            {
              suf = STR_TYSIACE_PL;
            }
          else
            {
              suf = STR_TYSIECY_PL;
            }

          strcat(output, " ");
          strcat(output, suf);
          break;

        case 6:
          num =  number(pattern, i - 2, i - 1, i);
          last = ASCII2NUM(pattern[i]);
          plast = ASCII2NUM(pattern[i-1]);
          if (num == 0)
            {
              suf = STR_MILIONOW_PL;
            }
          else if (num == 1)
            {
              suf = STR_MILION_PL;
            }
          else if ((last == 2 || last == 3 || last == 4) && (plast != 1))
            {
              suf = STR_MILIONY_PL;

            }
          else
            {
              suf = STR_MILIONOW_PL;
            }
          
          strcat(output, " ");
          strcat(output, suf);
          break;
        }
    }

  return output;
}

char *shape(int next, int k, int i) {
  char *str;

  switch (i%3)
    {
    case JEDYNKI: 
      str = jedynka[k];
      break;

    case DZIESIATKI:
      if (k == 1 && next != 0)
        {
          str = nastka[next];
        }
      else
        {
          str = dziesiatka[k];
        }
      break;

    case SETKI:
      str = setka[k];
      break;
    }

  return str;
}


void mirror(char *str)
{
  int n, i;

  void swap(char *, int, int);

  n = strlen(str);
  for (i = 0; i < n/2; i++)
    {
      swap(str, i, n - i - 1);
    }
  
}

void swap(char *tab, int a, int b)
{
  char t;

  t = tab[b];
  tab[b] = tab[a];
  tab[a] = t;
}

int number(char *pattern, int i2, int i1, int i0)
{
  int n;

  if (i2 == 0 && i1 == 1 && i0 == 2)
    {
      n = ASCII2NUM(pattern[i2])*100 + ASCII2NUM(pattern[i1])*10 + ASCII2NUM(pattern[i0]);
    }
  if (i2 == -1 && i1 == 0 && i0 == 1)
    {
      n = ASCII2NUM(pattern[i1])*10 + ASCII2NUM(pattern[i0]);
    }
  if (i2 == -2 && i1 == -1 && i0 == 0)
    {
      n = ASCII2NUM(pattern[i0]);
    }
  
  return n;
}











