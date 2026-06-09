; EXPECT AX=1111h
; EXPECT BX=2222h

; Setup segments
mov cx, 1000h
mov ds, cx
mov cx, 2000h
mov es, cx

; Write distinct values to the same offset in different segments
mov word [0050h], 1111h    ; Writes to DS:0050h (1000h:0050h)
mov word es:[0050h], 2222h ; Writes to ES:0050h (2000h:0050h)

; Read them back using segment overrides or defaults
mov bx, 0050h
mov ax, [bx]       ; Reads from DS:0050h (AX = 1111h)
mov bx, es:[bx]    ; Segment override! Reads from ES:0050h (BX = 2222h)
hlt
