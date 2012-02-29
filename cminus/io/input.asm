input:	ADD R6, R6, #-2
	STR R7, R6, #0
	ADD R6, R6, #-1
	STR R5, R6, #0
	ADD R5, R6, #-1
	ADD R6, R6, #-1
	STR R4, R6, #0
	LEA R1, ASCIIBUFF
	LD R2, MaxDigits
	GETC
	OUT
	LD R3,ASCIIMinus
	ADD R3,R3,R0
	ADD R6,R6,#-1
	STR R3,R6,#0 ;minus or not
	ADD R3,R3,#0
	BRz myInput ;escape '-' letter
ValueLoop: ADD R3, R0, XFFF6
	BRz GoodInput
	LD R3, NegZero
	ADD R4, R0, R3
	BRn InvalidInput
	ADD R3, R3, #-10
	ADD R4, R0, R3
	BRzp InvalidInput
	ADD R2, R2, #0
	BRz TooLargeInput
	ADD R2, R2, #-1
	STR R0, R1, #0
	ADD R1, R1, #1
myInput: GETC
	OUT
	BR ValueLoop
GoodInput: LEA R2, ASCIIBUFF
	NOT R2, R2
	ADD R2, R2, #1
	ADD R1, R1, R2
	JSR	ASCIItoBinary
	ADD R0, R0, #0
	BRzp NewCommand
	ADD R2, R0, #-1 ;input -32768
	BRn OverFlow
	LDR R3,R6,#0
	BRnp OverFlow
	BR NewCommand
	
InvalidInput: GETC
	OUT
	ADD R3, R0, xFFF6
	BRnp InvalidInput
	LEA R0, InvalidLetter
	PUTS
	AND R0, R0, #0
	BR NewCommand

TooLargeInput: GETC
	OUT
	ADD R3, R0, xFFF6
	BRnp TooLargeInput
	LEA R0, TooManyDigits
	PUTS
	AND R0,R0,#0
	BR	NewCommand
OverFlow: LEA R0, NumberOverFlow
	PUTS
	AND R0,R0,#0
NewCommand:
	ADD R6,R6,#1
	LDR R3,R6,#-1
	BRnp MyTag
	NOT R0,R0
	ADD R0,R0,#1
MyTag: LDR R4, R6, #0
	ADD R6, R6, #1
	LDR R5, R6, #0
	ADD R6, R6, #1
	LDR R7, R6, #0
	ADD R6, R6, #1
	STR R0, R6, #0
	RET

ASCIItoBinary:
	AND R0, R0, #0
	ADD R1, R1, #0
	BRz DoneAtoB
	LD R3, NegZero
	LEA R2, ASCIIBUFF
	ADD R2, R2, R1
	ADD R2, R2, #-1
	LDR R4, R2, #0
	ADD R4, R4, R3
	ADD R0, R0, R4
	ADD R1, R1, #-1
	BRz DoneAtoB
	ADD R2, R2, #-1
	LDR R4, R2, #0
	ADD R4, R4, R3
	LEA R5, Lookup10
	ADD R5, R5, R4
	LDR R4, R5, #0
	ADD R0, R0, R4
	ADD R1, R1, #-1
	BRz DoneAtoB
	ADD R2, R2, #-1
	LDR R4, R2, #0
	ADD R4, R4, R3
	LEA R5, Lookup100
	ADD R5, R5, R4
	LDR R4, R5, #0
	ADD R0, R0, R4
	ADD R1, R1, #-1
	BRz DoneAtoB
	ADD R2, R2, #-1
	LDR R4, R2, #0
	ADD R4, R4, R3
	LEA R5, Lookup1000
	ADD R5, R5, R4
	LDR R4, R5, #0
	ADD R0, R0, R4
	ADD R1, R1, #-1
	BRz DoneAtoB
	ADD R2, R2, #-1
	LDR R4, R2, #0
	ADD R4, R4, R3
	ADD R5, R4, #-3
	BRp OverFlow
	LEA R5, Lookup10000
	ADD R5, R5, R4
	LDR R4, R5, #0
	ADD R0, R0, R4
DoneAtoB: RET
ASCIIBUFF: .BLKW 6
Lookup10: .FILL #0
	 .FILL #10
	 .FILL #20
	 .FILL #30
	 .FILL #40
	 .FILL #50
	 .FILL #60
	 .FILL #70
	 .FILL #80
	 .FILL #90
Lookup100: .FILL #0
	  .FILL #100
	  .FILL #200
	  .FILL #300
	  .FILL #400
	  .FILL #500
	  .FILL #600
	  .FILL #700
	  .FILL #800
	  .FILL #900
Lookup1000: .FILL #0
	  .FILL #1000
	  .FILL #2000
	  .FILL #3000
	  .FILL #4000
	  .FILL #5000
	  .FILL #6000
	  .FILL #7000
	  .FILL #8000
	  .FILL #9000
Lookup10000: .FILL #0
	  .FILL #10000
	  .FILL #20000
	  .FILL #30000
NegZero: .FILL xFFD0
ASCIIMinus: .FILL xFFD3
TooManyDigits: .FILL x000A
	.STRINGZ "Too Many Digits"
NumberOverFlow: .FILL x000A
	.STRINGZ "Number OverFlow"
InvalidLetter: .FILL x000A
	.STRINGZ "Invalid Input"
MaxDigits: .FILL x0005
