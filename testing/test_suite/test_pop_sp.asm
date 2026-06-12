; EXPECT SP=1234h

mov sp, 0FFFEh
push 01234h
pop sp

; On real 8086:
; 1. Fetch 1234h from [SS:SP]
; 2. SP += 2 (SP = 0000h)
; 3. Destination (SP) = 1234h
; Final SP = 1234h

; In buggy emulator:
; 1. SP = fetched_val (SP = 1234h)
; 2. SP += 2 (SP = 1236h)
; Final SP = 1236h

hlt
