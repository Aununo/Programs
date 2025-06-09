extern int Proc_Arm (int a,int b,int c,int d,int e, int f);

__asm int convertNum(int i){
	// R0包含原始值（ABCD）
	AND R1,R0,#0x000000FF    // R1 = D
	AND R2,R0,#0x0000FF00		// R2 = C
	AND R3,R0,#0x00FF0000    // R3 = B
	MOV R4,R0,LSR #24        // R4 = A
	
	ORR R0,R1,R2,LSR #8    // R0 = D | (C << 8)
	ORR R0,R0,R3,LSL #16	   // R0 = (D | C << 8) | (B << 16)
	ORR R0,R0,R4,LSL #24   // R0 = (D | C << 8 | B << 16) | (A << 24)
	MOV PC, LR
}

int main (){
		
	int a,b, tmp;
	a=1;
	b=2;
	__asm{
		ADD tmp,a,b
		LSL tmp, #0x03
	}
	int convert=convertNum(0x12345678);
	convert+=1;
	int result =Proc_Arm(1,2,3,4,5,6);

}

