; EXPECT AX=0000h
; EXPECT DX=0001h
; EXPECT CF=1
; EXPECT OF=1

mov ax, 8000h
mov bx, 2
mul bx          ; 8000h * 2 = 10000h. DX=0001h, AX=0000h
                ; Because DX != 0, CF=1 and OF=1
hlt