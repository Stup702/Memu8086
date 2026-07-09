; EXPECT BX=1234h
; EXPECT CX=000Ah
; EXPECT DX=0700h
; EXPECT SI=0BEEh
; EXPECT DI=5555h

.MODEL SMALL
.STACK 100h

.DATA
    var1 DW 0AAAAh
    var2 DW 1234h

.CODE
    MOV AX, @DATA
    MOV DS, AX
    
    ; Test variables in .DATA
    MOV BX, var2    ; Should be 1234h
    
    ; Test multiple segments mapping correctly. PSP is 0700h. ES should be PSP when loaded.
    MOV DX, ES      ; Should be 0700h

.DATA
    var3 DW 000Ah

.CODE
    ; Code block 2
    MOV CX, var3    ; Should be 000Ah
    
    ; Test SS calculation (Stack Segment)
    ; SS should be allocated dynamically after DATA.
    MOV AX, SS
    MOV BP, DS
    CMP AX, BP
    JA check_stack_ok
    MOV SI, 2989    ; 0BADh
    JMP end_test
check_stack_ok:
    MOV SI, 3054    ; 0BEEh
    
    ; Let's test actual stack push/pop
    MOV AX, 5555h
    PUSH AX
    POP DI          ; DI should be 5555h

end_test:
    HLT
