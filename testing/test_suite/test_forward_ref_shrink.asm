; Test cascading jump shrinkage and assembler phase stability
; If the assembler doesn't handle the sizes properly, the jumps will land in the middle of instructions.

.MODEL SMALL
.STACK 100h
.CODE
MAIN PROC
    MOV AX, @DATA
    MOV DS, AX

    ; Initial state
    MOV AX, 0000h
    
    ; JMP1 is a forward reference to JMP2
    JMP jump2
    
    ; Filler to ensure this jump would be a near jump if evaluated wrong, 
    ; but is a short jump in reality (distance < 127)
    DB 100 DUP (90h) ; 100 NOPs

jump2:
    JMP jump3
    
    DB 100 DUP (90h) ; 100 NOPs
    
jump3:
    JMP end_jump
    
    DB 100 DUP (90h) ; 100 NOPs

end_jump:
    ; If we land here correctly, AX will become 1234h.
    ; If the offsets are wrong, we land in NOPs or garbage.
    MOV BX, 1234h
    ; EXPECT BX=1234h
    
    ; Exit
    MOV AH, 4ch
    INT 21h
MAIN ENDP
END MAIN
