; EXPECT AX=1337h

; IDIV Overflow Test (INT 0)
; 8-bit IDIV of -128 / -1 = 128. 128 doesn't fit in 8-bit signed register (max 127).

mov ax, 0
mov ds, ax
mov es, ax
mov ss, ax
mov sp, 0FFFEh

; Set up INT 0 (Divide Error) handler
mov word [0000h], offset handler
mov word [0002h], 0000h

; Trigger IDIV Overflow
mov ax, 0FF80h ; -128
mov bl, 0FFh   ; -1
idiv bl        ; -128 / -1 = 128. Triggers INT 0

; Return here
mov bx, 0
hlt

handler:
    mov ax, 1337h
    iret
