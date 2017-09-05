//Trabalho pratico da disciplina de Arquitetura de Computadores I
//Nome: Diego Garcia Segovia
//		Vitor Santa Barbara Lira
//Professor : Renan Marks

#include <stdio.h>
#include <stdlib.h>

/*Funções de decodificação*/
int opcode_f(int instrucao);
int rs_f(int instrucao);
int rd_f(int instrucao);
int rt_f(int instrucao);
int shamt_f(int instrucao);
int funct_f(int instrucao);
int imediato_I(int instrucao);
int imediato_J(int instrucao);

/*Funções aritiméticas*/
int add(int a, int b);
int sub(int a, int b);
int addi(int a, int C);
int addiu(int a, unsigned int C);
long long int mult(int a, int b, long long int HILO);
void divide(int a, int b, long long int *HILO);

/*Funções de transferência de dados*/
// int load_word(int memoria[], int C, int reg);
int load_halfword(int memoria[], int C, int rs);
int load_byte(int memoria[], int C, int rs);
int load_halfword_u(int memoria[], int C, int rs);
int load_byte_u(int memoria[], int C, int rs);
// int store_word(int memoria[], int C, int reg);
void store_halfword(int memoria[], int C, int rt, int rs);
void store_byte(int memoria[], int C, int rt, int rs);
int load_upper_i(int C);
int move_from_high(long long int HI);
int move_from_low(long long int LO);

/*Funções lógicas*/
int and(int a, int b);
int andi(int a, int C);
int or(int a, int b);
int ori(int a, int C);
int slt(int a, int b);
int slti(int a, int C);

/*Funções deslocamento de bits*/
int sll(int a, int shamt);
int srl(int a, int shamt);
int sra(int a, int shamt);

/*Desvio Condicional*/
void beq(int a, int b, int *PC, int C);
void bne(int a, int b, int *PC, int C);

/*Desvio Incondicional*/
void jump_f(int *PC);
void jump_resgister(int *PC, int *a);
void jump_and_link(int *PC, int *RA);


void imprime(int r[], int memoria[], int, int);
void imprime_nova(int memoria[1024 * 4], int r[32], int, int);


int main(int argc, char *argv[])
{  	//Inicio função principal
	long long int tamanho_texto, tamanho_dados;
	int memoria[0x00003ff0] = {0};	//Memória principal
	int r[32] = {0};			//Registradores
	int PC;						//Registrador especial PC
	long long int HILO;			//Registrador especial HILO
	int op;						//Determina operação
	int C;						//Immediate
	int temp;					//Variável temporária para uso geral


	FILE *texto,				//Pointer para o arquivo texto e data
		 *dados;

//Prólogo : Carregar os vetores com as informações do Dump do Mars

	r[28] = 0x1800;				//Registrador $gp é inicializado com valor 0x1800
	r[29] = 0x3FFC;				//Registrador $sp é inicializado com valor 0x3FFC

	texto = fopen(argv[1], "rb");
	if(texto == NULL){
		printf("ERRO AO ABRIR\n");
		return 0;
	}
	fseek(texto, 0, SEEK_END);
	tamanho_texto = ftell(texto);
	rewind(texto);
	fread(&memoria[0], sizeof(int), tamanho_texto, texto);
	fclose(texto);

	dados = fopen(argv[2], "rb");
	if(dados == NULL){
		printf("ERRO AO ABRIR\n");
		return 0;
	}
	fseek(dados, 0, SEEK_END);
	tamanho_dados = ftell(dados);
	rewind(dados);
	fread(&memoria[2048], sizeof(int), tamanho_dados, dados);
	fclose(dados);
	

//Fim do prólogo

//Corpo do programa : Decodificação
	for(int i=0; i<tamanho_texto; i++){
		op = opcode_f(memoria[i]);
		if(op != 0){		//Instruções do tipo I e J

			switch(op){

				case 0x08:		//Add Immediate (I)
					C = imediato_I(memoria[i]);
					r[rs_f(memoria[i])] = addi(r[rt_f(memoria[i])], C);
					break;
				case 0x09:		//Add Imediate Unsigned (I)
					C = imediato_I(memoria[i]);
					C = abs(C);
					r[rs_f(memoria[i])] = addiu(r[rt_f(memoria[i])], C);
					break;
				case 0x23: 		//Load Word (I)
					r[rt_f(memoria[i])] = memoria[imediato_I(memoria[i]) + rs_f(memoria[i])];
					break;
				case 0x21:		//Load Halfword (I)
					r[rt_f(memoria[i])] = load_halfword(memoria, imediato_I(memoria[i]), rs_f(memoria[i]));
					break;
				case 0x25:		//Load Halfword Unsigned
					r[rt_f(memoria[i])] = load_halfword_u(memoria, imediato_I(memoria[i]), rs_f(memoria[i]));
				case 0x20:		//Load Byte (I)
					r[rt_f(memoria[i])] = load_byte(memoria, imediato_I(memoria[i]), rs_f(memoria[i]));
					break;
				case 0x24:
					r[rt_f(memoria[i])] = load_byte_u(memoria, imediato_I(memoria[i]), rs_f(memoria[i]));
				case 0x2B:		//Store Word (I)
					memoria[imediato_I(memoria[i]) + rs_f(memoria[i])] = rt_f(memoria[i]);
					break;
				case 0x29:		//Store Halfword (I)
					store_halfword(memoria, imediato_I(memoria[i]), rt_f(memoria[i]), rs_f(memoria[i]));
					break;
				case 0x28:		//Store Byte (I)
					store_byte(memoria, imediato_I(memoria[i]), rt_f(memoria[i]), rs_f(memoria[i]));
					break;
				case 0x0F:		//Load Upper Immediate (I)
					r[rt_f(memoria[i])] = load_upper_i(memoria[i]);
					break;
				case 0x0C:		//And Immediate (I)
					r[rt_f(memoria[i])] = andi(rs_f(memoria[i]), imediato_I(memoria[i]));
					break;
				case 0x0D:		//Or Immediate (I)
					r[rt_f(memoria[i])] = ori(rs_f(memoria[i]), imediato_I(memoria[i]));
					break;
				case 0x0A:		//Set on Less Than Immediate (I)
					r[rt_f(memoria[i])] = slti(rs_f(memoria[i]), imediato_I(memoria[i]));
					break;
				case 0x04:		//Branch on equal (I)
					beq(rs_f(memoria[i]), rt_f(memoria[i]), &PC, imediato_I(memoria[i]));
					break;
				case 0x05:		//Branch on not equal
					bne(rs_f(memoria[i]), rt_f(memoria[i]), &PC, imediato_I(memoria[i]));
					break;
				case 0x02:		//Jump (J)
					jump_f(&PC);
					PC--;
					break;
				case 0x03:		//Jump and Link (J)
					jump_and_link(&PC, &r[31]);
					r[29] = imediato_J(memoria[i]);		//Registrador $sp recebe imediato
					r[29]++;							//Adianta uma posição para ser retirada mais tarde
					break;
			}
		}
		else{		//Instruções do tipo R

			op = funct_f(memoria[i]);

			switch(op){

			case 0x20:		//add
				r[rd_f(memoria[i])] = add(r[rs_f(memoria[i])], r[rt_f(memoria[i])]);
				break;
			case 0x18:		//mult
				HILO = mult(r[rs_f(memoria[i])], r[rt_f(memoria[i])], HILO);
				break;
			case 0x22:		//sub
				r[rd_f(memoria[i])] = sub(r[rs_f(memoria[i])], r[rt_f(memoria[i])]);
				break;
			case 0x1A:		//Div
				divide(r[rs_f(memoria[i])], r[rt_f(memoria[i])], &HILO);
				break;
			case 0x10:		//MFHI
				r[rd_f(memoria[i])] = move_from_high(HILO);
				break;
			case 0x12:		//MFLO
				r[rd_f(memoria[i])] = move_from_low(HILO);
				break;
			case 0x24:		//And
				r[rd_f(memoria[i])] = and(rs_f(memoria[i]), rt_f(memoria[i]));
				break;
			case 0x25:		//Or
				r[rd_f(memoria[i])] = or(rd_f(memoria[i]), rt_f(memoria[i]));
				break;
			case 0x2A:		//Set on Less Than
				r[rd_f(memoria[i])] = slt(rs_f(memoria[i]), rt_f(memoria[i]));
				break;
			case 0x0:		//Shift Left Logical
				r[rd_f(memoria[i])] = sll(rt_f(memoria[i]), shamt_f(memoria[i]));
				break;
			case 0x02:		//Shift Right Logical
				r[rd_f(memoria[i])] = srl(rt_f(memoria[i]), shamt_f(memoria[i]));
				break;
			case 0x03:		//Shift Right arithmetic
				r[rd_f(memoria[i])] = sra(rt_f(memoria[i]), shamt_f(memoria[i]));		
				break;
			case 0x08:		//Jump Register
				temp = rs_f(memoria[i]);
				jump_resgister(&PC, &temp);
				break;
			}
		}		//fim swich
		r[29] = r[29] - 1;				//Ponteiro $sp anda uma posição na pilha
		if(r[29] < 0x3000){				//Verifica stack overflow
			printf("Error: stack overflow.\n");
			imprime(r, memoria, tamanho_texto, tamanho_dados);
			return 0;
		}
		switch(r[2]){
			case 1:		//Syscall de print int
				printf("%d\n", r[4]);
				break;
			case 5:		//Syscall de read int
				scanf("%d", &r[2]);
				break;
			case 17:	//Syscall de exit
				imprime_nova(r, memoria, tamanho_texto, tamanho_dados);
				return 0;
		}
	}		//fim for 
	// imprime_nova(r, memoria, tamanho_texto, tamanho_dados);
	return 0;
}	//fim função principal

/*As função a seguir representam decodificação de campos de instrução.*/
//Tipo R :
int opcode_f(int instrucao){
	int mascara = 0x03FFFFFF;
	int opcode;
	opcode = instrucao & mascara;
	opcode = opcode >> 26;
	return opcode;
}

int rs_f(int instrucao){
	int mascara = 0x03E00000;
	int rs;
	rs = instrucao & mascara;
	rs = rs >> 21;
	return rs;
}

int rt_f(int instrucao){
	int mascara = 0x001F0000;
	int rt;
	rt = instrucao & mascara;
	rt = rt >> 16;
	return rt;
}

int rd_f(int instrucao){
	int mascara = 0x0000F800;
	int rd;
	rd = mascara & instrucao;
	rd = rd >> 11;
	return rd;
}

int shamt_f(int instrucao){
	int mascara = 0x000007C0;
	int shamt;
	shamt = mascara & instrucao;
	shamt = shamt >> 6;
	return shamt;
}

int funct_f(int instrucao){
	int mascara = 0x0000003F;
	int funct;
	funct = instrucao & mascara;
	return funct;
}

//Tipo I

int imediato_I(int instrucao){
	int mascara = 0x0000FFFF;
	int imediato_I;
	imediato_I = mascara & instrucao;
	return imediato_I;
}

//Tipo J

int imediato_J(int instrucao){
	int mascara = 0x03FFFFFF;
	int imediato_J;
	imediato_J = instrucao & mascara;
	return imediato_J;
}

/*Fim instruções de decodificação*/

/*Inicio das funções de aritimética*/
int add(int a, int b)
{
	return a + b;
}

int sub(int a, int b)
{
	return a - b;
}

int addi(int a, int C)
{
	return a + C;
}

int addiu(int a, unsigned int C)
{
	return a + C;
}

long long int mult(int a, int b, long long int HILO)
{
	HILO = a * b;
	return HILO;
}

void divide(int a, int b, long long int *HILO)
{
	long long int mask = 0xFFFFFFFF00000000;
	*HILO = a / b;
	*HILO = mask & *HILO;
	mask = 0x00000000FFFFFFFF;
	*HILO = a % b;
	*HILO = mask & *HILO; 
}

/*Fim das funções aritiméticas*/

/*Início das funções de transferência de dados*/

int load_halfword(int memoria[1024*4], int C, int rs)
{
	int mascara = 0x0000FFFF;
	int retorno;
	retorno = memoria[C + rs];
	retorno = retorno & mascara;
	return retorno;
}

int load_halfword_u(int memoria[1024*4], int C, int rs)
{
	int mascara = 0x0000FFFF;
	int retorno;
	C = abs(C);
	retorno = memoria[C+rs];
	retorno = retorno & mascara;
	return retorno;
}

int load_byte(int memoria[1024*4], int C, int rs)
{
	int mascara = 0x000000FF;
	int retorno;
	retorno = memoria[C + rs];
	retorno = retorno & mascara;
	return retorno;
}

int load_byte_u(int memoria[1024*4], int C, int rs)
{
	int mascara = 0x000000FF;
	int retorno;
	C = abs(C);
	retorno = memoria[C + rs];
	retorno = retorno & mascara;
	return retorno;
}

// 2 bytes
void store_halfword(int memoria[1024 * 4], int C, int rt, int rs)
{
	int mascara = 0x0000FFFF;
	memoria[C + rs] = rt;
	memoria[C + rs] = memoria[C + rs] & mascara;
}	

// 1 byte
void store_byte(int memoria[], int C, int rt, int rs)
{
	int mascara = 0x000000FF;
	memoria[C + rs] = rt;
	memoria[C + rs] = memoria[C + rs] & mascara;
}

int load_upper_i(int C)
{
	return C << 16;
}

int move_from_high(long long int HILO)
{
	long long int mascara = 0xFFFFFFFF00000000;
	int hi;
	HILO = HILO & mascara;
	hi = (int) HILO;
	return hi;
}

int move_from_low(long long int HILO)
{
	long long int mascara = 0x00000000FFFFFFFF;
	int lo;
	HILO = HILO & mascara;
	lo = (int) HILO;
	return lo;
}

/* Fim das funções de tranferência de dados*/

/*Início das funções lógicas*/
int and(int a, int b)
{
	return a & b;
}

int andi(int a, int C)
{
	return a & C;
}

int or(int a, int b)
{
	return a | b;
}

int ori(int a, int C)
{
	return a | C;
}

int slt(int a, int b)
{
	if(a < b)
		return 1;
	else
		return 0;
}

int slti(int a, int C)
{
	if(a < C)
		return 1;
	else
		return 0;
}

/*Fim das funções lógicas*/

/*Inicio das funções desloamento de bits*/
int sll(int a, int shamt)
{
	return a << shamt;
}

int srl(int a, int shamt)
{
	return a >> shamt;
}

int sra(int a, int shamt)
{
	int aux;

	aux = a >> shamt;
	if(a < 0){
		return aux | (0xFFFFFFFF << (32 - shamt));
	}
	else
		return aux;
}

/*Fim das funções de deslocamento de bits*/

/*Inicio das funções de desvio Condicional*/
void beq(int a, int b, int *PC, int C)
{
//a é o registrador $s e b é o registrador $t
	if (a == b){
		*PC = *PC + C;
	}
	return;
}

void bne(int a, int b, int *PC, int C)
{
	if (a != b){
		*PC = *PC + C;
	}
	return;
}

/*Fim das funções de desvio Condicional*/


/*Inicio das funções de desvio Incondicional*/
void jump_resgister(int *PC, int *a)
{
	*PC = *a;
	return;
}

void jump_f(int *PC)
{
	int mascara_temp = 0x0003FFFC;
	int temp_PC = 0;
	temp_PC = *PC;
	temp_PC = temp_PC << 18;
	temp_PC = temp_PC & mascara_temp;
	*PC = temp_PC;
}

void jump_and_link(int *PC, int *RA)
{
	*RA = *PC;
}

/*Fim das funções de desvio Incondicional*/

void imprime(int r[32], int memoria[1024*4], int tamanho_texto, int tamanho_dados)
{
	for(int i=0;i<32;i++){
		printf("$%d\t%#.8x\n", i, r[i]);
	}
	for(int i=0x00000000;i<=tamanho_texto;i=i+0x00000010){
		printf("MEM[%#.8x] %#.8x\t", i, memoria[i]);
		printf("%#.8x\t", memoria[i+0x00000004]);
		printf("%#.8x\t", memoria[i+0x00000008]);
		printf("%#.8x\n", memoria[i+0x0000000C]);
	}
	for (int i=0x00002000;i<=0x2000+tamanho_dados;i=i+0x00000010){
		printf("MEM[%#.8x] %#.8x\t", i, memoria[i]);
		printf("%#.8x\t", memoria[i+0x00000004]);
		printf("%#.8x\t", memoria[i+0x00000008]);
		printf("%#.8x\n", memoria[i+0x0000000C]);
	}
	for (int i = 0x3000;i<=0x3FFF;i=i+0x0010){
		printf("MEM[%#.8x] %#.8x\t", i, memoria[i]);
		printf("%#.8x\t", memoria[i+0x00000004]);
		printf("%#.8x\t", memoria[i+0x00000008]);
		printf("%#.8x\n", memoria[i+0x0000000C]);
	}
	return;
}

void imprime_nova(int memoria[0x00003ff0], int r[32], int tamanho_dados, int tamanho_texto){
	for (int i=0;i<32;i++){
		printf("$%d\t%#.8x\n", i, r[i]);
	}
	for (int i=0;i<=0x00003ff0;i+=16)
	{
		printf("Mem[%#.8x]\t", i);
		int j;
		for (j=0;j<4;++j)
		{
			printf("%#.8x\t", memoria[i+j]);
		}
		printf("\n");
	}
}

/*******Anotações


*/