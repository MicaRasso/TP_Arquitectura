#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//tipo tabla descriptores de segmento
typedef struct
{
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

void iniciaRegistros(int REG[])
{
    REG[0]=0;//CS
    REG[1]=0x10000;//DS
//  REG[2]=;
//  REG[3]=;
//  REG[4]=;
    REG[5]=REG[0];//IP, al principio es igual a CS
//  REG[6]=;
//  REG[7]=;
    REG[8]=0;//CC
/*  //no hay valores asignados para los siguientes registros
    REG[9]=;//AC
    REG[10]=;
    REG[11]=;
    REG[12]=;
    REG[13]=;
    REG[14]=;
    REG[15]=;
*/
}

short int baseds(TRTDS TDS[], int REG[])
{
    return TDS[(REG[1])>>16].base;
}

short int corrimiento (int aux, int izq, int der)
{
    aux = (aux<<8)>>8;
    return (aux<<izq)>>der;
}

void mascaras(int *valor, char tipo)
{
    switch(tipo)
    {
        case 0://byte 4
            (*valor)&=0xFF;
        break;
        case 1://byte 4 y 3
            (*valor)&=0xFFFF;
        break;
        case 2://byte 4, 3 y 2
            (*valor)&=0xFFFFFF;
        break;
        case 3://byte 4, 3, 2 y 1
            (*valor)&=0xFFFFFFFF;
        break;
        case 4://byte 3 - EXTRA
            (*valor)&=0xFF00;
        break;
    }
}

void lee4byte(int *var, char MEM[], int REG[])
{
    int i , j, aux = 0;
    j=4;
    for(i = 3 ; i >= 0 ; i--)
    {
        aux = MEM[REG[5]++];
        aux |= aux<<8*j--;
        mascaras(&aux, i);
    }
    *var = aux;
}

void lee2byte(short int *var, char MEM[], int REG[])
{
    int i , j, aux = 0;
    j=2;
    for(i = 1 ; i >= 0 ; i--)
    {
        aux = MEM[REG[5]++];
        aux |= aux<<8*j--;
        mascaras(&aux, i);
    }
    *var = aux;
}

void lee1byte(char *var, char MEM[], int REG[])
{
    int aux = 0;
    aux = MEM[REG[5]++];
    mascaras(&aux, 0);
    *var = aux;
}

void leedemem(int *var, char MEM[], short int posmem)
{
    int i, j = 3, aux2 = 0, aux = 0;

    for(i = 0 ; i < 4 ; i++)
    {
        aux = MEM[posmem+j--];
        aux = aux<<(8*i);
        mascaras(&aux, i);
        aux2 |= aux;
    }
    *var = aux2;
}

void leemem(short int *posmem, char MEM[], TRTDS TDS[], int REG[])
{
    char registro = 0;
    short int offset = 0;
    lee1byte(&registro, MEM, REG);
    lee2byte(&offset, MEM, REG);
    *posmem = baseds(TDS, REG) + offset;
    if(registro != 1)
        (*posmem) += REG[registro];
}

void leeinm(short int *inmediato, char MEM[], int REG[])
{
    short int aux = 0;
    lee2byte(&aux, MEM, REG);
    *inmediato = aux;
}

void leereg(char *registro, char *segmento, char MEM[], int REG[])
{
    char aux = 0;
    lee1byte(&aux, MEM, REG);
    *registro = aux&0x0F;
    *segmento = (aux>>4)&0x03;
}

void lectura(char MEM[], int *TAM)
{
    FILE *arch;
    char encabezado[6], version, c;
    //arch = fopen("C:/Users/micae/OneDrive/Documents/Facultad/Arquitectura/tp/ArchivosCampus/E_MV1/ej2b.vmx", "rb");
    arch = fopen("D:/Documentos/Facultad/arquitecturaDeComputadoras/ejemplosasm/ej1.vmx", "rb");
    int i;

    if(arch != NULL)
    {
        fread(encabezado, sizeof(char), 5, arch);
        encabezado[5] = '\0';
        printf("%s ", encabezado);

        fread(&version, sizeof(char), 1, arch);
        printf("Ver: %d ", version);

        /*tomo los caracteres uno a uno y los opero bit a bit para pasarlos a (int)tamaño*/
        *TAM=0x00;
        fread(&c,sizeof(char),1,arch);
        *TAM = c;
        *TAM = (*TAM<<0X8);
        fread(&c,sizeof(char),1,arch);
        *TAM |= c;
        printf("TAM: %d\n",*TAM);

        for (i=0 ; i < *TAM ; i++)
        {
            //printf("posicion: %d, tamanio: %d    memoria: ",i,*TAM);
            fread(&c,sizeof(char),1,arch);
            MEM[i] = c;
            //printf("%d  %c\n", MEM[i], MEM[i]);
        }
        fclose(arch);

    }
    else
        printf("No se pudo abrir el archivo\n");
}

void codigos(char inst, char *op1, char *op2, char *codop)
{
    int aux=0;
    aux=inst;
    *op1 = (inst>>0x6)&0x03;
    *op2 = (inst>>0x4)&0x03;
    if (*op1 == 3) // probar 0x03
        *codop = aux&0xFF;
    else
        if (*op2 == 3)
            *codop = inst&0x3F;
        else
            *codop = inst&0x0F;
    printf("\nop1: %d - op2: %d - codop: %x\n",*op1, *op2, *codop);
}

void MOV(char op1, char op2, char MEM[], int REG[], TRTDS TDS[])
{
    char registro1 = 0, registro2 = 0, segmento1 = 0, segmento2 = 0;
    short int posmem1 = 0, posmem2 = 0, aux2 = 0;
    int i = 0, valor2 = 0;

    switch (op1)
    {
        case 0: // op1 es memoria
            leemem(&posmem1,MEM,TDS,REG);
        break;
        case 2: // op1 es registro
            leereg(&registro1, &segmento1,MEM,REG);
        break;
    }

    switch (op2)
    {
        case 0: //op2 es memoria
            leemem(&posmem2,MEM,TDS,REG);
            leedemem(&valor2,MEM,posmem2);
        break;
        case 1: //op2 es inmediato
            leeinm(&aux2,MEM,REG);
            valor2 = aux2;
        break;
        case 2: //op2 es registro
            leereg(&registro2, &segmento2,MEM,REG);
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
        if (op2 == 0) //op2 es memoria
            for (i = 0 ; i < 4 ; i++)
                MEM[posmem1+i] = MEM[posmem2+i];
        else
            if (op2 == 1) // op2 es inmediato
            {
                MEM[posmem1+2] = valor2>>8;
                MEM[posmem1+3] = valor2;
            }
            else // op2 es registro
            {
                MEM[posmem1] = valor2>>24;
                MEM[posmem1+1] = valor2>>16;
                MEM[posmem1+2] = valor2>>8;
                MEM[posmem1+3] = valor2;
            }
    else //op1 es registro
    {
        switch(segmento1)
        {
            case 0:
                REG[registro1] = valor2;
            break;
            case 1:
                REG[registro1] &= 0xFFFFFF00;
                if (op2 != 2)
                    if ((((valor2>>8)&0x01) == 1))
                        valor2 |= 0xFFFFFF00;
                    else
                        valor2 &= 0x000000FF;
                REG[registro1] |= valor2;
            break;
            case 2:
                REG[registro1] &= 0xFFFF00FF;
                if (op2 != 2)
                    if ((((valor2>>8)&0x01) == 1))
                        valor2 |= 0xFFFFFF00;
                    else
                        valor2 &= 0x000000FF;
                REG[registro1] |= valor2<<8;
            break;
            case 3:
                REG[registro1] &= 0xFFFF0000;
                if (op2 != 2)
                    if ((((valor2>>16)&0x01) == 1))
                        valor2 |= 0xFFFF0000;
                    else
                        valor2 &= 0x0000FFFF;
                REG[registro1] |= valor2;
        }
    }
    if(op1 == 0)
        if(op2 == 0) //memoria a memoria
            printf("MOV [%d], [%d] \n",posmem1-TDS[1].base, posmem2-TDS[1].base);
        else
            if(op2 == 1) // inmediato a memoria
                printf("MOV [%d], %d \n",posmem1-TDS[1].base, valor2);
            else // registro a memoria
                printf("MOV [%d], %d \n",posmem1-TDS[1].base, registro2);
    else
        if(op2 == 0) //memoria a registro
            printf("MOV %d, [%d] \n", registro1, posmem2-TDS[1].base);
        else
            if(op2 == 1) // inmediato a registro
                printf("MOV %d, %d \n",registro1, valor2);
            else // registro a registro
                printf("MOV %d, %d \n",registro1, registro2);

}

void ADD(char op1, char op2, char MEM[], int REG[], TRTDS TDS[]) // todo roto
{
    char registro1 = 0, registro2 = 0, segmento1 = 0, segmento2 = 0;
    short int posmem1 = 0, posmem2 = 0, aux2 = 0, a = 0;
    int j = 0, i = 0, valor1 = 0, valor2 = 0;

    switch (op1)
    {
        case 0: // op1 es memoria
            leemem(&a, MEM, TDS, REG);
            posmem1 = a;
        break;
        case 2: // op1 es registro
            leereg(&registro1, &segmento1, MEM, REG);
        break;
    }
    switch (op2)
    {
        case 0: //op2 es memoria
            leemem(&a, MEM, TDS, REG);
            posmem2 = a;
            leedemem(&valor2, MEM, posmem2);
        break;
        case 1: //op2 es inmediato
            leeinm(&aux2, MEM, REG);
            valor2 = aux2;
            valor2 = corrimiento(valor2, 16, 16);
        break;
        case 2: //op2 es registro
            leereg(&registro2, &segmento2, MEM, REG);
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
    {
        leedemem(&valor1, MEM, posmem1);
        valor1 += valor2;
        MEM[posmem1+3] = valor1;
        MEM[posmem1+2] = valor1>>8;
        MEM[posmem1+1] = valor1>>16;
        MEM[posmem1] = valor1>>24;
    }
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
        valor1 = REG[registro1];
    }

    if(valor1 > 0)
        REG[8] = 0;
    else
        if(valor1 < 0)
            REG[8] = 0x80000000;
        else //valor1 == 0
            REG[8] = 0x40000000;

    if(op1 == 0)
        if(op2 == 0) //memoria a memoria
            printf("ADD [%d], [%d] \n",posmem1-TDS[1].base, posmem2-TDS[1].base);
        else
            if(op2 == 1) // inmediato a memoria
                printf("ADD [%d], %d \n", posmem1-TDS[1].base, valor2);
            else // registro a memoria
                printf("ADD [%d], %d \n", posmem1-TDS[1].base, registro2);
    else
        if(op2 == 0) //memoria a registro
            printf("ADD %d, [%d] \n", registro1, posmem2-TDS[1].base);
        else
            if(op2 == 1) // inmediato a registro
                printf("ADD %d, %d \n",registro1, valor2);
            else // registro a registro
                printf("ADD %d, %d \n",registro1, registro2);
}

void SUB(char op1, char op2, char MEM[], int REG[], TRTDS TDS[])
{
    char registro1 = 0, registro2 = 0, segmento1 = 0, segmento2 = 0;
    short int posmem1 = 0, posmem2 = 0, aux2 = 0, a = 0;
    int j = 0, i = 0, valor1 = 0, valor2 = 0;

    switch (op1)
    {
        case 0: // op1 es memoria
            leemem(&a,MEM,TDS,REG);
            posmem1=a;
        break;
        case 2: // op1 es registro
            leereg(&registro1, &segmento1,MEM,REG);
        break;
    }
    switch (op2)
    {
        case 0: //op2 es memoria
            leemem(&a,MEM,TDS,REG);
            posmem2=a;
            leedemem(&valor2,MEM,posmem2);
        break;
        case 1: //op2 es inmediato
            leeinm(&aux2,MEM,REG);
            valor2=aux2;
            valor2 = corrimiento(valor2, 16, 16);
        break;
        case 2: //op2 es registro
            leereg(&registro2, &segmento2,MEM,REG);
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
    {
        leedemem(&valor1,MEM,posmem1);
        valor1 -= valor2;
        MEM[posmem1+3] = valor1;
        MEM[posmem1+2] = valor1>>8;
        MEM[posmem1+1] = valor1>>16;
        MEM[posmem1] = valor1>>24;
    }
    else //op1 es registro
    {
        switch(segmento1)
        {
            case 0:
                REG[registro1] -= valor2;
            break;
            case 1:
                REG[registro1] -= valor2;
            break;
            case 2:
                REG[registro1] -= valor2<<8;
            break;
            case 3:
                REG[registro1] -= valor2;
        }
        valor1 = REG[registro1];
    }

    if(valor1 > 0)
        REG[8] = 0;
    else
        if(valor1 < 0)
            REG[8] = 0x80000000;
        else //valor1 == 0
            REG[8] = 0x40000000;

    if(op1 == 0)
        if(op2 == 0) //memoria a memoria
            printf("SUB [%d], [%d] \n",posmem1-TDS[1].base, posmem2-TDS[1].base);
        else
            if(op2 == 1) // inmediato a memoria
                printf("SUB [%d], %d \n",posmem1-TDS[1].base, valor2);
            else // registro a memoria
                printf("SUB [%d], %d \n",posmem1-TDS[1].base, registro2);
    else
        if(op2 == 0) //memoria a registro
            printf("SUB %d, [%d] \n", registro1, posmem2-TDS[1].base);
        else
            if(op2 == 1) // inmediato a registro
                printf("SUB %d, %d \n",registro1, valor2);
            else // registro a registro
                printf("SUB %d, %d \n",registro1, registro2);

}

void SWAP(char op1, char op2, char MEM[], int REG[], TRTDS TDS[])
{
    char auxV1[4], auxV2[4], auxchar = 0, registro1 = 0, registro2 = 0, segmento1 = 0, segmento2 = 0;
    short int posmem1 = 0, posmem2 = 0, aux2 = 0, inmediato2 = 0;
    int byte = 0, j = 0, i = 0, valor1 = 0, valor2 = 0;


    switch (op1)
    {
        case 0: // op1 es memoria
            leemem(&posmem1,MEM,TDS,REG);
        break;
        case 2: // op1 es registro
            leereg(&registro1, &segmento1,MEM,REG);
        break;
    }

    switch (op2)
    {
        case 0: //op2 es memoria
            leemem(&posmem2,MEM,TDS,REG);
            leedemem(&valor2,MEM,posmem2);
            valor2 = corrimiento(valor2, 16, 16);
        break;
        case 2: //op2 es registro
            leereg(&registro2, &segmento2,MEM,REG);
        break;
    }
    printf("MEM[%d] = %d - MEM[%d] = %d\n", posmem1, valor1, posmem2, valor2);
    if(op1 == 0) // op1 es memoria
        if (op2 == 0) // op2 es memoria
        {
            for(i = 0 ; i < 4 ; i++)
            {
                auxV1[i] = MEM[posmem1+i];
                auxV2[i] = MEM[posmem2+i];
            }
            for(i = 0 ; i < 4 ; i++)
            {
                MEM[posmem2+i] = auxV1[i];
                MEM[posmem1+i] = auxV2[i];
            }
            for(i = 0 ; i < 4 ; i++)
            {
                valor1 = valor1<<8;
                valor1 = MEM[posmem1+i];
            }
            for(i = 0 ; i < 4 ; i++)
            {
                valor2 = valor2<<8;
                valor2 = MEM[posmem2+i];
            }
        }
        else // op2 es registro
        {
            for(i = 0 ; i < 4 ; i++)
                auxV1[i] = MEM[posmem1+i];
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
            MEM[posmem1] = (valor2>>24)&0xFF;
            MEM[posmem1+1] = (valor2>>16)&0xFF;
            MEM[posmem1+2] = (valor2>>8)&0xFF;
            MEM[posmem1+3] = (valor2)&0xFF;

            for(i = 0 ; i < 4 ; i++)
            {
                REG[registro2]<<8;
                REG[registro2] = auxV1[i];
            }
        }
    else // op1 es registro
        if (op2 = 0) // op2 es memoria
        {
            for(i = 0 ; i < 4 ; i++)
                auxV2[i] = MEM[posmem2+i];
            valor1 = REG[registro1];
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
            MEM[posmem2] = (valor1>>24)&0xFF;
            MEM[posmem2+1] = (valor1>>16)&0xFF;
            MEM[posmem2+2] = (valor1>>8)&0xFF;
            MEM[posmem2+3] = (valor1)&0xFF;

            for(i = 0 ; i < 4 ; i++)
            {
                REG[registro1]<<8;
                REG[registro1] = auxV2[i];
            }
        }
        else // op2 es registro
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
        printf("MEM[%d] = %d - MEM[%d] = %d\n", posmem1, valor1, posmem2, valor2);
    if(op1 == 0)
        if(op2 == 0) //memoria a memoria
            printf("SWAP [%d], [%d] \n",posmem1-TDS[1].base, posmem2-TDS[1].base);
        else // registro a memoria
            printf("SWAP [%d], %d \n",posmem1-TDS[1].base, registro2);
    else
        if(op2 == 0) //memoria a registro
            printf("SWAP %d, [%d] \n", registro1, posmem2-TDS[1].base);
        else // registro a registro
            printf("SWAP %d, %d \n",registro1, registro2);
}

void MUL(char op1, char op2, char MEM[], int REG[], TRTDS TDS[])
{
    char registro1 = 0, registro2 = 0, segmento1 = 0, segmento2 = 0;
    short int posmem1 = 0, posmem2 = 0, aux2 = 0, a = 0;
    int byte = 0, j = 0, i = 0, valor1 = 0, valor2 = 0;

    switch (op1)
    {
        case 0: // op1 es memoria
            leemem(&a,MEM,TDS,REG);
            posmem1=a;
        break;
        case 2: // op1 es registro
            leereg(&registro1, &segmento1,MEM,REG);
        break;
    }

    switch (op2)
    {
        case 0: //op2 es memoria
            leemem(&a,MEM,TDS,REG);
            posmem2=a;
            leedemem(&valor2,MEM,posmem2);
        break;
        case 1: //op2 es inmediato
            leeinm(&aux2,MEM,REG);
            valor2=aux2;
            valor2 = corrimiento(valor2, 16, 16);
        break;
        case 2: //op2 es registro
            leereg(&registro2, &segmento2,MEM,REG);
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
    {
        leedemem(&valor1,MEM,posmem1);
        //printf("add valor1: %u\n", valor1);
        //printf("add valor2: %u\n", valor2);
        valor1 *= valor2;
        MEM[posmem1+3] = valor1;
        MEM[posmem1+2] = valor1>>8;
        MEM[posmem1+1] = valor1>>16;
        MEM[posmem1] = valor1>>24;
    }
    else //op1 es registro
    {
        switch(segmento1)
        {
            case 0:
                REG[registro1] *= valor2;
            break;
            case 1:
                REG[registro1] *= valor2;
            break;
            case 2:
                REG[registro1] *= valor2<<8;
            break;
            case 3:
                REG[registro1] *= valor2;
        }
        valor1 = REG[registro1];
    }

    if(valor1 > 0)
        REG[8] = 0;
    else
        if(valor1 < 0)
            REG[8] = 0x80000000;
        else //valor1 == 0
            REG[8] = 0x40000000;

    if(op1 == 0)
        if(op2 == 0) //memoria a memoria
            printf("MUL [%d], [%d] \n",posmem1-TDS[1].base, posmem2-TDS[1].base);
        else
            if(op2 == 1) // inmediato a memoria
                printf("MUL [%d], %d \n",posmem1-TDS[1].base, valor2);
            else // registro a memoria
                printf("MUL [%d], %d \n",posmem1-TDS[1].base, registro2);
    else
        if(op2 == 0) //memoria a registro
            printf("MUL %d, [%d] \n", registro1, posmem2-TDS[1].base);
        else
            if(op2 == 1) // inmediato a registro
                printf("MUL %d, %d \n",registro1, valor2);
            else // registro a registro
                printf("MUL %d, %d \n",registro1, registro2);
}

void DIV(char op1, char op2, char MEM[], int REG[], TRTDS TDS[])
{
    char registro1 = 0, registro2 = 0, segmento1 = 0, segmento2 = 0;
    short int posmem1 = 0, posmem2 = 0, aux2 = 0, a = 0;
    int j = 0, i = 0, valor1 = 0, valor2 = 0, aux;

    switch (op1)
    {
        case 0: // op1 es memoria
            leemem(&a,MEM,TDS,REG);
            posmem1 = a;
        break;
        case 2: // op1 es registro
            leereg(&registro1, &segmento1,MEM,REG);
        break;
    }

    switch (op2)
    {
        case 0: //op2 es memoria
            leemem(&a, MEM, TDS, REG);
            posmem2 = a;
            leedemem(&valor2, MEM, posmem2);
        break;
        case 1: //op2 es inmediato
            leeinm(&aux2, MEM, REG);
            valor2 = aux2;
            valor2 = corrimiento(valor2, 16, 16);
        break;
        case 2: //op2 es registro
            leereg(&registro2, &segmento2, MEM, REG);
            switch(segmento2)
            {
                case 0:
                    valor2 = REG[registro2];
                break;
                case 1:
                    valor2 = corrimiento(REG[registro2], 24, 24);
                break;
                case 2:
                    valor2 = corrimiento(REG[registro2], 16, 24);
                break;
                case 3:
                    valor2 = corrimiento(REG[registro2], 16, 16);
                break;
            }
        break;
    }
    if (valor2 == 0)
    {
        printf("Division por cero\n");
        REG[5] = baseds(TDS, REG) - 1;
    }
    else
        if(op1 == 0) //op1 es memoria
        {
            leedemem(&valor1,MEM,posmem1);
            REG[9] = valor1 % valor2;
            valor1 /= valor2;
            MEM[posmem1+3] = valor1;
            MEM[posmem1+2] = valor1>>8;
            MEM[posmem1+1] = valor1>>16;
            MEM[posmem1] = valor1>>24;
        }
        else //op1 es registro
        {
            REG[9] = REG[registro1] % valor2;
            switch(segmento1)
            {
                case 0:
                    REG[registro1] /= valor2;
                break;
                case 1:
                    REG[registro1] /= valor2;
                break;
                case 2:
                    REG[registro1] /= valor2<<8;
                break;
                case 3:
                    REG[registro1] /= valor2;
            }
            valor1 = REG[registro1];
        }

    if(valor1 > 0)
        REG[8] = 0;
    else
        if(valor1 < 0)
            REG[8] = 0x80000000;
        else //valor1 == 0
            REG[8] = 0x40000000;

    if(op1 == 0)
        if(op2 == 0) //memoria a memoria
            printf("DIV [%d], [%d] \n",posmem1-TDS[1].base, posmem2-TDS[1].base);
        else
            if(op2 == 1) // inmediato a memoria
                printf("DIV [%d], %d \n",posmem1-TDS[1].base, valor2);
            else // registro a memoria
                printf("DIV [%d], %d \n",posmem1-TDS[1].base, registro2);
    else
        if(op2 == 0) //memoria a registro
            printf("DIV %d, [%d] \n", registro1, posmem2-TDS[1].base);
        else
            if(op2 == 1) // inmediato a registro
                printf("DIV %d, %d \n",registro1, valor2);
            else // registro a registro
                printf("DIV %d, %d \n",registro1, registro2);
}

void CMP(char op1, char op2, char MEM[], int REG[], TRTDS TDS[])
{
    char registro1 = 0, registro2 = 0, segmento1 = 0, segmento2 = 0;
    short int posmem1 = 0, posmem2 = 0, aux1 = 0, aux2 = 0;
    int valor1 = 0, valor2 = 0;

    switch (op1)
    {
        case 0: // op1 es memoria
            leemem(&posmem1,MEM,TDS,REG);
            leedemem(&valor1,MEM,posmem1);
            printf("cmp posmem1: %d - valor1: %d\n",posmem1,valor1);
        break;
        case 1:
            leeinm(&aux1, MEM, REG);
            valor1 = aux1;
        break;
        case 2: // op1 es registro
            leereg(&registro1,&segmento1,MEM,REG);
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
            leemem(&posmem2, MEM, TDS, REG);
            leedemem(&valor2,MEM,posmem2);
            printf("cmp posmem2: %d - valor2: %d\n",posmem2,valor2);
        break;
        case 1: //op2 es inmediato
            leeinm(&aux2, MEM, REG);
            valor2 = aux2;
        break;
        case 2: //op2 es registro
            leereg(&registro2, &segmento2, MEM, REG);
            switch(segmento2)
            {
                case 0:
                    valor2 = REG[registro2];
                break;
                case 1:
                    valor2 = corrimiento(REG[registro2], 24, 24);
                break;
                case 2:
                    valor2 = corrimiento(REG[registro2], 16, 24);
                break;
                case 3:
                    valor2 = corrimiento(REG[registro2], 16, 16);
                break;
            }
        break;
    }
    printf("cmp valor1: %d / cmp valor2: %d\n", valor1, valor2);
    valor1 -= valor2;
    if(valor1 > 0)
        REG[8] = 0;
    else
        if(valor1 < 0)
            REG[8] = 0x80000000;
        else //valor1 == 0
            REG[8] = 0x40000000;
    printf("CMP %d, %d \n",valor1+valor2,valor2);
    printf("N: %d - Z: %d\n",(REG[8]>>31)&01, (REG[8]>>30)&01);
}

void SHL(char op1, char op2, char MEM[], int REG[], TRTDS TDS[])
{
    char registro1 = 0, registro2 = 0, segmento1 = 0, segmento2 = 0;
    short int posmem1 = 0, posmem2 = 0, aux2 = 0, a = 0;
    int byte = 0, j = 0, i = 0, valor1 = 0, valor2 = 0;

    switch (op1)
    {
        case 0: // op1 es memoria
            leemem(&a,MEM,TDS,REG);
            posmem1=a;
            leedemem(&valor1,MEM,posmem1);
        break;
        case 2: // op1 es registro
            leereg(&registro1, &segmento1,MEM,REG);
            valor1 = REG[registro1];
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
        break;
    }
    switch (op2)
    {
        case 0: //op2 es memoria
            leemem(&a,MEM,TDS,REG);
            posmem2=a;
            leedemem(&valor2,MEM,posmem2);
        break;
        case 1: //op2 es inmediato
            leeinm(&aux2,MEM,REG);
            valor2=aux2;
            valor2 = corrimiento(valor2, 16, 16);
        break;
        case 2: //op2 es registro
            leereg(&registro2, &segmento2,MEM,REG);
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
    {
        //printf("add valor1: %u\n", valor1);
        //printf("add valor2: %u\n", valor2);
        valor1=valor1<<valor2;
        MEM[posmem1+3] = valor1;
        MEM[posmem1+2] = valor1>>8;
        MEM[posmem1+1] = valor1>>16;
        MEM[posmem1] = valor1>>24;
    }
    else //op1 es registro
    {
        valor1=valor1<<valor2;
        switch(segmento1)
        {
            case 0:
                REG[registro1] = valor1;
            break;
            case 1:
                REG[registro1] = valor1&0xFF;
            break;
            case 2:
                REG[registro1] = (valor2&0xFF)<<8;
            break;
            case 3:
                REG[registro1] = valor2&0xFFFF;
        }
        valor1 = REG[registro1];
    }

    if(valor1 > 0)
        REG[8] = 0;
    else
        if(valor1 < 0)
            REG[8] = 0x80000000;
        else //valor1 == 0
            REG[8] = 0x40000000;

    if(op1 == 0)
        if(op2 == 0) //memoria a memoria
            printf("SHL [%d], [%d] \n",posmem1-TDS[1].base, posmem2-TDS[1].base);
        else
            if(op2 == 1) // inmediato a memoria
                printf("SHL [%d], %d \n",posmem1-TDS[1].base, valor2);
            else // registro a memoria
                printf("SHL [%d], %d \n",posmem1-TDS[1].base, registro2);
    else
        if(op2 == 0) //memoria a registro
            printf("SHL %d, [%d] \n", registro1, posmem2-TDS[1].base);
        else
            // registro a registro
            printf("SHL %d, %d \n",registro1, registro2);
}

void SHR(char op1, char op2, char MEM[], int REG[], TRTDS TDS[])
{
    char registro1 = 0, registro2 = 0, segmento1 = 0, segmento2 = 0;
    short int posmem1 = 0, posmem2 = 0, aux2 = 0, a = 0;
    int byte = 0, j = 0, i = 0, valor1 = 0, valor2 = 0;

    switch (op1)
    {
        case 0: // op1 es memoria
            leemem(&a,MEM,TDS,REG);
            posmem1=a;
        break;
        case 2: // op1 es registro
            leereg(&registro1, &segmento1,MEM,REG);
        break;
    }
    switch (op2)
    {
        case 0: //op2 es memoria
            leemem(&a,MEM,TDS,REG);
            posmem2=a;
            leedemem(&valor2,MEM,posmem2);
        break;
        case 1: //op2 es inmediato
            leeinm(&aux2,MEM,REG);
            valor2=aux2;
            valor2 = corrimiento(valor2, 16, 16);
        break;
        case 2: //op2 es registro
            leereg(&registro2, &segmento2,MEM,REG);
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
    {
        leedemem(&valor1,MEM,posmem1);
        valor1 += valor2;
        MEM[posmem1+3] = valor1;
        MEM[posmem1+2] = valor1>>8;
        MEM[posmem1+1] = valor1>>16;
        MEM[posmem1] = valor1>>24;
    }
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
        valor1 = REG[registro1];
    }

    if(valor1 > 0)
        REG[8] = 0;
    else
        if(valor1 < 0)
            REG[8] = 0x80000000;
        else //valor1 == 0
            REG[8] = 0x40000000;

    if(op1 == 0)
        if(op2 == 0) //memoria a memoria
            printf("SHR [%d], [%d] \n",posmem1-TDS[1].base, posmem2-TDS[1].base);
        else
            if(op2 == 1) // inmediato a memoria
                printf("SHR [%d], %d \n",posmem1-TDS[1].base, valor2);
            else // registro a memoria
                printf("SHR [%d], %d \n",posmem1-TDS[1].base, registro2);
    else
        if(op2 == 0) //memoria a registro
            printf("SHR %d, [%d] \n", registro1, posmem2-TDS[1].base);
        else
            if(op2 == 1) // inmediato a registro
                printf("SHR %d, %d \n",registro1, valor2);
            else // registro a registro
                printf("SHR %d, %d \n",registro1, registro2);
}

void AND(char op1, char op2, char MEM[], int REG[], TRTDS TDS[])
{
    char registro1 = 0, registro2 = 0, segmento1 = 0, segmento2 = 0;
    short int posmem1 = 0, posmem2 = 0, aux2 = 0, a = 0;
    int byte = 0, j = 0, i = 0, valor1 = 0, valor2 = 0;

    switch (op1)
    {
        case 0: // op1 es memoria
            leemem(&a,MEM,TDS,REG);
            posmem1=a;
        break;
        case 2: // op1 es registro
            leereg(&registro1, &segmento1,MEM,REG);
        break;
    }
    switch (op2)
    {
        case 0: //op2 es memoria
            leemem(&a,MEM,TDS,REG);
            posmem2=a;
            leedemem(&valor2,MEM,posmem2);
        break;
        case 1: //op2 es inmediato
            leeinm(&aux2,MEM,REG);
            valor2=aux2;
            valor2 = corrimiento(valor2, 16, 16);
        break;
        case 2: //op2 es registro
            leereg(&registro2, &segmento2,MEM,REG);
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
    {
        leedemem(&valor1,MEM,posmem1);
        //printf("add valor1: %u\n", valor1);
        //printf("add valor2: %u\n", valor2);
        valor1 &= valor2;
        MEM[posmem1+3] = valor1;
        MEM[posmem1+2] = valor1>>8;
        MEM[posmem1+1] = valor1>>16;
        MEM[posmem1] = valor1>>24;
    }
    else //op1 es registro
    {
        switch(segmento1)
        {
            case 0:
                REG[registro1] &= valor2;
            break;
            case 1:
                REG[registro1] &= valor2;
            break;
            case 2:
                REG[registro1] &= valor2<<8;
            break;
            case 3:
                REG[registro1] &= valor2;
        }
        valor1 = REG[registro1];
    }

    if(valor1 > 0)
        REG[8] = 0;
    else
        if(valor1 < 0)
            REG[8] = 0x80000000;
        else //valor1 == 0
            REG[8] = 0x40000000;

    if(op1 == 0)
        if(op2 == 0) //memoria a memoria
            printf("AND [%d], [%d] \n",posmem1-TDS[1].base, posmem2-TDS[1].base);
        else
            if(op2 == 1) // inmediato a memoria
                printf("AND [%d], %d \n",posmem1-TDS[1].base, valor2);
            else // registro a memoria
                printf("AND [%d], %d \n",posmem1-TDS[1].base, registro2);
    else
        if(op2 == 0) //memoria a registro
            printf("AND %d, [%d] \n", registro1, posmem2-TDS[1].base);
        else
            if(op2 == 1) // inmediato a registro
                printf("AND %d, %d \n",registro1, valor2);
            else // registro a registro
                printf("AND %d, %d \n",registro1, registro2);


}

void OR(char op1, char op2, char MEM[], int REG[], TRTDS TDS[])
{
    char registro1 = 0, registro2 = 0, segmento1 = 0, segmento2 = 0;
    short int posmem1 = 0, posmem2 = 0, aux2 = 0, a = 0;
    int byte = 0, j = 0, i = 0, valor1 = 0, valor2 = 0;

    switch (op1)
    {
        case 0: // op1 es memoria
            leemem(&a,MEM,TDS,REG);
            posmem1=a;
        break;
        case 2: // op1 es registro
            leereg(&registro1, &segmento1,MEM,REG);
        break;
    }

    switch (op2)
    {
        case 0: //op2 es memoria
            leemem(&a,MEM,TDS,REG);
            posmem2=a;
            leedemem(&valor2,MEM,posmem2);
        break;
        case 1: //op2 es inmediato
            leeinm(&aux2,MEM,REG);
            valor2=aux2;
            valor2 = corrimiento(valor2, 16, 16);
        break;
        case 2: //op2 es registro
            leereg(&registro2, &segmento2,MEM,REG);
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
    {
        leedemem(&valor1,MEM,posmem1);
        valor1 |= valor2;
        MEM[posmem1+3] = valor1;
        MEM[posmem1+2] = valor1>>8;
        MEM[posmem1+1] = valor1>>16;
        MEM[posmem1] = valor1>>24;
    }
    else //op1 es registro
    {
        switch(segmento1)
        {
            case 0:
                REG[registro1] |= valor2;
            break;
            case 1:
                REG[registro1] |= valor2;
            break;
            case 2:
                REG[registro1] |= valor2<<8;
            break;
            case 3:
                REG[registro1] |= valor2;
        }
        valor1 = REG[registro1];
    }

    if(valor1 > 0)
        REG[8] = 0;
    else
        if(valor1 < 0)
            REG[8] = 0x80000000;
        else //valor1 == 0
            REG[8] = 0x40000000;

    if(op1 == 0)
        if(op2 == 0) //memoria a memoria
            printf("OR [%d], [%d] \n",posmem1-TDS[1].base, posmem2-TDS[1].base);
        else
            if(op2 == 1) // inmediato a memoria
                printf("OR [%d], %d \n",posmem1-TDS[1].base, valor2);
            else // registro a memoria
                printf("OR [%d], %d \n",posmem1-TDS[1].base, registro2);
    else
        if(op2 == 0) //memoria a registro
            printf("OR %d, [%d] \n", registro1, posmem2-TDS[1].base);
        else
            if(op2 == 1) // inmediato a registro
                printf("OR %d, %d \n",registro1, valor2);
            else // registro a registro
                printf("OR %d, %d \n",registro1, registro2);

}

void XOR(char op1, char op2, char MEM[], int REG[], TRTDS TDS[])
{
    char registro1 = 0, registro2 = 0, segmento1 = 0, segmento2 = 0;
    short int posmem1 = 0, posmem2 = 0, aux2 = 0, a = 0;
    int j = 0, i = 0, valor1 = 0, valor2 = 0;

    switch (op1)
    {
        case 0: // op1 es memoria
            leemem(&a,MEM,TDS,REG);
            posmem1=a;
        break;
        case 2: // op1 es registro
            leereg(&registro1, &segmento1,MEM,REG);
        break;
    }
    switch (op2)
    {
        case 0: //op2 es memoria
            leemem(&a,MEM,TDS,REG);
            posmem2=a;
            leedemem(&valor2,MEM,posmem2);
        break;
        case 1: //op2 es inmediato
            leeinm(&aux2,MEM,REG);
            valor2=aux2;
            valor2 = corrimiento(valor2, 16, 16);
        break;
        case 2: //op2 es registro
            leereg(&registro2, &segmento2,MEM,REG);
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
    {
        leedemem(&valor1,MEM,posmem1);
        valor1 ^= valor2;
        MEM[posmem1+3] = valor1;
        MEM[posmem1+2] = valor1>>8;
        MEM[posmem1+1] = valor1>>16;
        MEM[posmem1] = valor1>>24;
    }
    else //op1 es registro
    {
        switch(segmento1)
        {
            case 0:
                REG[registro1] ^= valor2;
            break;
            case 1:
                REG[registro1] ^= valor2;
            break;
            case 2:
                REG[registro1] ^= valor2<<8;
            break;
            case 3:
                REG[registro1] ^= valor2;
        }
        valor1 = REG[registro1];
    }

    if(valor1 > 0)
        REG[8] = 0;
    else
        if(valor1 < 0)
            REG[8] = 0x80000000;
        else //valor1 == 0
            REG[8] = 0x40000000;

    if(op1 == 0)
        if(op2 == 0) //memoria a memoria
            printf("XOR [%d], [%d] \n",posmem1-TDS[1].base, posmem2-TDS[1].base);
        else
            if(op2 == 1) // inmediato a memoria
                printf("XOR [%d], %d \n",posmem1-TDS[1].base, valor2);
            else // registro a memoria
                printf("XOR [%d], %d \n",posmem1-TDS[1].base, registro2);
    else
        if(op2 == 0) //memoria a registro
            printf("XOR %d, [%d] \n", registro1, posmem2-TDS[1].base);
        else
            if(op2 == 1) // inmediato a registro
                printf("XOR %d, %d \n",registro1, valor2);
            else // registro a registro
                printf("XOR %d, %d \n",registro1, registro2);
}

void SYS(char op1, char op2, char MEM[], int REG[], TRTDS TDS[])
{
    int eax = 0, posmem = 0, i, j;
    short int cantidad = 0, al = 0, cl = 0, ch = 0, operacion = 0;
    int aux = 0, aux2 = 0;

    al = REG[10]&0xFF; //al
    ch = (REG[12]>>8) & 0xFF; //ch
    cl = REG[12] & 0xFF; //cl
    posmem = baseds(TDS, REG) + REG[13]&0xFFFF; //dx


    lee2byte(&operacion,MEM,REG);
    switch(operacion)
    {
        case 1: //lectura
            switch(al)
            {
                case 1: // decimal
                    for(i = 0 ; i < cl ; i++)
                    {
                        printf("Ingrese datos (decimal): ");
                        scanf("%d", &aux);

                        for(j = (ch-1) ; j >=0 ; j--)
                        {
                            MEM[posmem+j] = aux;
                            aux = aux>>8;
                        }
                        int valor1;
                        leedemem(&valor1,MEM,posmem);
                        posmem += 4;
                    }
                break;
                case 2: // caracteres
                    for(i = 0 ; i < cl ; i++)
                    {
                        printf("Ingrese datos (caracter): ");
                        scanf("%c", &aux);
                        for(j = 0 ; j < ch ; j++)
                        {
                            MEM[posmem+j] = aux;
                            aux = aux>>8;
                        }
                        posmem += 4;
                    }
                break;
                case 4: // octal
                    for(i = 0 ; i < cl ; i++)
                    {
                        printf("Ingrese datos (octal): ");
                        scanf("%o", &aux);
                        for(j = 0 ; j < ch ; j++)
                        {
                            MEM[posmem+j] = aux;
                            aux = aux>>8;
                        }
                        posmem += 4;
                    }
                break;
                case 8: // hexadecimal
                    for(i = 0 ; i < cl ; i++)
                    {
                        printf("Ingrese datos (hexadecimal): ");
                        scanf("%x", &aux);
                        for(j = 0 ; j < ch ; j++)
                        {
                            MEM[posmem+j] = aux;
                            aux = aux>>8;
                        }
                        posmem += 4;
                    }
                break;
            }
        break;
        case 2: // imprimir
            switch(al)
            {
                case 1: // decimal
                    for(i = 0 ; i < cl ; i++)
                    {
                        for(j = 0 ; j < ch ; j++)
                        {
                            aux2 = MEM[posmem+j];
                            mascaras(&aux2, 0);
                            aux |= aux2<<8*(ch-j-1);
                        }
                        posmem += 4;
                        printf("%d\n", aux);
                    }
                break;
                case 2: // caracteres
                    for(i = 0 ; i < cl ; i++)
                    {
                        for(j = 0 ; j < ch ; j++)
                        {
                            aux2 = MEM[posmem+j];
                            mascaras(&aux2, 0);
                            aux |= aux2<<8*(ch-j-1);
                        }
                        posmem += 4;
                        printf("%c\n", aux);
                    }
                break;
                case 4: // octal
                    for(i = 0 ; i < cl ; i++)
                    {
                        for(j = 0 ; j < ch ; j++)
                        {
                            aux2 = MEM[posmem+j];
                            mascaras(&aux2, 0);
                            aux |= aux2<<8*(ch-j-1);
                        }
                        posmem += 4;
                        printf("%o\n", aux);
                    }
                break;
                case 8: // hexadecimal
                    for(i = 0 ; i < cl ; i++)
                    {
                        for(j = 0 ; j < ch ; j++)
                        {
                            aux2 = MEM[posmem+j];
                            mascaras(&aux2, 0);
                            aux |= aux2<<8*(ch-j-1);
                        }
                        posmem += 4;
                        printf("%x\n", aux);
                    }
                break;
            }
        break;
    }
}

void JMP(char op1, char op2, char MEM[], int REG[], TRTDS TDS[])
{
    int N = (REG[8]>>31) & 0x01;
    short int inmediato;

    lee2byte(&inmediato,MEM,REG);
    printf("JMP %d\n", inmediato);
    REG[5] = inmediato;
}

void JZ(char op1, char op2, char MEM[], int REG[], TRTDS TDS[])
{
    int Z = (REG[8]>>30) & 0x01;
    short int inmediato;

    lee2byte(&inmediato,MEM,REG);
    if(Z == 1)
    {
        REG[5] = inmediato;
        printf("JZ %d\n",inmediato);
    }
    else
        printf("JZ no salta\n");
}

void JP(char op1, char op2, char MEM[], int REG[], TRTDS TDS[])
{
    int N = (REG[8]>>31) & 0x01;
    short int inmediato;

    lee2byte(&inmediato,MEM,REG);
    if(N == 0)
    {
        REG[5] = inmediato;
        printf("JP %d\n",inmediato);
    }
    else
        printf("JP no salta\n");
}

void JN(char op1, char op2, char MEM[], int REG[], TRTDS TDS[])
{
    int N = (REG[8]>>31) & 0x01;
    short int inmediato;

    lee2byte(&inmediato,MEM,REG);
    if(N == 1)
    {
        REG[5] = inmediato;
        printf("JN %d\n",inmediato);
    }
    else
        printf("JN no salta\n");
}

void JNZ(char op1, char op2, char MEM[], int REG[], TRTDS TDS[])
{
    int Z = (REG[8]>>30) & 0x01;
    short int inmediato;

    lee2byte(&inmediato,MEM,REG);
    if(Z == 0)
    {
        REG[5] = inmediato;
        printf("JNZ %d\n",inmediato);
    }
    else
        printf("JNZ no salta\n");
}

void JNP(char op1, char op2, char MEM[], int REG[], TRTDS TDS[])
{
    int N = (REG[8]>>31) & 0x01, Z = (REG[8]>>30) & 0x01;
    short int inmediato;

    lee2byte(&inmediato,MEM,REG);
    if(N == 1 || Z == 1 )
    {
        REG[5] = inmediato;
        printf("JNP %d\n",inmediato);
    }
    else
        printf("JNP no salta\n");
}

void JNN(char op1, char op2, char MEM[], int REG[], TRTDS TDS[])
{
    int N = (REG[8]>>31) & 0x01;
    short int inmediato;

    lee2byte(&inmediato,MEM,REG);
    if(N == 0)
    {
        REG[5] = inmediato;
        printf("JNN %d\n",inmediato);
    }
    else
        printf("JNN no salta\n");
}

void LDL(char op1, char op2, char MEM[], int REG[], TRTDS TDS[])
{
    char auxchar;

}

void LDH(char op1, char op2, char MEM[], int REG[], TRTDS TDS[])
{

}

void RND(char op1, char op2, char MEM[], int REG[], TRTDS TDS[])
{

}

void NOT(char op1, char op2, char MEM[], int REG[], TRTDS TDS[])
{
    char registro1 = 0, registro2 = 0, segmento1 = 0, segmento2 = 0;
    short int posmem1 = 0, posmem2 = 0, aux2 = 0, a = 0;
    int byte = 0, j = 0, i = 0, valor1 = 0, valor2 = 0;

    switch (op1)
    {
        case 0: // op1 es memoria
            leemem(&a, MEM, TDS, REG);
            posmem1 = a;
        break;
        case 2: // op1 es registro
            leereg(&registro1, &segmento1, MEM, REG);
        break;
    }

    if(op1 == 0) //op1 es memoria
    {
        leedemem(&valor1, MEM, posmem1);
        valor1 = ~valor1;
        MEM[posmem1+3] = valor1;
        MEM[posmem1+2] = valor1>>8;
        MEM[posmem1+1] = valor1>>16;
        MEM[posmem1] = valor1>>24;
    }
    else //op1 es registro
    {
        valor1 = ~REG[registro1];
        switch(segmento1)
        {
            case 0: // ex
                REG[registro1] = valor1;
            break;
            case 1: // l
                valor1 &= 0xFF;
                REG[registro1] &= 0xFF;
                REG[registro1] |= valor1;
            break;
            case 2: // h
                valor1 &= 0xFF00;
                REG[registro1] &= 0xFF00;
                REG[registro1] |= valor1;
            break;
            case 3: // x
                valor1 &= 0xFFFF;
                REG[registro1] &= 0xFFFF;
                REG[registro1] |= valor1;
        }
        valor1 = REG[registro1];
    }

    if(valor1 > 0)
        REG[8] = 0;
    else
        if(valor1 < 0)
            REG[8] = 0x80000000;
        else //valor1 == 0
            REG[8] = 0x40000000;
}

void STOP(char op1, char op2, char MEM[], int REG[], TRTDS TDS[])
{
    printf("FIN\n");
}

void procesoDatos(char MEM[], TRTDS TDS[], int REG[], t_func funciones[])
{
    char inst, op1 = 0, op2, codop;
    while(op1 != 3)
    {
        inst = MEM[REG[5]++];
        codigos(inst, &op1, &op2, &codop);
        funciones[codop](op1, op2, MEM, REG, TDS);
        //dissasembler(op1, op2, codop)
    }
    scanf("");
}

void cargaFunciones(t_func funciones[])
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

int main()
{
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

