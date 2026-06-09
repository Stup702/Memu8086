; EXPECT AX=FFFAh

; Test IMUL (Signed Multiply)
mov al, 0FEh    ; -2
mov bl, 0003h    ; 3
imul bl         ; AX = -6 (FFFAh)
hlt
