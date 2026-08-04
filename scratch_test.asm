; Test: MAIN first, PRINT_NUM second (user's original layout)
.MODEL small
.STACK 100H

.DATA
    dummy DW 0

.CODE
MAIN PROC
    MOV AX, @DATA
    MOV DS, AX

    MOV AX, 100
    CALL PRINT_NUM

    MOV AH, 4Ch
    INT 21h
MAIN ENDP

PRINT_NUM PROC
    PUSH AX
    PUSH BX
    PUSH CX
    PUSH DX

    MOV CX, 0
    MOV BX, 10

div_loop:
    MOV DX, 0
    DIV BX
    PUSH DX
    INC CX
    CMP AX, 0
    JNZ div_loop

prt_loop:
    POP DX
    ADD DL, '0'
    MOV AH, 02h
    INT 21h
    LOOP prt_loop

    POP DX
    POP CX
    POP BX
    POP AX
    RET
PRINT_NUM ENDP

END MAIN
; EXPECT AX=0064h
