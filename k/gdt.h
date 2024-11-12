#ifndef GDT_H
#define GDT_H

#include "types.h"

// GDT entry. To simplify things, the gdt entry is later encoded using
// encodeGdtEntry(). 
struct gdt_entry {
    u32 base;
    u32 limit;  // u32 is used as it is big enough to store the 20-bit limit. It is later checked that limit does not exceed that 20-bit size.
    u8 access_byte;
    u8 flags;   // u8 is used as it is big enough to store 4-bit flags.
}__attribute__((packed));

// GDT register.
struct gdtr {
    u16 size;
    u32 offset;
}__attribute__((packed));

void encodeGdtEntry(struct gdt_entry gdt_entry, u8 *encoded_gdt);

#endif /* !GDT_H */