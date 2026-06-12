; EXPECT AX=1337h

; DIV Overflow Test (INT 0)
; If the quotient of a division is too large to fit in the destination register,
; the 8086 generates an INT 0 (Divide by Zero) interrupt.

mov ax, 0
mov ds, ax
mov es, ax
mov ss, ax
mov sp, 0FFFEh

; Set up INT 0 (Divide Error) handler
mov word [0000h], offset handler
mov word [0002h], 0000h

; Trigger DIV Overflow
mov ax, 0200h  ; 512
mov bl, 2
div bl         ; 512 / 2 = 256. Overflow! Triggers INT 0

; Return here
mov bx, 0
hlt

handler:
    mov ax, 1337h
    iret
