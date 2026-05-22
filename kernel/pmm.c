#include "pmm.h"
#include "pro_os.h"
#include <string.h>
static uint8_t* bm; static size_t tp;
void pmm_init(struct limine_memmap_response* m) {
    uint64_t hi=0; for(uint64_t i=0;i<m->entry_count;i++) if(m->entries[i]->type==LIMINE_MEMMAP_USABLE){ uint64_t t=m->entries[i]->base+m->entries[i]->length; if(t>hi) hi=t; }
    tp=hi/4096; size_t bs=tp/8;
    for(uint64_t i=0;i<m->entry_count;i++) if(m->entries[i]->type==LIMINE_MEMMAP_USABLE && m->entries[i]->length>=bs){ bm=(uint8_t*)(m->entries[i]->base+hhdm_offset); memset(bm,0xFF,bs); m->entries[i]->base+=bs; m->entries[i]->length-=bs; break; }
    for(uint64_t i=0;i<m->entry_count;i++) if(m->entries[i]->type==LIMINE_MEMMAP_USABLE) for(uint64_t a=m->entries[i]->base;a<m->entries[i]->base+m->entries[i]->length;a+=4096) bm[(a/4096)/8]&=~(1<<( (a/4096)%8 ));
}
void* pmm_alloc(size_t p) {
    size_t c=0; for(size_t i=0;i<tp;i++) if(!(bm[i/8]&(1<<(i%8)))){ if(++c==p){ size_t s=i-p+1; for(size_t j=s;j<=i;j++) bm[j/8]|=(1<<(j%8)); return (void*)(uintptr_t)(s*4096); } } else c=0;
    return 0;
}
void pmm_free(void* ptr, size_t p) { size_t s=(uintptr_t)ptr/4096; for(size_t i=s;i<s+p;i++) bm[i/8]&=~(1<<(i%8)); }
