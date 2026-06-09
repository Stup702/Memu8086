; EXPECT MEM[1000h]=0EEh
; EXPECT MEM[1001h]=0EEh
; EXPECT DI=1002h

mov ax, 0
mov es, ax

mov al, 0EEh
mov di, 1000h
mov cx, 2
rep stosb ; Write 0EEh to 1000h, 1001h
hlt
