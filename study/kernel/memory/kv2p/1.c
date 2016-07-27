/*
 * Physical vs virtual RAM address space conversion.  These are
 * private definitions which should NOT be used outside memory.h
 * files.  Use virt_to_phys/phys_to_virt/__pa/__va instead.
 */
#define __virt_to_phys(x) ({                        \
    phys_addr_t __x = (phys_addr_t)(x);                 \
    __x & BIT(VA_BITS - 1) ? (__x & ~PAGE_OFFSET) + PHYS_OFFSET :   \
                 (__x - kimage_voffset); })

#define __phys_to_virt(x)       ((unsigned long)((x) - PHYS_OFFSET) | PAGE_OFFSET)
#define __phys_to_kimg(x)       ((unsigned long)((x) + kimage_voffset))

/*
 * Convert a page to/from a physical address
 */
#define page_to_phys(page)      (__pfn_to_phys(page_to_pfn(page)))
#define phys_to_page(phys)      (pfn_to_page(__phys_to_pfn(phys)))