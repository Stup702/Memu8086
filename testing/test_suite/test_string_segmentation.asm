; EXPECT AX=1234h
; EXPECT BX=5678h
; EXPECT CX=0000h
; EXPECT DX=0002h

.MODEL SMALL
.STACK 100h

.DATA
    dest_var DW 0000h
    dest_var2 DW 0000h

.CODE
MAIN PROC
    JMP start

    ; Define raw data in the CODE segment!
src_data DW 1234h, 5678h

start:
    ; Setup for MOVSW
    MOV AX, @DATA
    MOV ES, AX      ; ES points to DATA segment (Destination)
    
    MOV AX, CS
    MOV DS, AX      ; DS points to CODE segment (Source)

    ; We are going to copy src_data (from CS) into dest_var (in @DATA)
    LEA SI, src_data
    MOV DX, SI
    LEA DI, dest_var
    MOV CX, 2
    CLD             ; Clear direction flag (forward)
    REP MOVSW       ; Copies 4 bytes from DS:SI to ES:DI

    ; Now let's verify it worked by reading the variables back normally!
    MOV AX, @DATA
    MOV DS, AX
    
    MOV AX, dest_var   ; Should be 1234h
    MOV BX, dest_var2  ; Should be 5678h

    HLT
MAIN ENDP
END MAIN
