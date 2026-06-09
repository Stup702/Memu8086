; EXPECT AX=1337h
; EXPECT MEM[1000h]=01h
; EXPECT MEM[1001h]=02h
; EXPECT MEM[1002h]=03h
; EXPECT MEM[1003h]=05h
; EXPECT MEM[1004h]=08h

; THE ULTIMATE STRESS TEST

mov ax, 0
mov ds, ax
mov es, ax
mov ss, ax
mov sp, 0FFFEh

; 1. Set up INT 0 handler
mov word [0000h], offset done_handler
mov word [0002h], 0000h

; 2. Generate Fibonacci numbers into memory at 1000h
; Fib(2)=1, Fib(3)=2, Fib(4)=3, Fib(5)=5, Fib(6)=8
; We'll store them out of order then sort them.
mov ax, 6
call fib
mov [1000h], al ; 8

mov ax, 4
call fib
mov [1001h], al ; 3

mov ax, 5
call fib
mov [1002h], al ; 5

mov ax, 2
call fib
mov [1003h], al ; 1

mov ax, 3
call fib
mov [1004h], al ; 2

; 3. Sort the 5 bytes at 1000h
mov cx, 5
outer:
    push cx
    mov si, 1000h
    mov cx, 4
inner:
    mov al, [si]
    mov bl, [si+1]
    cmp al, bl
    jbe next
    mov [si], bl
    mov [si+1], al
next:
    inc si
    loop inner
    pop cx
    loop outer

; 4. Trigger Divide by Zero to finish
mov ax, 10
mov bl, 0
div bl

hlt ; Should be reached after IRET if IP was 0x1XXX? 
    ; Wait, the div instruction is at some IP. IRET returns to NEXT instruction.

fib:
    cmp ax, 2
    jbe fib_b
    push ax
    dec ax
    call fib
    pop dx
    push ax
    mov ax, dx
    sub ax, 2
    call fib
    pop dx
    add ax, dx
    ret
fib_b:
    mov ax, 1
    ret

done_handler:
    mov ax, 1337h
    iret
