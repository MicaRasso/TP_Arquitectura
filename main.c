#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    short int base, size;
} TRTDS;



void lectura(char MEM[], int TAM)
{
    FILE *arch;
    char encabezado[6], c;
    arch = fopen ("D:/Documentos/Facultad/arquitecturaDeComputadoras/ejemplosasm/fibo.vmx","rb");
    int i;

    if (arch == NULL)
    {
        printf("No se pudo abrir el archivo\n");
        return 1;
    }

    fread(encabezado, sizeof(char), 5, arch);
    encabezado[5] = '\0';
    printf("%s\n", encabezado);

    fread(c, sizeof(char), 1, arch);
    printf("%d\n", c);

    fread(MEM, sizeof(char), TAM, arch);

    for (i=0 ; i < TAM ; i++)
        printf("%c\n", MEM[i]);

    fclose(arch);
}

int main()
{
    int TAM = 0, RAM = 16384;
    short int REG[16]; //( 0 = CS / 1 = DS / 5 = IP / 8 = CC / 9 = AC )
    char MEM[RAM];


    TRTDS TDS[8];
    lectura(MEM, TAM);
    return 0;
}

