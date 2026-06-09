; EXPECT AX=1337h
; EXPECT DX=0001h

; Set up INT 0 (Divide by Zero) handler
mov ax, 0
mov es, ax
mov word es:[0000h], offset handler
mov word es:[0002h], 0000h

; Trigger Divide by Zero
mov ax, 10
mov bl, 0
div bl ; Triggers INT 0

; Return here
mov dx, 1
hlt

handler:
    mov ax, 1337h
    iret
