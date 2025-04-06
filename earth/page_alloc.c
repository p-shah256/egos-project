/* Author: CS6640 23fall staff
 * Description: a simple memory allocator
 */

#include "egos.h"
#include "string.h"

/* manage memory pages of:
 *    [FREE_MEM_START, FREE_MEM_END)
 * */
#define REMOVED_PAGES 900
/* original NPAGES: 1024-4=1020 */
#define NPAGES (((FREE_MEM_END - FREE_MEM_START) / PAGE_SIZE) - REMOVED_PAGES)

#define ROUNDUP(x,y) (x%y==0? x/y : (x+y-1)/y)

/* a bit map to track page availability
 * 0 for free; 1 for used
 */
unsigned char bitmap[ROUNDUP(NPAGES,8)];

/* bitmap functions
 */
void bitmap_set(int i) {
    bitmap[i/8] |= (1 << (i%8));
}

void bitmap_clear(int i) {
    bitmap[i/8] &= ~(1 << (i%8));
}

int bitmap_test(int i) {
    return bitmap[i/8] & (1 << (i%8));
}

/* memory allocator interfaces
 */

void *pageid2paddr(int pageid) {
    ASSERT(pageid < NPAGES, "pageid2paddr: pageid is too large");
    return (void *) (FREE_MEM_START + pageid * PAGE_SIZE);
}

int paddr2pageid(void *paddr) {
    ASSERT( (unsigned int)paddr % PAGE_SIZE == 0, "paddr2pageid: paddr is not page aligned");
    int pageid = ((unsigned int)paddr - FREE_MEM_START) / PAGE_SIZE;
    ASSERT(pageid < NPAGES, "paddr2pageid: pageid is too large");
    return pageid;
}

int tmp_test(int page_id) {
    return bitmap_test(page_id);
}

void* pmalloc(int clear_page) {
    for (int i = 0; i < NPAGES; i++) {
        if (bitmap_test(i) == 0) {
            bitmap_set(i);
            void *ret = pageid2paddr(i);
            if (clear_page) {
                memset((char*)ret, 0, PAGE_SIZE);
            }
            return ret;
        }
    }
    FATAL("pmalloc: no more available page");
}

void pfree(void *paddr) {
    int pageid = paddr2pageid(paddr);
    bitmap_clear(pageid);
}


