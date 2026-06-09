; EXPECT AX=8002h
; EXPECT CF=1

mov ax, 000Bh   ; 0000 0000 0000 1011b (11)
shr ax, 1       ; AX = 0005h, CF = 1
rcr ax, 1       ; CF(1) goes to MSB, LSB(1) goes to CF. AX = 8002h, CF = 1
hlt