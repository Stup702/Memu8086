; EXPECT AX=0F05h

mov al, 0F5h    ; 245 decimal
; AAM 16 in raw bytes (since many standard assemblers don't support the operand)
db 0D4h, 10h    ; AL / 16. Quotient goes to AH (0Fh), Remainder to AL (05h)
hlt