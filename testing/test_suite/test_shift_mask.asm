; EXPECT AX=0000h

; 8086 Shift Count Masking Test
; On modern x86 (286+), the shift count in CL is masked to 5 bits (CL & 1Fh).
; So shifting by 33 would only shift by 1.
; On a true 8086/8088, the shift count is NOT masked. It will shift 33 times.
; Shifting 0xFFFF left by 33 times on an 8086 results in 0x0000.

mov ax, 0FFFFh
mov cl, 33
shl ax, cl

hlt
