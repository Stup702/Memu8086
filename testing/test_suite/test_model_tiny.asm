; EXPECT AX=1234h
; EXPECT BX=0000h

.MODEL TINY
.CODE
    JMP start
.DATA
    var1 DW 1234h
.CODE
start:
    ; Under .MODEL TINY, @DATA is 0 because it's a flat memory model
    MOV BX, @DATA
    
    ; And variables are accessed perfectly
    MOV AX, var1 ; AX = 1234h
    HLT
