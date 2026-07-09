; EXPECT ASM_ERROR

.CODE
    ORG 100h
start:
    MOV [BX], [SI]
    INT 20h
END start
