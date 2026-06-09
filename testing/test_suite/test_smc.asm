; EXPECT AX=1234h

; Self-Modifying Code (SMC) Test
; We will write a "MOV AX, 1234h" instruction into memory and execute it.
; MOV AX, 1234h is B8 34 12

mov ax, 0
mov ds, ax

; The target address for our new instruction
mov di, 0500h

; Write B8 34 12 (MOV AX, 1234h)
mov byte ptr [di], 0xB8
mov byte ptr [di+1], 0x34
mov byte ptr [di+2], 0x12
; Write F4 (HLT)
mov byte ptr [di+3], 0xF4

; Jump to the code we just created
jmp 0500h

hlt ; Should not be reached
