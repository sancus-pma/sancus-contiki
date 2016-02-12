    .global __sm_port1_mmio_public_start
    .global __sm_port1_mmio_public_end
    .global __sm_port1_mmio_secret_start
    .global __sm_port1_mmio_secret_end
    .global __sm_port1_mmio_entry
    .global __sm_port1_mmio_entry_port1_mmio_read_idx
    .global __sm_port1_mmio_entry_port1_mmio_write_idx

    .section .sm.port1_mmio.text, "ax"
__sm_port1_mmio_public_start:
__sm_port1_mmio_entry:
    .word 0x1387
    cmp #2, r15
    jne .Lexit
    cmp #__sm_port1_mmio_entry_port1_mmio_read_idx, r6
    jeq port1_mmio_read
    cmp #__sm_port1_mmio_entry_port1_mmio_write_idx, r6
    jeq port1_mmio_write
    jmp .Lexit

port1_mmio_read:
    mov.w &0x0020, r12
    mov.w &0x0022, r13
    mov.w &0x0024, r14
    mov.w &0x0026, r15
    jmp .Lexit

port1_mmio_write:
    mov.w r12, &0x0020
    mov.w r13, &0x0022
    mov.w r14, &0x0024
    mov.w r15, &0x0026

.Lexit:
    mov.w #0xffff, r6
    br r7

__sm_port1_mmio_public_end:

__sm_port1_mmio_secret_start = 0x0020
__sm_port1_mmio_secret_end = 0x0028
__sm_port1_mmio_entry_port1_mmio_read_idx = 0
__sm_port1_mmio_entry_port1_mmio_write_idx = 1
