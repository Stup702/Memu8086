; Test IMUL boundary conditions
; Tests -128 * -1 which is a classic boundary case

.MODEL SMALL
.STACK 100h

.CODE
MAIN PROC
    MOV AX, @DATA
    MOV DS, AX

    ; -128 (80h) * -1 (FFh)
    MOV AL, 80h
    MOV BL, 0FFh
    IMUL BL         ; AX = 0080h (128). 
                    ; Lower half is 80h (which is negative if interpreted as 8-bit).
                    ; Since 128 doesn't fit in 8-bit signed (-128 to 127), OF and CF must be set.
    
    ; EXPECT AX=0080h
    
    PUSHF
    POP CX
    
    ; We need to check if CF and OF are set.
    ; Since we can't easily EXPECT bits without masking, we'll just use a conditional jump
    ; to load a value.
    MOV DX, 0000h
    JO of_set
    JMP test_done
    
of_set:
    MOV DX, 1111h

test_done:
    ; EXPECT DX=1111h
    HLT
MAIN ENDP
END MAIN
