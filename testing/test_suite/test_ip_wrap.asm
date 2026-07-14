; Test IP wrapping across the 64K segment boundary
; We will pad the file with NOPs up to FFFFh and put an instruction that spans FFFFh to 0000h
; Wait, we can't easily assemble a 64K file with DUP because the assembler creates a byte array
; in memory. Let's try ORG FFFDh instead!

.MODEL TINY
.CODE
ORG 0FFFDh

    ; FFFD: MOV AX, 1234h (3 bytes: B8 34 12)
    ; FFFD: B8
    ; FFFE: 34
    ; FFFF: 12
    ; 0000: HLT (1 byte: F4)
    ; 0001: padding
    
    MOV AX, 1234h
    ; EXPECT AX=1234h
    HLT

    ; The assembler will generate memory up to FFFFh, and wrap around?
    ; Wait, our assembler doesn't wrap the output buffer. 
    ; It emits to code_bytes array sequentially.
    ; If we ORG 0FFFDh, it pads with 0 until FFFDh.
    ; Then it emits 3 bytes. The total size is 65536 bytes.
    ; The HLT will be at 65536, which is outside the 64K limit for a single segment.
    ; So maybe this test isn't assembler-friendly.
