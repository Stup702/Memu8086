; EXPECT AX=0073h
; EXPECT CF=0

mov al, 38h
add al, 35h     ; 38h + 35h = 6Dh (Not a valid BCD number)
daa             ; Adjusts AL. Lower nibble 'D' > 9, so it adds 6. 6Dh + 06h = 73h.
mov ah, 0       ; Clear AH to verify clean AX
hlt