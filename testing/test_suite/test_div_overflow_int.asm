; Test DIV overflow exception handling
; Should trigger interrupt 0 (Division by zero/overflow)

.MODEL SMALL
.STACK 100h

.CODE
MAIN PROC
    MOV AX, @DATA
    MOV DS, AX
    MOV ES, AX

    ; Install our own interrupt 0 handler
    MOV AX, 0000h
    MOV ES, AX          
    LEA AX, int0_handler
    MOV WORD PTR ES:[0000h], AX
    MOV AX, CS
    MOV WORD PTR ES:[0002h], AX
    
    ; Trigger division overflow (1000h / 1 in 8-bit mode)
    ; DX:AX is not used for 8-bit, just AX
    MOV AX, 1000h
    MOV CL, 01h
    DIV CL              ; 1000h / 1 = 1000h > FFh. Should trigger INT 0!

    MOV BX, 5555h
    HLT

int0_handler:
    MOV DX, 9999h
    ; EXPECT DX=9999h
    HLT                 

MAIN ENDP
END MAIN
