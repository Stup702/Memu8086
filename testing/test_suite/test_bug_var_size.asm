; EXPECT AX=4C00h

.MODEL small

.STACK 200h

.DATA 

name_here DW ?

.CODE
MAIN PROC
MOV AX, @DATA
MOV DS, AX

MOV name_here, 2341h

	MOV AH, 4Ch    ; DOS interrupt code to terminate program
    INT 21h   
MAIN ENDP
END MAIN
