; EXPECT MEM[1000h]=01h
; EXPECT MEM[1001h]=02h
; EXPECT MEM[1002h]=03h
; EXPECT MEM[1003h]=04h
; EXPECT MEM[1004h]=05h

; Bubble Sort Implementation
; Sorts 5 bytes at [1000h]

mov ax, 0
mov ds, ax

; Initialize data
mov byte [1000h], 05h
mov byte [1001h], 03h
mov byte [1002h], 04h
mov byte [1003h], 01h
mov byte [1004h], 02h

mov cx, 5       ; N = 5
outer_loop:
    push cx
    mov si, 1000h
    mov cx, 4   ; N - 1 iterations
inner_loop:
    mov al, [si]
    mov bl, [si+1]
    cmp al, bl
    jbe no_swap
    ; Swap
    mov [si], bl
    mov [si+1], al
no_swap:
    inc si
    loop inner_loop
    pop cx
    loop outer_loop

hlt
