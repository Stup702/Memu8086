; EXPECT SI=00FDh
; EXPECT DI=01FDh
; EXPECT CX=0000h
; EXPECT DF=1

mov si, 00FFh
mov di, 01FFh
mov cx, 0002h   ; Loop 2 times
std             ; Set Direction Flag (DF=1)
rep movsb       ; Copies byte, decrements SI and DI by 1 per iteration
hlt