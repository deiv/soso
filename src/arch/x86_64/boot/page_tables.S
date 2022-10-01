#include <memory.h>

.global boot_p4
.global boot_p3

.set ENTRIES_PER_TABLE, 512

.section .data
	.align PAGE_SIZE

boot_p4:
	.quad virtual_to_physical(boot_p3) + (PAGE_PRESENT | PAGE_WRITABLE)
	.rept ENTRIES_PER_TABLE - 2
	  .quad 0
	.endr
	.quad virtual_to_physical(boot_p3) + (PAGE_PRESENT | PAGE_WRITABLE)
boot_p3:
	.quad virtual_to_physical(boot_p2) + (PAGE_PRESENT | PAGE_WRITABLE)
	.rept ENTRIES_PER_TABLE - 3
	  .quad 0
	.endr
	.quad virtual_to_physical(boot_p2) + (PAGE_PRESENT | PAGE_WRITABLE)
	.quad 0
boot_p2:
	.quad virtual_to_physical(boot_p1) + (PAGE_PRESENT | PAGE_WRITABLE)
	.rept ENTRIES_PER_TABLE - 1
	  .quad 0
	.endr
boot_p1:
	.set i,0
	.rept ENTRIES_PER_TABLE
	  .quad (i << 12) + (PAGE_PRESENT | PAGE_WRITABLE)
	  .set i, (i+1)
	.endr

