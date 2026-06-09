; EXPECT AX=0037h

; Recursive Fibonacci(10)
; Fib(1)=1, Fib(2)=1, Fib(3)=2, Fib(4)=3, Fib(5)=5, Fib(6)=8, Fib(7)=13, Fib(8)=21, Fib(9)=34, Fib(10)=55 (37h)

mov ax, 0
mov ss, ax
mov sp, 0FFFEh

mov ax, 10      ; N = 10
call fib
hlt

fib:
    cmp ax, 2
    jbe fib_base
    push ax
    dec ax
    call fib    ; Fib(N-1)
    pop dx      ; DX = original N
    push ax     ; Save Fib(N-1)
    mov ax, dx
    sub ax, 2
    call fib    ; Fib(N-2)
    pop dx      ; DX = Fib(N-1)
    add ax, dx  ; AX = Fib(N-2) + Fib(N-1)
    ret
fib_base:
    mov ax, 1
    ret
