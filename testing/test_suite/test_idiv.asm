; EXPECT AX=FEFDh

; Test IDIV (Signed Division)
mov ax, 0FFF5h  ; -11
mov bl, 0003h   ; 3
idiv bl         ; AL = -3 (FDh), AH = -2 (FEh) -> AX = FEFDh
hlt
