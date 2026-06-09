; EXPECT BX=0000h
; EXPECT CF=1
; EXPECT ZF=1
; EXPECT OF=0
; EXPECT SF=0

; Test Carry (CF) and Zero (ZF) on max unsigned wrap
mov bx, 0FFFFh
add bx, 1       ; Wraps to 0000h. CF=1, ZF=1, OF=0
hlt
