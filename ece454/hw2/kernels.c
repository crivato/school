/********************************************************
 * Kernels to be optimized for the CS:APP Performance Lab
 ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "defs.h"

/* 
 * ECE454 Students: 
 * Please fill in the following team struct 
 */
team_t team = {
    "S·P·Q·R·",              /* Team name */

    "Cristian Valentini",     /* First member full name */
    "cristian.valentini@utoronto.ca",  /* First member email address */

    "",                   /* Second member full name (leave blank if none) */
    ""                    /* Second member email addr (leave blank if none) */
};

/***************
 * ROTATE KERNEL
 ***************/

/******************************************************
 * Your different versions of the rotate kernel go here
 ******************************************************/

/* 
 * naive_rotate - The naive baseline version of rotate 
 */
char naive_rotate_descr[] = "naive_rotate: Naive baseline implementation";
void naive_rotate(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    for (i = 0; i < dim; i++)
	for (j = 0; j < dim; j++)
	    dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
}

/*
 * ECE 454 Students: Write your rotate functions here:
 */ 




/* 
 * rotate - Your current working version of rotate
 * IMPORTANT: This is the version you will be graded on
 */
char rotate_descr[] = "rotate: Current working version";
void rotate(int dim, pixel *src, pixel *dst) 
{
	int i, j, count;
	int cacheBuffer = 16; 
	int jump = 4;   
	src += dim - 1; 

	if(dim < 1024){
		cacheBuffer = 32;
		jump = 5;
	}

	if(dim == 64 || dim == 128){
		cacheBuffer = 64;
		jump = 6;
	}

	for(count = dim >> jump; count > 0 ; count--, src += dim * (cacheBuffer + 1), dst -= dim * dim - cacheBuffer){
		for(i = dim; i > 0 ; i--, src -= dim * cacheBuffer + 1, dst += dim - cacheBuffer){
			for(j = cacheBuffer; j > 0 ; j--,src += dim){
                		*dst++ = *src;
         		} 	
		} 	
	}
}

/*
Rotate: Version = rotate: Current working version:
Dim		64	128	256	512	1024	2048	4096	Mean
Your CPEs	4.2	4.3	4.3	6.4	15.0	18.3	19.8
Baseline CPEs	5.1	6.4	10.7	15.3	20.4	102.3	114.1
Speedup		1.2	1.5	2.5	2.4	1.4	5.6	5.8	2.4
*/


/********************************************************************
*********************************************************************
*********************************************************************
*
*		The following is rough work.
*	
*********************************************************************
*********************************************************************
*********************************************************************/

//REMINDER:
//RIDX(i,j,n) actually means ((i)*(n)+(j))

char rotate_two_descr[] = "second attempt: remove RIDX macro";
void attempt_two(int dim, pixel *src, pixel *dst) 
{
	int i, j, n;
	for (i = 0; i < dim; i++){
		n = i*dim;
		for (j = 0; j < dim; j++)
		    dst[RIDX(dim-1-j, i, dim)] = src[n + j];
	}
}



char rotate_three_descr[] = "third attempt: unrolling inner loop";
void attempt_three(int dim, pixel *src, pixel *dst) 
{
	int i, j;
	for (j = 0; j < dim; j++)
	{
		for (i = 0; i < dim; i+=4)
		{
			dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
			dst[RIDX(dim-1-j, i+1, dim)] = src[RIDX(i+1, j, dim)];
			dst[RIDX(dim-1-j, i+2, dim)] = src[RIDX(i+2, j, dim)];
			dst[RIDX(dim-1-j, i+3, dim)] = src[RIDX(i+3, j, dim)];
		}
	}
	for (; j < dim; j++)
		for (; i < dim; i++)
			dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
}



char rotate_four_descr[] = "fourth attempt: combining attempts 2 + 3";
void attempt_four(int dim, pixel *src, pixel *dst) 
{
	int i, j, n;
	for (j = 0; j < dim; j++){
		n = (dim-1-j)*dim;
		for (i = 0; i < dim; i+=4){
			dst[n + i] = src[RIDX(i, j, dim)];
			dst[n + i+1] = src[RIDX(i+1, j, dim)];
			dst[n + i+2] = src[RIDX(i+2, j, dim)];
			dst[n + i+3] = src[RIDX(i+3, j, dim)];
		}
	}
	for (; j < dim; j++)
		for (; i < dim; i++)
			dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
}



char rotate_five_descr[] = "fifth attempt: use \"matrix within matrix\" (see lecture notes)";
void attempt_five(int dim, pixel *src, pixel *dst) 
{
	int i, j, count;
	int cacheBuffer = 32;
	src += dim - 1; 

	for(count = dim >> 5; count > 0 ; count--){
		for(i = dim; i > 0 ; i--){
			for(j = 32; j > 0 ; j--,src += dim){
                		*dst++ = *src;
           		} 
			src -= dim * cacheBuffer + 1;
			dst += dim - cacheBuffer;
		} 
		src += dim * (cacheBuffer + 1);
		dst -= dim * dim - cacheBuffer;
	}
}



char rotate_six_descr[] = "sixth attempt: \"matrix within matrix\": lower buffer (ie. more/smaller matrices)";
void attempt_six(int dim, pixel *src, pixel *dst) 
{
	int i, j, count;
	int cacheBuffer = 16;
	src += dim - 1; 

	for(count = dim >> 4; count > 0 ; count--){
		for(i = dim; i > 0 ; i--){
			for(j = 16; j > 0 ; j--,src += dim){
                		*dst++ = *src;
           		} 
			src -= dim * cacheBuffer + 1;
			dst += dim - cacheBuffer;
		} 
		src += dim * (cacheBuffer + 1);
		dst -= dim * dim - cacheBuffer;
	}
}



char rotate_seven_descr[] = "seventh attempt: \"matrix within matrix\": combine above findings";
void attempt_seven(int dim, pixel *src, pixel *dst) 
{
	int i, j, count;
	int cacheBuffer = 16; 
	int jump = 4;   
	src += dim - 1; 

	if(dim < 1024){
		cacheBuffer = 32;
		jump = 5;
	}

	for(count = dim >> jump; count > 0 ; count--){
		for(i = dim; i > 0 ; i--){
			for(j = cacheBuffer; j > 0 ; j--,src += dim){
                		*dst++ = *src;
         		} 
			src -= dim * cacheBuffer + 1;
			dst += dim - cacheBuffer;
		} 
		src += dim * (cacheBuffer + 1);
		dst -= dim * dim - cacheBuffer;
	}
}



char rotate_eight_descr[] = "eighth attempt: \"matrix within matrix\": extend to 128 ";
void attempt_eight(int dim, pixel *src, pixel *dst) 
{
	int i, j, count;
	int cacheBuffer = 16; 
	int jump = 4;   
	src += dim - 1; 
   
	if(dim < 1024){
		cacheBuffer = 32;
		jump = 5;
	}

	else if(dim == 4096){
		cacheBuffer = 128;
		jump = 7;
	}

	for(count = dim >> jump; count > 0 ; count--){
		for(i = dim; i > 0 ; i--){
			for(j = cacheBuffer; j > 0 ; j--,src += dim){
                		*dst++ = *src;
         		} 
			src -= dim * cacheBuffer + 1;
			dst += dim - cacheBuffer;
		} 
		src += dim * (cacheBuffer + 1);
		dst -= dim * dim - cacheBuffer;
	}
}

/*********************************************************************
**********************************************************************
*  Other buffers were expiremented with.  Results are as follows.
**********************************************************************
**********************************************************************
*
*WITH 16
Rotate: Version = sixth attempt -- use matrix within matrix: change buffer:
Dim		64	128	256	512	1024	2048	4096	Mean
Your CPEs	4.5	4.6	4.7	7.6	16.6	19.3	20.4
Baseline CPEs	5.1	6.4	10.7	15.3	20.4	102.3	114.1
Speedup		1.1	1.4	2.3	2.0	1.2	5.3	5.6	2.2
*
*
*WITH 32
Rotate: Version = fifth attempt -- use matrix within matrix:
Dim		64	128	256	512	1024	2048	4096	Mean
Your CPEs	4.4	4.3	4.3	6.5	17.3	20.0	21.1
Baseline CPEs	5.1	6.4	10.7	15.3	20.4	102.3	114.1
Speedup		1.2	1.5	2.5	2.4	1.2	5.1	5.4	2.3
*
*
*WITH 16 & 32
Rotate: Version = seventh attempt -- use matrix within matrix: change buffer for seperate:
Dim		64	128	256	512	1024	2048	4096	Mean
Your CPEs	4.3	4.3	4.4	6.5	14.7	18.2	19.9
Baseline CPEs	5.1	6.4	10.7	15.3	20.4	102.3	114.1
Speedup		1.2	1.5	2.5	2.3	1.4	5.6	5.7	2.4
*
*
* WITH 16, 32, 126
Rotate: Version = ninth attempt -- use matrix within matrix: extend to 128 :
Dim		64	128	256	512	1024	2048	4096	Mean
Your CPEs	4.6	4.6	4.7	7.0	15.4	18.4	19.3
Baseline CPEs	5.1	6.4	10.7	15.3	20.4	102.3	114.1
Speedup		1.1	1.4	2.3	2.2	1.3	5.6	5.9	2.3
*
*********************************************************************
*********************************************************************/


//This is a rescursion helper function
void recursionHelper(int count, int cacheBuffer, int dim, pixel *src, pixel *dst) {
	count--;
	if( !(count > 0))
		return;

	else{
	int i, j;
		for(i = dim; i > 0 ; i--){
			for( j = cacheBuffer; j > 0 ; j--,src += dim){
	                	*dst++ = *src;
	         	} 
			src -= dim * cacheBuffer + 1;
			dst += dim - cacheBuffer;
		} 
		src += dim * (cacheBuffer + 1);
		dst -= dim * dim - cacheBuffer;
	
		recursionHelper(count, cacheBuffer, dim, src, dst);
	}
}
//END OF recursion helper function

char rotate_nine_descr[] = "ninth attempt: \"matrix within matrix\" implimented with recursion instead of loops";
void attempt_nine(int dim, pixel *src, pixel *dst) 
{
	int cacheBuffer = 16; 
	int jump = 4;   
	src += dim - 1; 

	if(dim < 1024){
		cacheBuffer = 32;
		jump = 5;
	}

	int count = (dim >> jump) +1;
	recursionHelper(count, cacheBuffer, dim, src, dst);
}



char rotate_ten_descr[] = "tenth attempt: \"matrix within matrix\": buffer extended -- exclusively using L2 cache ";
void attempt_ten(int dim, pixel *src, pixel *dst) 
{
	int i, j, count;
	int cacheBuffer = 32; 
	int jump = 5;   
	src += dim - 1; 
   
	if(dim >= 512){
		cacheBuffer = 512;
		jump = 9;
	}

	for(count = dim >> jump; count > 0 ; count--){
		for(i = dim; i > 0 ; i--){
			for(j = cacheBuffer; j > 0 ; j--,src += dim){
                		*dst++ = *src;
         		} 
			src -= dim * cacheBuffer + 1;
			dst += dim - cacheBuffer;
		} 
		src += dim * (cacheBuffer + 1);
		dst -= dim * dim - cacheBuffer;
	}
}


/*********************************************************************
 * register_rotate_functions - Register all of your different versions
 *     of the rotate kernel with the driver by calling the
 *     add_rotate_function() for each test function. When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/

void register_rotate_functions() 
{
    add_rotate_function(&rotate, rotate_descr); 
    add_rotate_function(&naive_rotate, naive_rotate_descr);   


    add_rotate_function(&attempt_two, rotate_two_descr);   
    add_rotate_function(&attempt_three, rotate_three_descr);  
    add_rotate_function(&attempt_four, rotate_four_descr); 
    add_rotate_function(&attempt_five, rotate_five_descr);  
    add_rotate_function(&attempt_six, rotate_six_descr); 
    add_rotate_function(&attempt_seven, rotate_seven_descr); 
    add_rotate_function(&attempt_eight, rotate_eight_descr);   
    add_rotate_function(&attempt_nine, rotate_nine_descr);   
    add_rotate_function(&attempt_ten, rotate_ten_descr);   
    //add_rotate_function(&attempt_eleven, rotate_eleven_descr);   

    /* ... Register additional rotate functions here */
}

