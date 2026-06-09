; EXPECT AX=1234h
; EXPECT CX=1234h
; EXPECT DX=1234h
; EXPECT SI=1234h
; EXPECT DI=1234h
; EXPECT BP=1234h

; Stress Test: Advanced Addressing Modes
mov ax, 0
mov ds, ax
mov ss, ax
mov sp, 0FFFEh
mov bp, 0F000h

mov word [0100h], 1234h
mov bx, 0100h
mov si, 0000h
mov di, 0000h

; Different ways to access the same value
mov ax, [bx]            ; Simple BX
mov cx, [bx+si]         ; BX+SI
mov dx, [bx+di]         ; BX+DI
mov si, [bx+0]          ; BX + 8-bit disp
mov di, [bx+0000h]      ; BX + 16-bit disp

; Using BP (SS segment)
mov word [0F000h], 1234h
mov bp, 0F000h
mov bp, [bp]            ; BP indirect (defaults to SS, which is 0)

hlt
