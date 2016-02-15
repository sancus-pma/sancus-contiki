    .global __sm_lcd_mmio_public_start
    .global __sm_lcd_mmio_public_end
    .global __sm_lcd_mmio_secret_start
    .global __sm_lcd_mmio_secret_end
    .global __sm_lcd_mmio_entry
    .global __sm_lcd_mmio_entry_lcd_mmio_init_idx
    .global __sm_lcd_mmio_entry_lcd_mmio_write_byte_idx

    .section .sm.lcd_mmio.text, "ax"
__sm_lcd_mmio_public_start:
__sm_lcd_mmio_entry:
    ; Make sure we don't overwrite r15 because we need it in lcd_mmio_write_byte
    mov r15, r14
    .word 0x1387
    cmp #4, r15
    jne .Lexit
    cmp #__sm_lcd_mmio_entry_lcd_mmio_init_idx, r6
    jeq lcd_mmio_init
    cmp #__sm_lcd_mmio_entry_lcd_mmio_write_byte_idx, r6
    jeq lcd_mmio_write_byte
    jmp .Lexit

lcd_mmio_init:
    # See pmodcls_init()
    mov.w #2083, &0x0092
    mov.b #1,    &0x0090
    jmp .Lexit

lcd_mmio_write_byte:
    # See lcd_uart_write_byte()
1:
    mov.b &0x0091, r15
    and   #8,      r15
    tst   r15
    jnz   1b
    mov.b r14,     &0x0094
    # This delays about 20000 cycles.
    mov.w #6666,   r14
1:
    dec   r14
    tst   r14
    jnz   1b

.Lexit:
    mov.w #0xffff, r6
    br r7

__sm_lcd_mmio_public_end:

__sm_lcd_mmio_secret_start = 0x0090
__sm_lcd_mmio_secret_end = 0x0096
__sm_lcd_mmio_entry_lcd_mmio_init_idx = 0
__sm_lcd_mmio_entry_lcd_mmio_write_byte_idx = 1
