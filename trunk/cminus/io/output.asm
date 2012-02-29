output:	ADD R6, R6, #-2
	STR R7, R6, #0
	ADD R6, R6, #-1
	STR R5, R6, #0
	ADD R6, R6, #-1
	STR R4, R6, #0
	LEA R1, ASCIIBUF ; pt to result string
	ADD R1, R1, #1
	LEA R5, NUMBER
	AND R7, R7, #0
	ADD R7, R7, #4
	ADD R4, R0, #0   ; test sign of value
	BRp Begin
	BRn Negative
	LD R2, ASCIIoffset
	LEA R0, ASCIIBUF
	ADD R0, R0, #5
	STR R2, R0, #0
	AND R2, R2, #0
	ADD R2, R2, #10
	STR R2, R0, #1
	BR putsNumber
Negative: NOT R0, R0
	ADD R0, R0, #1
Begin:  LD R2, ASCIIoffset
	LDR R3, R5, #0
Loop:   ADD R0, R0, R3
	BRn End
	ADD R2, R2, #1
	BRnzp Loop
End:     STR R2, R1, #0
	ADD R1, R1, #1
	LDR R3, R5, #1
	ADD R0, R0, R3
	ADD R5, R5, #2
	ADD R7, R7, #-1
	BRp Begin
	LD R2, ASCIIoffset
	ADD R2, R2, R0
	STR R2, R1, #0
	AND R2, R2, #0
	ADD R2, R2 ,#10
	STR R2, R1, #1
	LEA R0, ASCIIBUF ; pt to result string
	LD R2,NegativeZero
Con: ADD R0, R0, #1
	LDR R1, R0, #0
	ADD R3, R1, R2
	BRz Con
	ADD R4, R4, #0
	BRzp putsNumber
	LD R4,ASCIIneg
	ADD R0, R0, #-1
	STR R4, R0, #0
putsNumber: PUTS
	LDR R4, R6, #0
	ADD R6, R6, #1
	LDR R5, R6, #0
	ADD R6, R6, #1
	LDR R7, R6, #0
	ADD R6, R6, #1
	RET
ASCIIplus: .FILL x2B
ASCIIneg:  .FILL x2D
ASCIIoffset: .FILL x30
NegativeZero: .FILL xFFD0
NUMBER:    .FILL #-10000
           .FILL #10000
           .FILL #-1000
           .FILL #1000
           .FILL #-100
           .FILL #100
           .FILL #-10
           .FILL #10
ASCIIBUF: .BLKW 8
