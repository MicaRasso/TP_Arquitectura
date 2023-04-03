#include <stdio.h>
#include <stdlib.h>

//tipo tabla descriptores de segmento
typedef struct
{
    short int base, size;
} TRTDS;



void lectura(char MEM[], int *TAM)
{
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

int main()
{
    int TAM, RAM = 16384;
    short int REG[16]; //( 0 = CS / 1 = DS / 5 = IP / 8 = CC / 9 = AC )
    char MEM[RAM];

    //inicializacion de variables y carga de memoria
    TRTDS TDS[8];
    lectura(MEM, &TAM);


    return 0;
}

