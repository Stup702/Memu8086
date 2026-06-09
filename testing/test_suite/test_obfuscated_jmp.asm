; EXPECT AX=5555h
; EXPECT BX=AAAAh

; Control Flow Obfuscation
; Using PUSH and RET to jump

mov ax, 0
mov ss, ax
mov sp, 0FFFEh

mov ax, 0112h ; Target at 0112h
push ax

push 01234h
pop bx

ret
hlt

; Target at 0112h
mov ax, 05555h
mov bx, 0AAAAh
hlt
