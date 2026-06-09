; EXPECT SI=0504h
; EXPECT DI=1004h
; EXPECT MEM[1000h]=41h
; EXPECT MEM[1001h]=42h
; EXPECT MEM[1002h]=43h
; EXPECT MEM[1003h]=44h

mov ax, 0
mov ds, ax
mov es, ax

cld
mov si, 0500h
mov di, 1000h
mov byte [si], 041h
mov byte [si+1], 042h
mov byte [si+2], 043h
mov byte [si+3], 044h
mov cx, 4
rep movsb ; Copy 4 bytes to 1000h
hlt
