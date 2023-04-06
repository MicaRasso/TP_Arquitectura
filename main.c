#include <stdio.h>
#include <stdlib.h>

//tipo tabla descriptores de segmento
typedef struct{
    short int base, size;
} TRTDS;

//declaracion del tipo vector de funciones PENDIENTE
typedef void (*t_func)(char op1, char op2, char MEM[], int REG[], TRTDS TDS[]);

void iniciaTablaDeSegmentos(TRTDS TDS[], int RAM, int TAM)
{
    TDS[0].base=0;
    TDS[0].size=TAM;
    TDS[1].base=TAM;
    TDS[1].size=RAM-TAM;
}

void iniciaRegistros(int REG[]){
    REG[0]=0;//CS
    REG[1]=0x10000;//DS
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

short int corrimiento (int aux, int izq, int der)
{
    aux = (aux<<8)>>8;
    return (aux<<izq)>>der;
}

void lectura(char MEM[], int *TAM)
{
    FILE *arch;
    char encabezado[6], version, c;
    //arch = fopen("C:/Users/micae/Documents/Facultad/Arquitectura/tp/ArchivosCampus/E_MV1/fibo.vmx", "rb");
    arch = fopen("D:/Documentos/Facultad/arquitecturaDeComputadoras/ejemplosasm/fibo.vmx", "rb");
    int i;

    if(arch != NULL)
    {
        fread(encabezado, sizeof(char), 5, arch);
        encabezado[5] = '\0';
        printf("%s\n", encabezado);

        fread(&version, sizeof(char), 1, arch);
        printf("%d\n", version);

        /*tomo los caracteres uno a uno y los opero bit a bit para pasarlos a (int)tamaño*/
        *TAM=0x00;
        fread(&c,sizeof(char),1,arch);
        *TAM=(c<<0X8);
        fread(&c,sizeof(char),1,arch);
        *TAM|=c;
        printf("TAM: %d\n",*TAM);

        for (i=0 ; i < *TAM ; i++)
        {
            //printf("posicion: %d, tamanio: %d    memoria: ",i,*TAM);
            fread(&c,sizeof(char),1,arch);
            MEM[i]=c;
            //printf("%d  %c\n", MEM[i], MEM[i]);
        }
        fclose(arch);

    }
    else
        printf("No se pudo abrir el archivo\n");
}

void codigos(char inst, char *op1, char *op2, char *codop)
{
    *op1 = (inst>>0x6)&0x03;
    *op2 = (inst>>0x4)&0x03;
    if (*op1 == 3) // probar 0x03
        *codop = inst;
    else
        if (*op2 == 3)
            *codop = inst&0x3F;
        else
            *codop = inst&0x0F;
    printf("\nop1: %d - op2: %d - codop: %d\n",*op1, *op2, *codop);
}

void MOV(char op1, char op2, char MEM[], int REG[], TRTDS TDS[])
{
    char auxchar = 0, posmem1 = 0, posmem2 = 0, registro1 = 0, registro2 = 0, segmento1 = 0, segmento2 = 0;
    short int inmediato2 = 0, offset1 = 0, offset2 = 0;
    int valor2 = 0; // "instrucciones"

    switch (op1)
    {
        case 0: // op1 es memoria
            registro1 = MEM[REG[5]++];
            offset1 = MEM[REG[5]++];
            offset1 = offset1<<8;
            offset1 |= MEM[REG[5]++];
            posmem1 = TDS[REG[1]>>16].base;
            if(registro1 != 1)
                posmem1 += REG[registro1];
        break;
        case 2: // op1 es registro
            auxchar = MEM[REG[5]++];
            registro1 = auxchar&0x0F;
            segmento1 = (auxchar>>4)&0x03;
        break;
    }

    switch (op2)
    {
        case 0: //op2 es memoria
            registro2 = MEM[REG[5]++];
            offset2 = MEM[REG[5]++];
            offset2 = offset2<<8;
            offset2 |= MEM[REG[5]++];
            posmem2 = TDS[REG[1]>>16].base;
            if(registro2 != 1)
                posmem2 += REG[registro2];
            valor2 = MEM[posmem2+offset2];

        break;
        case 1: //op2 es inmediato
            inmediato2 = MEM[REG[5]++];
            inmediato2 = inmediato2<<8;
            inmediato2 |= MEM[REG[5]++];
            valor2 = inmediato2;
        break;
        case 2: //op2 es registro
            printf("valor 2:%d", valor2);
            auxchar = MEM[REG[5]++];
            registro2 = auxchar&0x0F;
            segmento2 = (auxchar>>4)&0x03;
            valor2 = REG[registro2];
            switch(segmento2)
            {
                case 1:
                    valor2 = corrimiento(valor2, 24, 24);
                break;
                case 2:
                    valor2 = corrimiento(valor2, 16, 24);
                break;
                case 3:
                    valor2 = corrimiento(valor2, 16, 16);
                break;
            }
        break;
    }

    if(op1 == 0) //op1 es memoria
        MEM[posmem1+offset1] = valor2;
    else //op1 es registro
    {
        switch(segmento1)
        {
            case 0:
                REG[registro1] = valor2;
            break;
            case 1:
                REG[registro1] &= 0xFFFFFF00;
                REG[registro1] |= valor2;
            break;
            case 2:
                REG[registro1] &= 0xFFFF00FF;
                REG[registro1] |= valor2<<8;
            break;
            case 3:
                REG[registro1] &= 0xFFFF0000;
                REG[registro1] |= valor2;
        }
    }
    if(op1 == 0)
        if(op2 == 0) //memoria a memoria
            printf("MOV [%d], [%d] \n",posmem1+offset1-61, posmem2+offset2-61);
        else
            if(op2 == 1) // inmediato a memoria
                printf("MOV [%d], %d \n",posmem1+offset1-61, valor2);
            else // registro a memoria
                printf("MOV [%d], %d \n",posmem1+offset1-61, registro2);
    else
        if(op2 == 0) //memoria a registro
            printf("MOV %d, [%d] \n", registro1, posmem2+offset2-61);
        else
            if(op2 == 1) // inmediato a registro
                printf("MOV %d, %d \n",registro1, valor2);
            else // registro a registro
                printf("MOV %d, %d \n",registro1, registro2);

}

void ADD(char op1, char op2, char MEM[], int REG[], TRTDS TDS[])
{
    char auxchar = 0, posmem1 = 0, posmem2 = 0, registro1 = 0, registro2 = 0, segmento1 = 0, segmento2 = 0;
    short int inmediato2 = 0, offset1 = 0, offset2 = 0;
    int valor2 = 0; // "instrucciones"
    switch (op1)
    {
        case 0: // op1 es memoria
            registro1 = MEM[REG[5]++];
            offset1 = MEM[REG[5]++];
            offset1 = offset1<<8;
            offset1 |= MEM[REG[5]++];
            posmem1 = TDS[REG[1]>>16].base;
            if(registro1 != 1)
                posmem1 += REG[registro1];
        break;
        case 2: // op1 es registro
            auxchar = MEM[REG[5]++];
            registro1 = auxchar&0x0F;
            segmento1 = (auxchar>>4)&0x03;
        break;
    }
    switch (op2)
    {
        case 0: //op2 es memoria
            registro2 = MEM[REG[5]++];
            offset2 = MEM[REG[5]++];
            offset2 = offset2<<8;
            offset2 |= MEM[REG[5]++];
            posmem2 = TDS[REG[1]>>16].base;
            if(registro2 != 1)
                posmem2 += REG[registro2];
            valor2 = MEM[posmem2+offset2];

        break;
        case 1: //op2 es inmediato
            inmediato2 = MEM[REG[5]++];
            inmediato2 = inmediato2<<8;
            inmediato2 |= MEM[REG[5]++];
            valor2 = inmediato2;
        break;
        case 2: //op2 es registro
            auxchar = MEM[REG[5]++];
            registro2 = auxchar&0x0F;
            segmento2 = (auxchar>>4)&0x03;

            valor2 = REG[registro2];
            switch(segmento2)
            {
                case 1:
                    valor2 = corrimiento(valor2, 24, 24);
                break;
                case 2:
                    valor2 = corrimiento(valor2, 16, 24);
                break;
                case 3:
                    valor2 = corrimiento(valor2, 16, 16);
                break;
            }
        break;
    }
    if(op1 == 0) //op1 es memoria
        MEM[posmem1+offset1] += valor2;
    else //op1 es registro
    {
        switch(segmento1)
        {
            case 0:
                REG[registro1] += valor2;
            break;
            case 1:
                REG[registro1] += valor2;
            break;
            case 2:
                REG[registro1] += valor2<<8;
            break;
            case 3:
                REG[registro1] += valor2;
        }
    }
    if(op1 == 0)
        if(op2 == 0) //memoria a memoria
            printf("ADD [%d], [%d] \n",posmem1+offset1-61, posmem2+offset2-61);
        else
            if(op2 == 1) // inmediato a memoria
                printf("ADD [%d], %d \n",posmem1+offset1-61, valor2);
            else // registro a memoria
                printf("ADD [%d], %d \n",posmem1+offset1-61, registro2);
    else
        if(op2 == 0) //memoria a registro
            printf("ADD %d, [%d] \n", registro1, posmem2+offset2-61);
        else
            if(op2 == 1) // inmediato a registro
                printf("ADD %d, %d \n",registro1, valor2);
            else // registro a registro
                printf("ADD %d, %d \n",registro1, registro2);
}

void SWAP(char op1, char op2, char MEM[], int REG[], TRTDS TDS[])
{
    char auxV1[4], auxV2[4], auxchar = 0, posmem1 = 0, posmem2 = 0, registro1 = 0, registro2 = 0, segmento1 = 0, segmento2 = 0;
    short int inmediato2 = 0, offset1 = 0, offset2 = 0;
    int i, valor1 = 0, valor2 = 0, auxreg = 0; // "instrucciones"

    switch (op1)
    {
        case 0: // op1 es memoria
            registro1 = MEM[REG[5]++];
            offset1 = MEM[REG[5]++];
            offset1 = offset1<<8;
            offset1 |= MEM[REG[5]++];
            posmem1 = TDS[REG[1]>>16].base;
            if(registro1 != 1)
                posmem1 += REG[registro1];
        break;
        case 2: // op1 es registro
            auxchar = MEM[REG[5]++];
            registro1 = auxchar&0x0F;
            segmento1 = (auxchar>>4)&0x03;
        break;
    }

    switch (op2)
    {
        case 0: //op2 es memoria
            registro2 = MEM[REG[5]++];
            offset2 = MEM[REG[5]++];
            offset2 = offset2<<8;
            offset2 |= MEM[REG[5]++];
            posmem2 = TDS[REG[1]>>16].base;
            if(registro2 != 1)
                posmem2 += REG[registro2];
            valor2 = MEM[posmem2+offset2];
            if (op1 == 2)
            {
                registro2 = registro1;
                segmento2 = segmento1;
                posmem1 = posmem2;
                offset1 = offset2;
            }
        break;
        case 2: //op2 es registro
            auxchar = MEM[REG[5]++];
            registro2 = auxchar&0x0F;
            segmento2 = (auxchar>>4)&0x03;
        break;
    }
    if(op1 == 0)
        if (op2 == 0) // memoria a memoria
        {
            for(i = 0 ; i < 4 ; i++)
            {
                auxV1[i] = MEM[i+posmem1+offset1];
                auxV2[i] = MEM[i+posmem2+offset2];
            }
            for(i = 0 ; i < 4 ; i++)
            {
                MEM[i+posmem2+offset2] = auxV1[i];
                MEM[i+posmem1+offset1] = auxV2[i];
            }
        }
        else // memoria a registro
        {
            for(i = 0 ; i < 4 ; i++)
                auxV1[i] = MEM[i+posmem1+offset1];

            valor2 = REG[registro2];
            switch(segmento2)
            {
                case 1:
                    valor2 = corrimiento(valor2, 24, 24);
                break;
                case 2:
                    valor2 = corrimiento(valor2, 16, 24);
                break;
                case 3:
                    valor2 = corrimiento(valor2, 16, 16);
                break;
            }
            MEM[posmem1+offset1] = (valor2>>24)&0xFF;
            MEM[posmem1+offset1+1] = (valor2>>16)&0xFF;
            MEM[posmem1+offset1+2] = (valor2>>8)&0xFF;
            MEM[posmem1+offset1+3] = (valor2)&0xFF;

            for(i = 0 ; i < 4 ; i++)
            {
                REG[registro2]<<8;
                REG[registro2] = auxV1[i];
            }
        }
    else //registro a registro
    {
        valor1 = REG[registro1];
        valor2 = REG[registro2];
        switch(segmento1)
        {
            case 1:
                valor1 = corrimiento(valor1, 24, 24);
            break;
            case 2:
                valor1 = corrimiento(valor1, 16, 24);
            break;
            case 3:
                valor1 = corrimiento(valor1, 16, 16);
            break;
        }
        switch(segmento2)
        {
            case 0:
                REG[registro2] = valor1;
            case 1:
                valor2 = corrimiento(valor2, 24, 24);
                REG[registro2] &= 0xFFFFFF00;
                REG[registro2] |= valor1;
            break;
            case 2:
                valor2 = corrimiento(valor2, 16, 24);
                REG[registro2] &= 0xFFFF00FF;
                REG[registro2] |= (valor1<<8);
            break;
            case 3:
                valor2 = corrimiento(valor2, 16, 16);
                REG[registro2] &= 0xFFFF0000;
                REG[registro2] |= (valor1);
            break;
        }
        switch(segmento1)
        {
            case 0:
                REG[registro1] = valor2;
            case 1:
                REG[registro1] &= 0xFFFFFF00;
                REG[registro1] |= valor2;
            break;
            case 2:
                REG[registro1] &= 0xFFFF00FF;
                REG[registro1] |= (valor2<<8);
            break;
            case 3:
                REG[registro1] &= 0xFFFF0000;
                REG[registro1] |= (valor2);
            break;
        }
    }
    if(op1 == 0)
        if(op2 == 0) //memoria a memoria
            printf("SWAP [%d], [%d] \n",posmem1+offset1-61, posmem2+offset2-61);
        else // registro a memoria
            printf("SWAP [%d], %d \n",posmem1+offset1-61, registro2);
    else
        if(op2 == 0) //memoria a registro
            printf("SWAP %d, [%d] \n", registro1, posmem2+offset2-61);
        else // registro a registro
            printf("SWAP %d, %d \n",registro1, registro2);
}

void CMP(char op1, char op2, char MEM[], int REG[], TRTDS TDS[])
{
    char auxchar = 0, posmem1 = 0, posmem2 = 0, registro1 = 0, registro2 = 0, segmento1 = 0, segmento2 = 0;
    short int inmediato2 = 0, offset1 = 0, offset2 = 0;
    int valor1 = 0, valor2 = 0; // "instrucciones"

    switch (op1)
    {
        case 0: // op1 es memoria
            registro1 = MEM[REG[5]++];
            offset1 = MEM[REG[5]++];
            offset1 = offset1<<8;
            offset1 |= MEM[REG[5]++];
            posmem1 = TDS[REG[1]>>16].base;
            if(registro1 != 1)
                posmem1 += REG[registro1];
            valor1 = MEM[posmem1+offset1];
        break;
        case 2: // op1 es registro
            auxchar = MEM[REG[5]++];
            registro1 = auxchar&0x0F;
            segmento1 = (auxchar>>4)&0x03;
            switch (segmento1)
            {
                case 0:
                    valor1 = REG[registro1];
                break;
                case 1:
                    valor1 = corrimiento(REG[registro1], 24, 24);
                break;
                case 2:
                    valor1 = corrimiento(REG[registro1], 16, 24);
                break;
                case 3:
                    valor1 = corrimiento(REG[registro1], 16, 16);
                break;
            }
        break;
        }

    switch (op2)
    {
        case 0: //op2 es memoria
            registro2 = MEM[REG[5]++];
            offset2 = MEM[REG[5]++];
            offset2 = offset2<<8;
            offset2 |= MEM[REG[5]++];
            posmem2 = TDS[REG[1]>>16].base;
            if(registro2 != 1)
                posmem2 += REG[registro2];
            valor2 = MEM[posmem2+offset2];

        break;
        case 1: //op2 es inmediato
            inmediato2 = MEM[REG[5]++];
            inmediato2 = inmediato2<<8;
            inmediato2 |= MEM[REG[5]++];
            valor2 = inmediato2;
        break;
        case 2: //op2 es registro
            auxchar = MEM[REG[5]++];
            registro2 = auxchar&0x0F;
            segmento2 = (auxchar>>4)&0x03;

            valor2 = REG[registro2];
            switch(segmento2)
            {
                case 1:
                    valor2 = corrimiento(valor2, 24, 24);
                break;
                case 2:
                    valor2 = corrimiento(valor2, 16, 24);
                break;
                case 3:
                    valor2 = corrimiento(valor2, 16, 16);
                break;
            }
        break;
    }
    valor1 -= valor2;
    REG[8] = (valor1) & 0x80000000;
    if (valor1 == 0)
        REG[8] |= 0x40000000;
    else
        REG[8] &= 0x80000000;
    printf("N: %d - Z: %d\n",(REG[8]>>31)&01, (REG[8]>>30)&01);
}

void JP(char op1, char op2, char MEM[], int REG[], TRTDS TDS[])
{
    int N = (REG[8]>>31) & 0x01;
    short int inmediato;

    inmediato = MEM[REG[5]++];
    inmediato = inmediato<<8;
    inmediato |= MEM[REG[5]++];
   // printf("byte: %d\n", inmediato);
    if(N == 0)
        REG[5] = inmediato;
}

void SYS(char op1, char op2, char MEM[], int REG[], TRTDS TDS[])
{
    int formato = 0, posmem = 0, i, j;
    short int cantidad = 0, tamanio = 0, operacion = 0;
    int aux;

    formato = REG[10]; //al
    tamanio = (REG[12]>>8) & 0xFF; //ch
    cantidad = REG[12] & 0xFF; //cl
    posmem = TDS[REG[1]].base + REG[13]; //dx

    operacion = MEM[REG[5]++];
    operacion = operacion<<8;
    operacion |= MEM[REG[5]++];

    switch(operacion)
    {
        case 1: //lectura
            switch(formato)
            {
                case 1: // decimal
                    for(i = 0 ; i < cantidad ; i++)
                    {
                        scanf("%d", &aux);
                        for(j = tamanio ; j > 0 ; j--)
                            MEM[posmem+i+j] = (aux>>(j*8))&0xFF;
                    }
                case 2: // caracteres
                    for(i = 0 ; i < cantidad ; i++)
                    {
                        scanf("%c", &aux);
                        for(j = tamanio ; j > 0 ; j--)
                            MEM[posmem+i+j] = (aux>>(j*8))&0xFF;
                    }
                break;
                case 4: // octal
                    for(i = 0 ; i < cantidad ; i++)
                    {
                        scanf("%o", &aux);
                        for(j = tamanio ; j > 0 ; j--)
                            MEM[posmem+i+j] = (aux>>(j*8))&0xFF;
                    }
                break;
                case 8: // hexadecimal
                    for(i = 0 ; i < cantidad ; i++)
                    {
                        scanf("%x", &aux);
                        for(j = tamanio ; j > 0 ; j--)
                            MEM[posmem+i+j] = (aux>>(j*8))&0xFF;
                    }
                break;
            }
        break;
        case 2: // escritura
            switch(formato)
            {
                case 1: // decimal
                    for(i = 0 ; i < cantidad ; i++)
                    {
                        for(j = tamanio ; j > 0 ; j--)
                        {
                            aux = aux<<8;
                            aux = MEM[posmem+i*tamanio+j];
                        }
                        printf("%d\n", aux);
                    }
                break;
                case 2: // caracteres
                    for(i = 0 ; i < cantidad ; i++)
                    {
                        for(j = 0 ; j < tamanio ; j++)
                            printf("%c", MEM[posmem+i+j]);
                        printf("\n");
                    }
                break;
                case 4: // octal
                    for(i = 0 ; i < cantidad ; i++)
                    {
                        for(j = 0 ; j < tamanio ; j++)
                            printf("%o", MEM[posmem+i+j]);
                        printf("\n");
                    }
                break;
                case 8: // hexadecimal
                    for(i = 0 ; i < cantidad ; i++)
                    {
                        for(j = 0 ; j < tamanio ; j++)
                            printf("%x", MEM[posmem+i+j]);
                        printf("\n");
                    }
                break;
            }
        break;
    }
}

void procesoDatos(char MEM[], TRTDS TDS[], int REG[], t_func funciones[])
{
    char inst, op1, op2, codop;
    int i, n = TDS[REG[1]>>16].base;
    for (i = 0 ; i < n ; i++)
    {
        inst = MEM[REG[5]++];
        codigos(inst, &op1, &op2, &codop);
        funciones[codop](op1, op2, MEM, REG, TDS);
        //dissasembler(op1, op2, codop)
    }
}

void cargaFunciones(t_func funciones[])
{
        funciones[0] = MOV;
        funciones[1] = ADD;
  //      funciones[2] = SUB;
        funciones[3] = SWAP;/*
        funciones[4] = MUL;
        funciones[5] = DIV;*/
        funciones[6] = CMP;
    /*  funciones[7] = SHL;
        funciones[8] = SHR;
        funciones[9] = AND;
        funciones[10] = OR;
        funciones[11] = XOR;
*/
        funciones[48] = SYS;/*
        funciones[49] = JMP;
        funciones[50] = JZ;*/
        funciones[51] = JP;
      /*  funciones[52] = JN;
        funciones[53] = JNZ;
        funciones[54] = JNP;
        funciones[55] = JNN;
        funciones[56] = LDL;
        funciones[57] = LDH;
        funciones[58] = RND;
        funciones[59] = NOT;
        funciones[240] = STOP;*/
    }



int main(){
    int TAM, RAM = 16384, REG[16];
    //( 0 = CS / 1 = DS / 5 = IP / 8 = CC / 9 = AC )
    char MEM[RAM];
    TRTDS TDS[8];
    //declaracion de funciones
    t_func funciones[256];
    cargaFunciones(funciones);

    //inicializacion de variables y carga de memoria
    lectura(MEM, &TAM);
    iniciaTablaDeSegmentos(TDS, RAM, TAM);
    iniciaRegistros(REG);


    procesoDatos(MEM, TDS, REG, funciones);
    return 0;
}

