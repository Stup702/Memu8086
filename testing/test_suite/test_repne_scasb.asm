; Test REPNE SCASB
; Should scan until it finds the target byte in AL

.MODEL SMALL
.STACK 100h

.DATA
    str1 DB 'Hello, World!'

.CODE
MAIN PROC
    MOV AX, @DATA
    MOV DS, AX
    MOV ES, AX

    LEA DI, str1
    MOV CX, 13
    MOV AL, 'W'
    
    ; Scan for 'W' (which is the 8th character, offset 7)
    REPNE SCASB
    
    ; After 'W', it matches and exits.
    ; First 7 characters are NOT 'W', so CX decrements 7 times.
    ; The 8th character IS 'W', so CX decrements once more and then exits.
    ; Total decrements = 8.
    ; CX = 13 - 8 = 5.
    
    ; EXPECT CX=0005h
    ; EXPECT DI=0008h
    ; EXPECT ZF=1
    
    HLT
MAIN ENDP
END MAIN
