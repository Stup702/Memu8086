; EXPECT AX=ABCDh
; EXPECT BX=0010h
; EXPECT CX=000Fh
; EXPECT DX=ABCDh

mov aX, 0aBcDh  ; Mixed case and requires leading zero for hex starting with a letter
mov bx, 16d     ; Explicit decimal suffix
mov cx, 1111b   ; Binary suffix
MOV DX, ax;No space before comment
hlt