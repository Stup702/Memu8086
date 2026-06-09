; EXPECT AX=8000h
; EXPECT OF=1
; EXPECT CF=0

; Test SHL (Shift Left)
mov ax, 4000h
shl ax, 1       ; AX = 8000h, CF=0, OF=1
hlt
