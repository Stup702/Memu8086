; EXPECT AL=42h

; Test XLAT (Table Lookup)
; AL = DS:[BX + AL]

mov ax, 0
mov ds, ax
mov bx, 0500h

; Setup table at 0500h
mov byte ptr [0500h], 0x10
mov byte ptr [0501h], 0x20
mov byte ptr [0502h], 0x42
mov byte ptr [0503h], 0x40

mov al, 2
xlat ; AL = DS:[0500h + 2] = 42h
hlt
