; EXPECT AX=8000h
; EXPECT OF=1
; EXPECT SF=1

; Test Signed Overflow (OF) and Sign Flag (SF)
mov ax, 7FFFh   ; Max signed 16-bit positive integer
add ax, 1       ; Pushes into negative territory (8000h). OF=1, SF=1
hlt
