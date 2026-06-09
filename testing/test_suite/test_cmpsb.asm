; EXPECT SI=0502h
; EXPECT DI=1002h
; EXPECT ZF=1

mov ax, 0
mov ds, ax
mov es, ax

; Initialize data
mov si, 0500h
mov di, 1000h
mov byte [si], 041h
mov byte [si+1], 042h
mov byte [di], 041h
mov byte [di+1], 042h

cld
mov cx, 2
repe cmpsb ; Should match both bytes
; SI=0502h, DI=1002h, CX=0, ZF=1

hlt
