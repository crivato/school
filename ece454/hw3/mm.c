/*
 *
 * This implemenation stores all free elements in a doubly linked list.
 * The head and tail are universally declared variables. 
 * Any blocks that are subsequently freed will have the first block after the head point to the next free block
 * and the second block after the header point to the previous block.
 * The third block contains a flag (1010101010)
 * The user made functions include add to list and remove from list. 
 * These functions take a pointer to a free block and will remove it from the linked list
 *
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "S·P·Q·R·",
    /* First member's full name */
    "Cristian Valentini",
    /* First member's email address */
    "cristian.valentini@utoronto.ca",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/*************************************************************************
 * Basic Constants and Macros
 * You are not required to use these macros but may find them helpful.
*************************************************************************/
#define WSIZE       4            /* word size (bytes) */
#define DSIZE       8            /* doubleword size (bytes) */
#define CHUNKSIZE   (1<<7)      /* initial heap size (bytes) */
#define OVERHEAD    8            /* overhead of header and footer (bytes) */

#define MAX(x,y) ((x) > (y)?(x) :(y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)          (*(size_t *)(p))
#define PUT(p,val)      (*(size_t *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)     (GET(p) & ~0x7)
#define GET_ALLOC(p)    (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)        ((char *)(bp) - WSIZE)
#define FTRP(bp)        ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

#define NEXT(bp) ((char *)(bp) + WSIZE )
#define PREV(bp) ((char *)(bp) - WSIZE )

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

void* heap_listp = NULL;

size_t maxmem = 1;
void* UHead = NULL;
void* UTail = NULL;

/**********************************************************
 * remove_from_ll
 * removes block from linked list
 **********************************************************/
void remove_from_ll(void* bp)
{
    void* ciel = (void*)GET(bp);
    void* floor = (void*)GET(bp+WSIZE);
    PUT( bp+DSIZE , 0000000000 );

    if(floor == UHead)
    {
        if (ciel == UTail)
        {
            PUT( ciel , (size_t)floor );
            PUT( floor , (size_t)ciel  );
        }
        else
        {
           PUT( floor , (size_t)ciel  );
           PUT( NEXT(ciel) , (size_t)floor );
        }
    }
    else if (ciel == UTail)
    {
        PUT( ciel , (size_t)floor );
        PUT( PREV(floor) , (size_t)ciel  );
    }
    else
    {
        PUT( NEXT(ciel) , (size_t)floor );
        PUT( PREV(floor) , (size_t)ciel  );
    }
    return;
}

/**********************************************************
 * add_to_end_ll
 * adds free block inbetween tail and last element
 **********************************************************/
void add_to_end_ll(void* bp)
{
	if( (size_t)UHead == GET(UTail)) //first free element
	{
		PUT(UHead, (size_t)bp);
		PUT(UTail, (size_t)NEXT(bp));
		
		PUT( bp, (size_t)UTail);
		PUT( NEXT(bp), (size_t)UHead);
		PUT( bp+DSIZE , 1010101010 ); 
		return;
	}

	else //place at end of linked list
	{
		PUT( bp+WSIZE, GET(UTail)); 
		PUT( (GET(UTail)-WSIZE) , (size_t)bp );    
		PUT( bp, (size_t)UTail );
		PUT( UTail, (size_t)NEXT(bp));
		PUT( bp+DSIZE , 1010101010 ); 

		return;
	}
}

/**********************************************************
 * mm_init
 * Initialize the heap, including "allocation" of the
 * prologue and epilogue
 **********************************************************/
 int mm_init(void)
 {
     if ((heap_listp = mem_sbrk(4*WSIZE)) == -1)
         return -1;
     PUT(heap_listp, 0);                         // alignment padding 
     PUT(heap_listp+WSIZE, PACK(OVERHEAD, 1));   // prologue header
     PUT(heap_listp+DSIZE, PACK(OVERHEAD, 1));   // prologue footer
     PUT(heap_listp+WSIZE+DSIZE, PACK(0, 1));    // epilogue header
     heap_listp += DSIZE; 
     maxmem = 1;
 
     return 0;
 }
 
/**********************************************************
 * coalesce
 * Covers the 4 cases discussed in the text:
 * - both neighbours are allocated
 * - the next block is available for coalescing
 * - the previous block is available for coalescing
 * - both neighbours are available for coalescing
 **********************************************************/
void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {       /* Case 1 */
        return bp;
    }

    else if (prev_alloc && !next_alloc) { /* Case 2 */
	
	if (GET(NEXT_BLKP(bp)+DSIZE) == 1010101010) //checks for flag before coalescing
		remove_from_ll(NEXT_BLKP(bp));

        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        return (bp);
    }

    else if (!prev_alloc && next_alloc) { /* Case 3 */

	if (GET(PREV_BLKP(bp)+DSIZE) == 1010101010)
		remove_from_ll(PREV_BLKP(bp));

        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        return (PREV_BLKP(bp));
    }

    else {            /* Case 4 */
	if (GET(NEXT_BLKP(bp)+DSIZE) == 1010101010 && GET(PREV_BLKP(bp)+DSIZE) == 1010101010)
	{
		remove_from_ll(NEXT_BLKP(bp));
		remove_from_ll(PREV_BLKP(bp));
	}

        size += GET_SIZE(HDRP(PREV_BLKP(bp)))  +
            GET_SIZE(FTRP(NEXT_BLKP(bp)))  ;
        PUT(HDRP(PREV_BLKP(bp)), PACK(size,0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size,0));
        return (PREV_BLKP(bp));
    }
}

/**********************************************************
 * extend_heap
 * Extend the heap by "words" words, maintaining alignment
 * requirements of course. Free the former epilogue block
 * and reallocate its new header
 **********************************************************/
void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignments */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    if ( (bp = mem_sbrk(size)) == -1 )
        return NULL;

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));                // free block header
    PUT(FTRP(bp), PACK(size, 0));                // free block footer
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));        // new epilogue header

    bp = coalesce(bp);
    size = GET_SIZE(HDRP(bp));

	if(maxmem == 2){
	    maxmem++;
	}
	else{ 
		add_to_end_ll(bp);
	}
	return bp;
}


/**********************************************************
 * find_fit
 * Traverse the heap searching for a block to fit asize
 * Return NULL if no free blocks can handle that size
 * Assumed that asize is aligned
 **********************************************************/
void * find_fit(size_t asize) 
{
	if(maxmem == 2 || (size_t)UHead == GET(UTail) ){
	    return NULL;
	}

	if( (size_t)UHead == GET(UTail))
	{
		return NULL;
	}

	void *bp;
	for (bp = (void *)GET(UHead); GET_SIZE(HDRP(bp)) > 0 && bp != UTail; bp = (void *)GET(bp))
	{
        	if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp))) && GET( bp+DSIZE ) == 1010101010 )
        	{
        	    return bp;
        	}
    	}
	return NULL;
}



/**********************************************************
 * place
 * Mark the block as allocated
 **********************************************************/
void place(void* bp, size_t asize) //Delete element from linked list
{
	/* Get the current block size */
	size_t bsize = GET_SIZE(HDRP(bp));
	size_t csize = bsize - asize;

	if(128 <  csize) //splits allocated block, if there is a lot of room leftover
	{
		if( GET( bp+DSIZE ) == 1010101010 && !GET_ALLOC(HDRP(bp)) )
			remove_from_ll(bp);

		PUT(HDRP(bp), PACK(asize, 1));
		PUT(FTRP(bp), PACK(asize, 1));

		void*unallocatedBlock =  NEXT_BLKP(bp); //adjust new unallocated block
		PUT( HDRP(unallocatedBlock), PACK(csize, 0) ); // make new header of following unallocated space

		if ( !GET_ALLOC(HDRP(NEXT_BLKP(unallocatedBlock)))  ) //Checks if boardering block is unallocated and coallescs
		{
			csize += GET_SIZE(  HDRP( NEXT_BLKP(unallocatedBlock) )  );
			PUT( HDRP(unallocatedBlock), PACK(csize, 0) );
			PUT( FTRP(unallocatedBlock), PACK(csize, 0) );
		}

		else{ //  just makes new footer
			PUT( FTRP(unallocatedBlock), PACK(csize, 0) ); // make new footer	
		}

		add_to_end_ll(unallocatedBlock);
	}

	else if( GET( bp+DSIZE ) == 1010101010 )
	{
	    if(!GET_ALLOC(HDRP(bp)) ) //removes from ll then allocates
		remove_from_ll(bp);
	    PUT(HDRP(bp), PACK(bsize, 1));
	    PUT(FTRP(bp), PACK(bsize, 1));
	}

	else{//putting at end of stack.  Don't care
	    PUT(HDRP(bp), PACK(bsize, 1));
	    PUT(FTRP(bp), PACK(bsize, 1));
	}
}


/**********************************************************
 * mm_free
 * Free the block and coalesce with neighbouring blocks
 **********************************************************/
void mm_free(void *bp) //Add element to linked list
{
	bp = coalesce(bp);
	size_t size = GET_SIZE(HDRP(bp));

	    if( (size_t)UHead == GET(UTail)) //first free element
	    {
		PUT(UHead, (size_t)bp);
		PUT(UTail, (size_t)NEXT(bp));
		
		PUT( bp, (size_t)UTail);
		PUT( NEXT(bp), (size_t)UHead);
		PUT( bp+DSIZE , 1010101010 ); 

 		PUT(HDRP(bp), PACK(size,0));
	    	PUT(FTRP(bp), PACK(size,0));
		return;
	    }

	    else //place at end of linked list
	    {
		PUT( bp+WSIZE, GET(UTail)); 
		PUT( (GET(UTail)-WSIZE) , (size_t)bp );    
		PUT( bp, (size_t)UTail );
		PUT( UTail, (size_t)NEXT(bp));
		PUT( bp+DSIZE , 1010101010 ); 

 		PUT(HDRP(bp), PACK(size,0));
	    	PUT(FTRP(bp), PACK(size,0));
		return;
	    }    
}


/**********************************************************
 * mm_malloc
 * Allocate a block of size bytes.
 * The type of search is determined by find_fit
 * The decision of splitting the block, or not is determined
 *   in place(..)
 * If no block satisfies the request, the heap is extended
 **********************************************************/
void *mm_malloc(size_t size)
{

if(maxmem == 1)
{//initialize space for header and tail pointers
	maxmem++;
	UHead = mm_malloc(DSIZE);
	UTail = (UHead + WSIZE);
	PUT(UHead, (size_t)UTail);
	PUT(UTail, (size_t)UHead);
}

    size_t asize; /* adjusted block size */
    size_t extendsize; /* amount to extend heap if no fit */
    char * bp;
    
    /* Ignore spurious requests */
    if (size <= 0)
        return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= DSIZE)
        asize = DSIZE + OVERHEAD;
    else
        asize = DSIZE * ((size + (OVERHEAD) + (DSIZE-1))/ DSIZE);

    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }
    
    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    return bp;
}




/**********************************************************
 * mm_realloc
 * Implemented simply in terms of mm_malloc and mm_free
 *********************************************************/
void *mm_realloc(void *ptr, size_t size)
{
   void *oldptr = ptr;
//This doesn't work.  Unfortunately, it would have increased performance substantially
   /* size_t asize, oldsizeOH; //These have overhead accounted for
    size_t asize2, oldsize; //Overhead is removed

    if (size <= DSIZE)
        asize2 = DSIZE + OVERHEAD;
    else
        asize2 = DSIZE * ((size + (OVERHEAD) + (DSIZE-1))/ DSIZE);

    oldsizeOH = GET_SIZE(HDRP(oldptr));
    oldsize = oldsizeOH - OVERHEAD;
    asize = (asize2 - OVERHEAD);

	if(asize < oldsize) //shrink condition
	{
		size_t whiteSpace;
		void *unallocatedBlock;
		void *prevptr = PREV_BLKP(oldptr);
		void *newptr;

		if ( !GET_ALLOC(HDRP(prevptr))  ) //Want to move the reallocated block up in mem
		{
			if( GET( prevptr+DSIZE ) == 1010101010 )
				remove_from_ll(prevptr);
			size_t emptyBlockSize = GET_SIZE(HDRP(prevptr)); //contains buffer

			//Calculate the size of the unallocated memory to follow
			if(asize2 < emptyBlockSize) 
			{
				whiteSpace = emptyBlockSize - asize2;
				whiteSpace += oldsizeOH; 
			}

			else if (asize2 > emptyBlockSize)
			{
				whiteSpace = asize2 - emptyBlockSize;
				whiteSpace = (oldsizeOH - whiteSpace) ;
			}

			else if (asize2 == emptyBlockSize)
			{
				whiteSpace = oldsizeOH;
			}		


			if(whiteSpace < (WSIZE + DSIZE + OVERHEAD) )
			{
				asize2 = asize2 + whiteSpace;
				whiteSpace = 0;
			}

			PUT( HDRP(prevptr), PACK(asize2, 1) ); 
			memmove(prevptr, oldptr, size);       //Must memmove before making footer -- can write over memory if you don't
			PUT( FTRP(prevptr), PACK(asize2, 1) );
			
			if(!whiteSpace)
				return(prevptr);

			newptr = prevptr;
			unallocatedBlock =  NEXT_BLKP(prevptr); //adjust new unallocated block
		}

		else //just shrinking original block 
		{
			//Calculate size of unallocated block to follow
			whiteSpace = (oldsizeOH - asize2); //This takes into account overhead for unallocated space

			if(whiteSpace < (WSIZE + DSIZE + OVERHEAD) )
			{
				asize2 = asize2 + whiteSpace;
				whiteSpace = 0;
			}

			PUT( HDRP(oldptr), PACK(asize2, 1) ); //Pack in new size in header
			PUT( FTRP(oldptr), PACK(asize2, 1) ); //Pack in new size in new footer

			if(!whiteSpace)
				return(oldptr);

			newptr = oldptr; //return value
			unallocatedBlock =  NEXT_BLKP(oldptr);	
		}

		PUT( HDRP(unallocatedBlock), PACK(whiteSpace, 0) ); // make new header of following unallocated space

		if ( !GET_ALLOC(HDRP(NEXT_BLKP(unallocatedBlock)))  ) //Checks if boardering block is unallocated and coallescs
		{
			whiteSpace += GET_SIZE(  HDRP( NEXT_BLKP(unallocatedBlock) )  );
			PUT( HDRP(unallocatedBlock), PACK(whiteSpace, 0) );
			PUT( FTRP(unallocatedBlock), PACK(whiteSpace, 0) );
		}

		else{ //  just makes new footer
			PUT( FTRP(unallocatedBlock), PACK(whiteSpace, 0) ); // make new footer		
		}
		add_to_end_ll(unallocatedBlock);
		return(newptr);
    	}

	else if(asize > oldsize) //grow
	{
		void *prevptr = PREV_BLKP(oldptr);
		void *nextptr = NEXT_BLKP(oldptr);
		size_t difference = asize2 - oldsizeOH; //Amount of space required to grow

		size_t whiteSpace;
		void *unallocatedBlock;


if (  ( !GET_ALLOC(HDRP(prevptr)) && difference <= GET_SIZE(HDRP(prevptr)) ) || (( !GET_ALLOC(HDRP(prevptr)) && !GET_ALLOC(HDRP(nextptr)) ) &&  (difference <= (  GET_SIZE(HDRP(prevptr)) + GET_SIZE(HDRP(nextptr))  )) )  )  
//This hurts my head.  It was a bitch to write, it should be a bitch to read
// Checks previous space if unallocated  AND  checks if enough new space exists    OR   Checks if previous AND next spaces are unallocated  AND if enough room between the two
		{

			if(!GET_ALLOC(HDRP(prevptr)) && GET( prevptr+DSIZE ) == 1010101010 )
				remove_from_ll(prevptr);
			if(!GET_ALLOC(HDRP(nextptr)) && GET( nextptr+DSIZE ) == 1010101010 )
				remove_from_ll(nextptr);

			size_t totalSize;
			if ( difference <= GET_SIZE(HDRP(prevptr)) ) //can move entirely into prev adjacent space
			{
				totalSize = ( GET_SIZE(HDRP(prevptr)) + oldsizeOH ); // contains buffer space
				whiteSpace = totalSize - asize2; //contains overhead for buffers
			}

			else //if ( !GET_ALLOC(HDRP(nextptr))  &&   difference <= (  GET_SIZE(HDRP(prevptr)) + GET_SIZE(HDRP(nextptr))  )  )
			{//can move into combination of prev & next adjacent spaces
				totalSize = ( GET_SIZE(HDRP(prevptr)) + GET_SIZE(HDRP(nextptr)) + oldsizeOH ); // contains buffer space
				whiteSpace = totalSize - asize2; //contains overhead for buffers
			}

			if(whiteSpace < (WSIZE + DSIZE + OVERHEAD) )
			{
				asize2 = totalSize;
				whiteSpace = 0;
			}
		
			PUT( HDRP(prevptr), PACK(asize2, 1) ); 
			memmove( prevptr, oldptr, GET_SIZE(HDRP(oldptr)) );//Must memmove before making footer -- else can overwrite memory
			PUT( FTRP(prevptr), PACK(asize2, 1) );

			if(!whiteSpace)
				return(prevptr);
			
			unallocatedBlock =  NEXT_BLKP(prevptr); //adjust new unallocated block
			PUT( HDRP(unallocatedBlock), PACK(whiteSpace, 0) ); // make new header of following unallocated space

			if ( !GET_ALLOC(HDRP(NEXT_BLKP(unallocatedBlock)))  ) //Checks if boardering block is unallocated and coallescs
			{
				whiteSpace += GET_SIZE(  HDRP( NEXT_BLKP(unallocatedBlock) )  );
				PUT( HDRP(unallocatedBlock), PACK(whiteSpace, 0) );
				PUT( FTRP(unallocatedBlock), PACK(whiteSpace, 0) );
			}

			else{ //  just makes new footer
				PUT( FTRP(unallocatedBlock), PACK(whiteSpace, 0) ); // make new footer		
			}

			add_to_end_ll(unallocatedBlock);
			return(prevptr);
		}

	        else if (  ( !GET_ALLOC(HDRP(nextptr)) ) && ( difference <= GET_SIZE(HDRP(nextptr)) )  )//can move into next adjacent space
		{
			size_t totalSize = ( GET_SIZE(HDRP(nextptr)) + oldsizeOH ); // contains buffer space
			whiteSpace = totalSize - asize2; //contains overhead for buffers

			if(!GET_ALLOC(HDRP(nextptr)) && GET( nextptr+DSIZE ) == 1010101010 )
				remove_from_ll(nextptr);

			if(whiteSpace < (WSIZE + DSIZE + OVERHEAD) )
			{
				asize2 = totalSize;
				whiteSpace = 0;
			}
			
			PUT( HDRP(oldptr), PACK(asize2, 1) ); //Pack in new size in header
			PUT( FTRP(oldptr), PACK(asize2, 1) ); //Pack in new size in new footer

			if(!whiteSpace)
				return(oldptr);

			unallocatedBlock =  NEXT_BLKP(oldptr);
			PUT( HDRP(unallocatedBlock), PACK(whiteSpace, 0) ); // make new header of following unallocated space

			if ( !GET_ALLOC(HDRP(NEXT_BLKP(unallocatedBlock)))  ) //Checks if boardering block is unallocated and coallescs
			{
				whiteSpace += GET_SIZE(  HDRP( NEXT_BLKP(unallocatedBlock) )  );
				PUT( HDRP(unallocatedBlock), PACK(whiteSpace, 0) );
				PUT( FTRP(unallocatedBlock), PACK(whiteSpace, 0) );
			}

			else{ //  just makes new footer
				PUT( FTRP(unallocatedBlock), PACK(whiteSpace, 0) ); // make new footer		
			}

			add_to_end_ll(unallocatedBlock);
			return(oldptr);
		}
	}

	else if(size == oldsize)
	{
		return(oldptr);
	}
	else{ }*/
	size_t copySize;
	void *newptr;
    
	newptr = mm_malloc(size);
	if (newptr == NULL)
		return NULL;
	
	copySize = GET_SIZE(HDRP(oldptr));
	if (size < copySize)
		copySize = size;
	memcpy(newptr, oldptr, copySize);
	mm_free(oldptr);
	return newptr;
}


/**********************************************************
 * mm_check
 * Implemented simply to check free vs free
 *********************************************************/

int mm_check(void)
{

int notFree = 0;
int free = 0;
int totalFree = 0;

void *bp;
	for (bp = (void *)GET(UHead); GET_SIZE(HDRP(bp)) > 0 && bp != UTail; bp = (void *)GET(bp)) 
//goes through linked list and checks the number of free elements and allocated elements
	{
        	if (GET_ALLOC(HDRP(bp)) )
        	{
        	    notFree++;
        	}
		if (!GET_ALLOC(HDRP(bp)) )
        	{
        	    free++;
        	}
    	}

    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
//goes through entire heap and checks the number of free elements
        if (!GET_ALLOC(HDRP(bp)) )
        {
          totalFree++;
        }
    }

	if(totalFree != free) //compares total number of free elements with free elements in linked list
	{
		printf("There are not an equal number of free elements\n");
	}

	if(notFree)//should be 0
	{
		printf("There allocated elements in linked list\n");
	}
    return;
}






