; Test segment overrides on string instructions
; Validates that CS:LODSW reads from CS instead of DS

.MODEL SMALL
.STACK 100h
.DATA
    ; DS points here by default.
    ; This data should NOT be read.
    wrong_data DW 0000h, 0000h
    
.CODE
    ; The actual data we want to read is in CS
src_data DW 1234h, 5678h

MAIN PROC
    MOV AX, @DATA
    MOV DS, AX
    MOV ES, AX

    ; Read using LODSW with CS override
    LEA SI, src_data
    
    ; CS override LODSW (2Eh is CS segment override prefix)
    DB 2Eh
    LODSW           ; AX = 1234h
    MOV BX, AX
    
    DB 2Eh
    LODSW           ; AX = 5678h
    MOV CX, AX

    ; EXPECT BX=1234h
    ; EXPECT CX=5678h

    ; Exit
    MOV AH, 4ch
    INT 21h
MAIN ENDP
END MAIN
