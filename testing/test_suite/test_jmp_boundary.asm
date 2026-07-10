; Test short jump boundaries
; Verifies that -128 backward and +127 forward jump limits work correctly

.MODEL SMALL
.STACK 100h

.CODE
MAIN PROC
    MOV AX, @DATA
    MOV DS, AX

    ; Initial state
    MOV AX, 0000h

    ; Forward jump of exactly 127 bytes from the END of the JMP instruction
    ; JMP short is 2 bytes. 
    ; If the jump is exactly 127 bytes, it should compile as short.
    ; If it compiles as near, it's 3 bytes.
    ; We can force a specific number of bytes by using DUP.
    JMP fwd_target

    DB 127 DUP (90h)

fwd_target:
    MOV AX, 1111h

    ; Backward jump of exactly 128 bytes from the END of the JMP instruction
    ; The backward jump itself takes 2 bytes.
    ; The target is 126 bytes of NOPs + 2 bytes of MOV = 128 bytes backward.
    JMP end_test

bwd_target:
    MOV AX, 2222h
    JMP end_test

    DB 124 DUP (90h)
    JMP bwd_target

end_test:
    ; EXPECT AX=1111h
    HLT
MAIN ENDP
END MAIN
