; Flag test (Carry and Zero)
; EXPECT AX=0000
; EXPECT CF=1
; EXPECT ZF=1

mov ax, 0FFFFh
add ax, 1
hlt
