#include "gdt.h"

void encodeGdtEntry(struct gdt_entry gdt_entry, u8 *encoded_gdt_entry) {
    // Ensure that limit does not exceed 20-bit size (see declaration of struct gdt_entry).
    if (gdt_entry.limit > 0xFFFFF) {
        err("GDT entry cannot have limit larger thatn 0xFFFFF (20 bits).");
    }

    // Encode limit.
    encoded_gdt_entry[0] = gdt_entry.limit & 0xFF;
    encoded_gdt_entry[1] = (gdt_entry.limit >> 8) & 0xFF;
    encoded_gdt_entry[6] = (gdt_entry.limit >> 16) & 0x0F;

    // Encode base.
    encoded_gdt_entry[2] = gdt_entry.base & 0xFF;
    encoded_gdt_entry[3] = (gdt_entry.base >> 8) & 0xFF;
    encoded_gdt_entry[4] = (gdt_entry.base >> 16) & 0xFF;
    encoded_gdt_entry[7] = (gdt_entry.base >> 24) & 0xFF;

    // Encode access byte.
    encoded_gdt_entry[5] = gdt_entry.access_byte;

    // Encode flags.
    encoded_gdt_entry[6] |= (gdt_entry.flags << 4); // Note: flags are stored in lower bits of non-encoded gdt_entry.
}

void setup_gdt_protected_flat_model(u64 *gdt) {
    struct gdt_entry null_descriptor = {
        .base = 0x0,
        .limit = 0x0,
        .access_byte = 0x0,
        .flags = 0x0
    };  
    struct gdt_entry kernel_code_segment = {
        .base = 0x0,
        .limit = 0xFFFFF,
        .access_byte = 0x9A,
        .flags = 0xC
    };  
    struct gdt_entry kernel_data_segment = {
        .base = 0x0,
        .limit = 0xFFFFF,
        .access_byte = 0x92,
        .flags = 0xC
    };  
    struct gdt_entry user_code_segment = {
        .base = 0x0,
        .limit = 0xFFFFF,
        .access_byte = 0xFA,
        .flags = 0xC
    };  
    struct gdt_entry user_data_segment = {
        .base = 0x0,
        .limit = 0xFFFFF,
        .access_byte = 0xF2,
        .flags = 0xC
    };
    // TODO: TSS.
    /*  
        struct gdt_entry tss = {
            .base = 0x0,
            .limit = 0x0,
            .access_byte = 0x0,
            .flags = 0x0
        };
    */

   // Disable interrupts.
   asm volatile ("cli");

   // Fill up table.
   encodeGdtEntry(null_descriptor, &gdt[0]);
   encodeGdtEntry(kernel_code_segment, &gdt[1]);
   encodeGdtEntry(kernel_data_segment, &gdt[2]);
   encodeGdtEntry(user_code_segment, &gdt[3]);
   encodeGdtEntry(user_data_segment, &gdt[4]);
}

void setup_and_load_gdt() {
    u64 gdt[5];
    setup_gdt_protected_flat_model(gdt);

    struct gdtr gdtr = {
        .size = sizeof(gdt) - 1,
        .offset = (u32)&gdt // Explicit cast to type expected by offset in 32-bit protected mode.
    };

	asm volatile ("lgdt %0" : /* No output */ : "r"(&gdtr));
}