; EXPECT AX=000Ah
; EXPECT BX=0005h
; EXPECT DX=0001h

; Test CALL and RET
mov ax, 0
call my_sub
jmp skip_sub

my_sub:
    add ax, 5
    ret

skip_sub:
    call my_sub ; AX becomes 10 (0Ah)

; Test JCC (Jump if Condition Met)
mov bx, 5
cmp bx, 5
je its_equal
hlt ; Should not reach

its_equal:
    mov dx, 1

hlt
