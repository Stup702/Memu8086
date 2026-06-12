; EXPECT DX=FFFEh
; EXPECT BX=FFFCh

; Test PUSH SP consistency
mov ax, 0
mov ss, ax
mov sp, 0FFFEh

mov dx, sp
; FF F4 is PUSH SP
db 0FFh, 0F4h
pop bx 

hlt
