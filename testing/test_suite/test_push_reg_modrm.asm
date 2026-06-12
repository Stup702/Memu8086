; EXPECT AX=1234h
; EXPECT BX=1234h

mov ax, 01234h
mov si, 05678h

; FF F0 is PUSH AX
db 0FFh, 0F0h
pop bx

hlt
