; EXPECT SP=FFFEh
; EXPECT AX=1234h
; EXPECT BX=1234h

mov sp, 0000h
mov ax, 1234h
push ax         ; SP wraps to FFFEh
pop bx          ; Reads 1234h into BX, SP back to 0000h
push ax         ; SP back to FFFEh so the EXPECT catches it at the end
hlt