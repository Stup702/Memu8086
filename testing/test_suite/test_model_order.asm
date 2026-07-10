; Test arbitrary segment order and multiple segment switches

.MODEL SMALL
.STACK 100h

.CODE
MAIN PROC
    JMP start

    ; Read a variable that's defined later in the file
start:
    MOV AX, @DATA
    MOV DS, AX
    MOV AX, var1
    
    ; Exit
    HLT
MAIN ENDP

.DATA
    var1 DW 9999h

.CODE
    ; Extra code block just to see if the assembler appends to the code segment correctly
    NOP
    NOP

.DATA
    var2 DW 1111h

.CODE
    ; Another code block
    ; EXPECT AX=9999h

END MAIN
