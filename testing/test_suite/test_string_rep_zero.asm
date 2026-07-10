; Test REP with CX=0
; Should not execute the string instruction at all

.MODEL SMALL
.STACK 100h

.DATA
    var1 DB 11h
    var2 DB 22h

.CODE
MAIN PROC
    MOV AX, @DATA
    MOV DS, AX
    MOV ES, AX

    MOV SI, 0000h
    MOV DI, 0001h
    MOV CX, 0000h
    
    ; Should do nothing
    REP MOVSB
    
    ; Ensure SI and DI were not modified
    ; EXPECT SI=0000h
    ; EXPECT DI=0001h
    
    ; Ensure var2 was not overwritten (should still be 22h)
    MOV AL, [0001h]
    ; EXPECT AL=22h

    HLT
MAIN ENDP
END MAIN
