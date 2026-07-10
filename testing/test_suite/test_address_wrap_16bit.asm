; Test 16-bit effective address wrapping
; The 8086 wraps effective addresses around the 64K segment limit.

.MODEL SMALL
.STACK 100h

.DATA
    var1 DB 11h
    var2 DB 22h

.CODE
    ; Expected:
    ; BX = FFFFh
    ; SI = 0002h
    ; EA = FFFFh + 0002h = 10001h -> 0001h
    ; DS:0001h points to var2 (1 byte after var1)
    
MAIN PROC
    MOV AX, @DATA
    MOV DS, AX

    MOV BX, 0FFFFh
    MOV SI, 0002h
    
    MOV AL, [BX+SI]
    ; EXPECT AL=22h
    
    ; Test BP+DI wrapping
    MOV BP, 0FFFFh
    MOV DI, 0002h
    ; Ensure SS points to DATA for this test
    PUSH DS
    POP SS
    MOV AH, [BP+DI]
    ; EXPECT AH=22h

    HLT
MAIN ENDP
END MAIN
