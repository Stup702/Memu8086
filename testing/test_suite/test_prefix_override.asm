; EXPECT AX=1234h

; Multiple Segment Prefix Override Test
; When multiple segment prefixes are used, the last one before the opcode wins.
; The assembler allows chaining them if written in sequence (e.g. es: ds: [bx]).

mov ax, 0
mov ds, ax
mov cx, 0FFFFh
mov es, cx

; Setup distinct values in different segments
mov word [0100h], 1234h        ; DS:0100h = 1234h
mov word es:[0100h], 5678h     ; ES:0100h = 5678h

mov bx, 0100h

; Write the instruction manually to ensure ES then DS prefixes
; 26 = ES:, 3E = DS:, 8B 07 = MOV AX, [BX]
db 026h, 03Eh, 08Bh, 007h

; The last prefix (DS, 3Eh) should win, so AX = 1234h.

hlt
