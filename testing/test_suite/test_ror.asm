; EXPECT DX=8000h
; EXPECT CF=1
; EXPECT OF=1

; Test ROR (Rotate Right)
mov dx, 0001h
ror dx, 1       ; DX = 8000h, CF=1, OF=1
hlt
