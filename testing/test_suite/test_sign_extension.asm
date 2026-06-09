; EXPECT AX=007Fh
; EXPECT DX=0000h
; EXPECT BX=007Fh
; EXPECT CX=0000h

; Test Negative Extension
mov al, 080h    ; -128 in two's complement
cbw             ; AX becomes FFFFh (Sign bit 7 was 1)
cwd             ; DX becomes FFFFh (Sign bit 15 was 1)

; Test Positive Extension
mov bl, 7Fh     ; 127 in two's complement
mov al, bl
cbw             ; AX becomes 007Fh (Sign bit 7 was 0)
mov bx, ax
cwd             ; DX becomes 0000h (Sign bit 15 was 0)
mov cx, dx
hlt
