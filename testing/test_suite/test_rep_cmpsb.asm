; Test REP CMPSB early exit
; Verifies that REP stops when ZF=0 (mismatch)

.MODEL SMALL
.STACK 100h

.DATA
    str1 DB 'Hello, World!'
    str2 DB 'Hello, xorld!'

.CODE
MAIN PROC
    MOV AX, @DATA
    MOV DS, AX
    MOV ES, AX

    LEA SI, str1
    LEA DI, str2
    MOV CX, 13
    
    ; Should match until 'x' (which is the 8th character, offset 7)
    ; CX should be decremented for every character compared
    REPE CMPSB
    
    ; After 'W' vs 'x', they don't match, so it exits.
    ; First 7 characters ('H', 'e', 'l', 'l', 'o', ',', ' ') MATCH.
    ; The 8th character ('W' vs 'x') MISMATCHES.
    ; CX starts at 13.
    ; 1. 'H' -> CX=12
    ; 2. 'e' -> CX=11
    ; 3. 'l' -> CX=10
    ; 4. 'l' -> CX=9
    ; 5. 'o' -> CX=8
    ; 6. ',' -> CX=7
    ; 7. ' ' -> CX=6
    ; 8. 'W' vs 'x' -> CX=5, ZF=0 -> exits REPE
    
    ; EXPECT CX=0005h
    ; EXPECT SI=0008h
    ; EXPECT DI=0015h  ; Because str2 starts at offset 13 (000Dh) + 8 = 21 (0015h)
    
    HLT
MAIN ENDP
END MAIN
