; Test DIV by zero exception handling
; Should trigger interrupt 0 (Division by zero)

.MODEL SMALL
.STACK 100h

.CODE
MAIN PROC
    MOV AX, @DATA
    MOV DS, AX
    MOV ES, AX

    ; Install our own interrupt 0 handler
    MOV AX, 0000h
    MOV ES, AX          ; ES points to IVT (segment 0)
    
    ; Setup IVT entry for INT 0 (offset 0x0000)
    LEA AX, int0_handler
    MOV WORD PTR ES:[0000h], AX
    MOV AX, CS
    MOV WORD PTR ES:[0002h], AX
    
    ; Trigger division by zero
    MOV AX, 1000h
    MOV CX, 0000h
    DIV CX              ; Should trigger INT 0

    ; If we returned here, the handler executed!
    ; BUT INT 0 handler typically doesn't return to the next instruction safely 
    ; without adjusting IP, because the faulting instruction's IP was pushed.
    ; Wait, our emulator pushes the IP *before* advancing it?
    ; In real 8086, it pushes the IP of the instruction that caused the fault!
    
    MOV BX, 5555h
    HLT

int0_handler:
    MOV DX, 1234h
    ; EXPECT DX=1234h
    HLT                 ; Just halt inside the handler to keep it simple

MAIN ENDP
END MAIN
