.MODEL SMALL
.STACK 100H

.CODE
MAIN PROC
    MOV AX, 0FFFFH 
    MOV BX, 0001H
    ADD AX, BX     ; AX = FFFF + 0001 = 0000H. Carry Flag (CF) is set to 1.
    
    MOV CX, 0000H
    ADC CX, 0000H  

    MOV AX, 0005H
    MOV BX, 0008H
    SUB AX, BX     
    MOV DX, 0002H
    SBB DX, 0000H 
     
    MOV AX, 0005H
    INC AX         ; AX = AX + 1 = 0006H
    DEC AX         ; AX = AX - 1 = 0005H

    MOV BX, 0005H
    NEG BX         ; BX becomes FFFBH (-5 in 2's complement)

    MOV AH, 4CH
    INT 21H
MAIN ENDP
END MAIN