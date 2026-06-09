; EXPECT CX=0000h
; EXPECT AX=0001h

mov cx, 0001h
mov ax, 0000h

start_loop:
inc ax
loop start_loop  ; CX becomes 0, jump is NOT taken. AX=1.

hlt