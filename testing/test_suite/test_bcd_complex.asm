; EXPECT AX=0012h
; EXPECT BX=0009h
; EXPECT CX=0102h
; EXPECT DX=0008h

; Test DAA (Addition Adjustment)
mov al, 09h
add al, 03h     ; AL = 0Ch
daa             ; AL = 12h
mov ah, 0
mov si, ax      ; SI = 0012h

; Test DAS (Subtraction Adjustment)
mov al, 12h
sub al, 03h     ; AL = 0Fh
das             ; AL = 09h
mov ah, 0
mov bx, ax      ; BX = 0009h

; Test AAA (ASCII Adjust after Addition)
mov ax, 0009h
add al, 03h     ; AL = 0Ch, AH = 0
aaa             ; AL = 02h, AH = 1
mov cx, ax      ; CX = 0102h

; Test AAS (ASCII Adjust after Subtraction)
mov ax, 0102h
sub al, 04h     ; AL = FEh (-2), AH = 1
aas             ; AL = 08h, AH = 0
mov dx, ax      ; DX = 0008h

mov ax, si      ; AX = 0012h (Match EXPECT AX)
hlt
