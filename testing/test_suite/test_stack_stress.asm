; EXPECT AX=0001h
; EXPECT BX=0001h
; EXPECT CX=0001h
; EXPECT DX=0001h
; EXPECT SI=0001h
; EXPECT DI=0001h

mov ax, 0
mov ss, ax
mov sp, 0FFFEh

mov ax, 1
mov bx, 1
mov cx, 1
mov dx, 1
mov si, 1
mov di, 1

push ax
push bx
push cx
push dx
push si
push di

call nest1

pop di
pop si
pop dx
pop cx
pop bx
pop ax

hlt

nest1:
    push ax
    push bx
    push cx
    call nest2
    pop cx
    pop bx
    pop ax
    ret

nest2:
    push dx
    push si
    push di
    
    ; Large loop on stack
    mov cx, 10
p_loop:
    push cx
    loop p_loop
    
    mov cx, 10
pp_loop:
    pop dx
    loop pp_loop
    
    ; Restore original DX from register pressure
    mov dx, 1

    pop di
    pop si
    pop dx
    ret
