; Test DAA boundary conditions
; DAA should handle carrying across both nibbles

.MODEL SMALL
.STACK 100h

.CODE
MAIN PROC
    ; 99 + 1 = 100 in BCD (which is 00 with carry)
    MOV AL, 99h
    ADD AL, 01h     ; AL = 9Ah, AF = 0
    DAA             ; AL = 00h, CF = 1, AF = 1
    MOV DL, AL
    
    ; Also test 90 + 90 = 180 (80 with carry)
    MOV AL, 90h
    ADD AL, 90h     ; AL = 20h, CF = 1, AF = 0
    DAA             ; AL = 80h, CF = 1
    MOV DH, AL

    ; EXPECT DX=8000h

    HLT
MAIN ENDP
END MAIN
