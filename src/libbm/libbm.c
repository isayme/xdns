#include <stdio.h>
#include "libbm.h"

static int preBmBc(unsigned char *pattern, int pattern_len, int bmBc[])
{
    int i;
    
    if (NULL == pattern || 0 > pattern_len || NULL == bmBc)
    {
        return R_ERROR;
    }
    
    for (i = 0; i < ASIZE; ++i)
    {
#ifdef BM_VERSION
        bmBc[i] = -1;
#else
        bmBc[i] = pattern_len;
#endif              
    }

    for (i = 0; i < pattern_len - 1; ++i)
    {
#ifdef BM_VERSION
        bmBc[pattern[i]] = i ;
#else
        bmBc[pattern[i]] = pattern_len - i - 1;
#endif      
    }
      
    return R_OK;
}

static int suffixes(unsigned char *pattern, int pattern_len, int *suffix)
{
    int f, g, i;

    if (NULL == pattern || 0 > pattern_len || NULL == suffix)
    {
        return R_ERROR;
    }
    
    suffix[pattern_len - 1] = pattern_len;
   
    g = pattern_len - 1;
    for (i = pattern_len - 2; i >= 0; --i)
    {
        if (i > g && suffix[i + pattern_len - 1 - f] < i - g)
        {
            suffix[i] = suffix[i + pattern_len - 1 - f];
        }
        else
        {
            if (i < g)
            {
                g = i;
            }
            
            f = i;
            
            while (g >= 0 && pattern[g] == pattern[g + pattern_len - 1 - f])
            {
                --g;
            }
            
            suffix[i] = f - g;
        }
    }
   
    return R_OK;
   
    /* Faster ablove code, general below
    suffix[m-1] = m;
    for (i = m-2; i >= 0; --i){
        q = i;
        while (q >= 0 && P[q] == P[m - 1 - i + q])
            --q;
        suffix[i]=i-q;
    }*/
}
 
static int preBmGs(unsigned char *pattern, int pattern_len, int bmGs[])
{
    int i, j, suffix[XSIZE];

    if (NULL == pattern || 0 > pattern_len || NULL == bmGs)
    {
        return R_ERROR;
    }
    
    suffixes(pattern, pattern_len, suffix);
    
    for (i = 0; i < pattern_len; ++i)
    {
        bmGs[i] = pattern_len;
    }
      
    j = 0;
    for (i = pattern_len - 1; i >= 0; --i)
    {
        if (suffix[i] == i + 1)
        {
            for (; j < pattern_len - 1 - i; ++j)
            {
                if (bmGs[j] == pattern_len)
                {
                    bmGs[j] = pattern_len - 1 - i;
                }
            }
        }
    }
               
    for (i = 0; i <= pattern_len - 2; ++i)
    {
        bmGs[pattern_len - 1 - suffix[i]] = pattern_len - 1 - i;
    }  
      
    return R_OK;
}
 
 
int BM(unsigned char *pattern, int pattern_len, unsigned char *text, int text_len)
{
    int i, j, bmGs[XSIZE], bmBc[ASIZE];
    
    if (NULL == pattern || 0 > pattern_len || NULL == text || text_len < pattern_len)
    {
        return R_ERROR;
    }
    
    /* Preprocessing */
    if (R_ERROR == preBmGs(pattern, pattern_len, bmGs))
    {
        return R_ERROR;
    }
    if (R_ERROR == preBmBc(pattern, pattern_len, bmBc))
    {
        return R_ERROR;
    }
    
    /* Searching */
    j = 0;
    while (j <= text_len - pattern_len)
    {
        for (i = pattern_len - 1; i >= 0 && pattern[i] == text[i + j]; --i);
        if (i < 0)
        {
            //j += bmGs[0];
            return j;
        }
        else
        {
#ifdef BM_VERSION
            j += MAX(bmGs[i], i - bmBc[text[i + j]]);
#else
            j += MAX(bmGs[i], bmBc[text[i + j]] - pattern_len + 1 + i);
#endif
        } 
    }
    
    return R_ERROR;
}
