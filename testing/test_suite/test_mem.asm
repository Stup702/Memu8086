; Memory write test
; EXPECT MEM[500h]=42h

mov ax, 0
mov ds, ax
mov bx, 500h
mov byte ptr [bx], 42h
hlt
