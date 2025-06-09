	AREA TT,CODE,READONLY
	PRESERVE8
	EXPORT Proc_Arm

Proc_Arm
	LDR R4,[SP]		;i5→R4
	LDR R5,[SP,#4]		;i6→R5
	ADDS R6,R0,R1		;i1+i2
	ADDS R6,R6,R2		;i1+i2+i3
	ADDS R6,R6,R3		;i1+i2+i3+i4
	MUL R0,R6,R4		;(i1+i2+i3+i4)*i5
	SUB R0,R0,R5		;(i1+i2+i3+i4)*i5-i6
	MOV PC,LR          ;子函数执行后返回


	END
	