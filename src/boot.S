#define MULTIBOOT_MAGIC 0x1BADB002
#define MODULE_ALIGN (1 << 0)
#define MEMORY_MAP (1 << 1)

#define FLAGS (MODULE_ALIGN | MEMORY_MAP)
#define CHECKSUM -(MULTIBOOT_MAGIC + FLAGS)

        .text
        .align 4
        .long MULTIBOOT_MAGIC
        .long FLAGS
        .long CHECKSUM

        .global boot
boot:   movb $48, 0xB8000
        hlt