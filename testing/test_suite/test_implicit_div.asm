; EXPECT AX=0005h
; EXPECT DX=0002h

mov ax, 0011h   ; Dividend lower half (17 decimal)
mov dx, 0000h   ; Dividend upper half (MUST be initialized)
mov bx, 0003h   ; Divisor (3 decimal)
div bx          ; 17 / 3 = 5 (AX), Remainder = 2 (DX)
hlt