;
; boot.s -- Kernel start location. Also defines multiboot header.
;
;           Based on JamesM's kernel development tutorials.
;

MULTIBOOT_PAGE_ALIGN    equ 1<<0        ; Load kernel and modules on a page boundary.
MULTIBOOT_MEM_INFO      equ 1<<1        ; Provide your kernel with memory info.
MULTIBOOT_HEADER_MAGIC  equ 0x1BADB002  ; Multiboot Magic value.

MULTIBOOT_HEADER_FLAGS  equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEM_INFO
MULTIBOOT_CHECKSUM      equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

KERNEL_MAGIC		    equ	0x13265213  ; Our magic number.

[BITS 32]                       ; All instructions should be 32-bit.

[GLOBAL mboot]                  ; Make 'mboot' accessible from C.
[EXTERN code]                   ; Start of the '.text' section.
[EXTERN bss]                    ; Start of the .bss section.
[EXTERN end]                    ; End of the last loadable section.

; Multiboot header.
mboot:
    dd  MULTIBOOT_HEADER_MAGIC      ; GRUB will search for this value on each
                                ; 4-byte boundary in your kernel file.
    dd  MULTIBOOT_HEADER_FLAGS      ; How GRUB should load your file / settings.
    dd  MULTIBOOT_CHECKSUM          ; To ensure that the above values are correct.
    
    dd  mboot                   ; Location of this descriptor.
    dd  code                    ; Start of kernel '.text' (code) section.
    dd  bss                     ; End of kernel '.data' section.
    dd  end                     ; End of kernel.
    dd  start                   ; Kernel entry point (initial EIP).

[GLOBAL start]                  ; Kernel entry point.
[EXTERN main]                   ; This is the entry point of our C code.

start:
	; Check if data segment linked, located, and loaded properly.
	mov eax, [kernel_magic]
	cmp eax, KERNEL_MAGIC
	jne die

    push esp                    ; Pass the stack pointer.
    push ebx                    ; And the multiboot header to main().


    ; Execute the kernel:
    cli                         ; Disable interrupts.
    call main                   ; call our main() function.
    jmp $                       ; Enter an infinite loop, to stop the processor.
                                ; executing whatever rubbish is in the memory
                                ; after our kernel!
								
die:
	; Display a blinking white-on-blue 'D' and freeze.
    ; TODO: better die message...
	mov word [0B8000h],9F44h
	jmp short $

SECTION .data

kernel_magic:
	dd KERNEL_MAGIC

