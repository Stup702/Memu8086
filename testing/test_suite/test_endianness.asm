; EXPECT MEM[0200h]=34h
; EXPECT MEM[0201h]=12h
; EXPECT AX=1234h
; EXPECT BX=1234h

mov ax, 1234h
mov [0200h], ax      ; Little-endian write: 34h at 200h, 12h at 201h
mov bx, [0200h]      ; Should read back as 1234h natively
hlt