#include <stdio.h>
#include <stdlib.h>

//tipo tabla descriptores de segmento
typedef struct{
    short int base, size;
} TRTDS;




//declaracion del tipo vector de funciones PENDIENTE
typedef void (t_func[256])(char op1, char op2, char MEM[], short int REG[], short int Byte[]);

void iniciaTablaDeSegmentos(TRTDS *TDD, int RAM, int TAM){
    (*TDD[0]).base=0;
    (*TDD[0]).size=TAM;
    (*TDD[1]).base=TAM;
    (*TDD[1]).size=RAM-TAM;
}

void iniciaRegistros(short int REG){
    REG[0]=0;//CS
    REG[1]=1;//DS
//  REG[2]=;
//  REG[3]=;
//  REG[4]=;
    REG[5]=REG[0];//IP, al principio es igual a CS
//  REG[6]=;
//  REG[7]=;
/* //no hay valores asignados para los siguientes registros
    REG[8]=;//CC
    REG[9]=;//AC
    REG[10]=;
    REG[11]=;
    REG[12]=;
    REG[13]=;
    REG[14]=;
    REG[15]=;
*/
}

void lectura(char MEM[], int *TAM){
    FILE *arch;
    char encabezado[6], version, c;
    char mica[]="C:/Users/micae/Documents/Facultad/Arquitectura/tp/ArchivosCampus/E_MV1/fibo.vmx";
    char santi[]="D:/Documentos/Facultad/arquitecturaDeComputadoras/ejemplosasm/fibo.vmx";
    arch = fopen (mica,"rb");
    int i;

    if(arch = fopen (mica,"rb")){
        fread(encabezado, sizeof(char), 5, arch);
        encabezado[5] = '\0';
        printf("%s\n", encabezado);

        fread(&version, sizeof(char), 1, arch);
        printf("%d\n", version);

/*      //no estoy segura de porque este codigo no funciona, se puede hacer un salvataje con la implementacion siguiente
        char t[3];
        fread(t,sizeof(char),2,arch);
        t[2]='\0';
        printf("%d    %c\n",t,t);
*/
        /*tomo los caracteres uno a uno y los opero bit a bit para pasarlos a (int)tamaño*/
        *TAM=0x00;
        fread(&c,sizeof(char),1,arch);
        *TAM=(c<<0X8);
        fread(&c,sizeof(char),1,arch);
        *TAM|=c;
        printf("TAM: %d\n",*TAM);

        for (i=0 ; i < *TAM ; i++){
            //printf("posicion: %d, tamanio: %d    memoria: ",i,*TAM);
            fread(&c,sizeof(char),1,arch);
            MEM[i]=c;
            //printf("%d  %c\n", MEM[i], MEM[i]);
        }
        fclose(arch);

    }else
        printf("No se pudo abrir el archivo\n");
}
    void codigos(char inst, char *op1, char *op2, char *codop)
    {
        char aux = inst;
        *op1 = (inst<<0x6)&0x03;
        *op2 = (inst<<0x4)&0x03;
        if (*op1 == 3) // probar 0x03
            *codop = inst;
        else
            if (*op2 == 3)
                *codop = inst&0x3F;
            else
                *codop = inst&0x0F;
    }

    void MOV(char op1, char op2, char MEM[], short int REG[], TRTDS TDS[], short int Byte)
    {
        char auxchar, registro1, registro2;
        short int offset1, offset2;
        int i, aux1 = 0, aux2 = 0; // "instrucciones"
        switch (op1)
        {
            case 0:
                for (i = 0 ; i < 3, i++)
                {
                    aux1 = aux1<<8;
                    auxchar = MEM[REG[5]++];
                    aux1 |= auxchar;
                }
            break;
            case 2:
                auxchar = MEM[REG[5]++];
                aux1 = auxchar;
            break;

            }

        switch (op2)
        {
            case 0:
                for (i = 0 ; i < 3, i++)
                {
                    aux2 = aux2<<8;
                    auxchar = MEM[REG[5]++];
                    aux2 |= auxchar;
                }
            break;
            case 1:
                for (i = 0 ; i < 2, i++)
                {
                    aux2 = aux2<<8;
                    auxchar = MEM[REG[5]++];
                    aux2 |= auxchar;
                }
            break;
            case 2:
                auxchar = MEM[REG[5]++];
                aux2 = auxchar;
            break;
        if(op1 == 0)
        {
            registro1 = (op1>>0x10)&0x0F;
            offset1 = (op1&0xFFFF);
            if(op2 == 0)
            {
                registro2 = (op2>>0x10)&0x0F;
                offset2 = (op2&0xFFFF);
                if(registro1 == 0) //ds
                    if (registro2 == 0)
                        MEM[TDS[REG[1]]+offset1] = MEM[TDS[REG[1]]+offset2];
                    else
                        MEM[TDS[REG[1]]+offset1] = MEM[REG[registro2]+offset2];
                else
                    if (registro2 == 0)
                        MEM[REG[registro1]+offset1] = MEM[TDS[REG[1]]+offset2];
                    else
                        MEM[REG[registro1]+offset1] = MEM[REG[registro2]+offset2];
            }
            else
                if(op2 == 1)
                    if(registro1 == 0) //ds
                        MEM[TDS[REG[1]]+offset1] = aux2;
                    else
                        MEM[REG[registro1]+offset1] = aux2;
                else //op2 es 2
                    if(registro1 == 0) //ds
                        MEM[TDS[REG[1]]+offset1] = REG[registro2];
                    else
                        MEM[REG[registro1]+offset1] = REG[registro2];
        }
        else //op1 es registro
        {

        }
    }

    void procesoDatos(char MEM[], TRTDS TDS[], short int REG[], short int Byte[], t_func funciones)
    {
        char inst, op1, op2, codop;
        int i, n = TDS[REG[1]].base;
        short int IPaux;

        for (i = 0 ; i < n ; i++)
        {
            inst = MEM[i];
            codigos(inst, &op1, &op2, &codop);
            //IPaux = REG[5];
            REG[5] += 1;// + Byte[op1] + Byte[op2];

            funciones[codop](op1, op2, MEM, REG, Byte);
        }
    }

    void cargaFunciones(t_func funciones)
    {
        funciones[0] = MOV;
        funciones[1] = ADD;
        funciones[2] = SUB;
        funciones[3] = SWAP;
        funciones[4] = MUL;
        funciones[5] = DIV;
        funciones[6] = CMP;
        funciones[7] = SHL;
        funciones[8] = SHR;
        funciones[9] = AND;
        funciones[10] = OR;
        funciones[11] = XOR;
        funciones[48] = SYS;
        funciones[49] = JMP;
        funciones[50] = JZ;
        funciones[51] = JP;
        funciones[52] = JN;
        funciones[53] = JNZ;
        funciones[54] = JNP;
        funciones[55] = JNN;
        funciones[56] = LDL;
        funciones[57] = LDH;
        funciones[58] = RND;
        funciones[59] = NOT;
        funciones[240] = STOP;
    }



int main(){
    int TAM, RAM = 16384;
    short int REG[16]; //( 0 = CS / 1 = DS / 5 = IP / 8 = CC / 9 = AC )
    short int Byte[4] = {3,2,1,0};
    char MEM[RAM];
    TRTDS TDS[8];
    //declaracion de funciones
    t_func funciones;
    cargaFunciones(funciones);

    //inicializacion de variables y carga de memoria
    lectura(MEM, &TAM);
    iniciaTablaDeSegmentos(&TDS,RAM,TAM);
    iniciaRegistros(REG);


    proceso(MEM,TAM);
    return 0;
}

