; EXPECT SI=0000h
; EXPECT ZF=1

mov si, 0AAAAh
and si, 05555h  ; SI = 0, ZF=1
hlt
