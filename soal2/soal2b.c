#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <pthread.h> 
#include<stdlib.h>

typedef struct helper_t
{
    int A;
    int B;
    int row;
    int col;
} helper;

int matB[4][6];
unsigned long long result[4][6];
void *solution( void *ptr );
unsigned long long fact( int start, int end);

int main()
{

    key_t key = 1234;
    int (*matA)[6];

    int shmid = shmget(key, sizeof(int[4][6]), IPC_CREAT | 0666);
    matA = shmat(shmid, NULL, 0);

    printf("Masukkan Matriks 4x6:\n");
    for(int i = 0; i<4; i++)
    {
            for(int j = 0; j<6; j++)
            {
                    scanf("%d", &matB[i][j]);
            }
    }

    pthread_t tid[24];
    int  iret[24];
    helper *help = malloc(24 * sizeof(helper));

    for(int i = 0; i<4; i++)
    {
        for(int j = 0; j<6; j++)
        {
            help[(i*6)+j].A = matA[i][j];
            help[(i*6)+j].B = matB[i][j];
            help[(i*6)+j].row = i;
            help[(i*6)+j].col = j;
            iret[(i*6)+j] = pthread_create( &(tid[(i*6)+j]), NULL, solution, &help[(i*6)+j]); //membuat thread pertama
            if(iret[(i*6)+j]) //jika eror
            {
                fprintf(stderr,"Error - pthread_create() return code: %d\n",iret[(i*6)+j]);
                exit(EXIT_FAILURE);
            }     
        }
    }

    for(int i = 0; i<24 ;i++)
    {
        pthread_join( tid[i], NULL);  
    }
    printf("\nMatriks Hasil:\n");
    for(int i = 0; i<4; i++)
    {
            for(int j = 0; j<6; j++)
            {
                    printf("%llu ", result[i][j]);
            }
            printf("\n");
    }

    free(help);
    shmdt(matA);
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}

void *solution( void *ptr )
{
    helper *help = (helper *) ptr;
    int a = help->A, b = help->B, row = help->row, col = help->col;
    //printf("%d %d %d %d\n",help->A, help->B,help->row, help->col);

    if(!a || !b) result[row][col] = 0;
    else if (a >= b)
    {
        int start = a, end = a-b;
        result[row][col] = fact(start,end);
    }
    else result[row][col] = fact(a,0);
    return NULL;
}

unsigned long long fact(int start, int end)
{
    if(start == end) return 1;
    else return start*fact(start-1,end);
}