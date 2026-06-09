; Basic MOV and ADD test
; EXPECT AX=0003
; EXPECT BX=0002
; EXPECT CF=0
; EXPECT ZF=0

mov ax, 1
mov bx, 2
add ax, bx
hlt
