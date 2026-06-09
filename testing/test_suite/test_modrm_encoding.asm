; EXPECT MEM[0100h]=AAh
; EXPECT MEM[0105h]=BBh
; EXPECT MEM[0200h]=CCh

mov bx, 0100h
mov byte ptr [bx], 0xAA
mov byte ptr [bx+5], 0xBB
mov byte ptr [bx+256], 0xCC
hlt
