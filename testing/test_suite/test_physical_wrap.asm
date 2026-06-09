; EXPECT AX=1234h
; EXPECT BX=5678h

; Physical Address Wrap Test (8086 wraps at 1MB)
; Physical Address = (Seg << 4) + Offset

mov ax, 0
mov ds, ax

; 0xFFFF:0x0010 -> 0x100000 -> 0x00000
; 0xFFFF:0x0011 -> 0x100001 -> 0x00001
; 0xFFFF:0x0012 -> 0x100002 -> 0x00002

; Write to 0x00000 and 0x00001
mov word [0000h], 1234h
mov word [0002h], 5678h

; Read them back using wrapped segment
mov cx, 0xFFFF
mov ds, cx

mov ax, [0010h] ; Should be 1234h
mov bx, [0012h] ; Should be 5678h

hlt
