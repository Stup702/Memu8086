; EXPECT AX=FFFBh
; EXPECT BX=FFFFh
; EXPECT CF=1

mov ax, 5
mov bx, 0
sub ax, 10      ; 5 - 10 = FFFBh. CF=1 (borrow required)
sbb bx, 0       ; BX = 0 - 0 - CF(1) = FFFFh. CF=1
hlt