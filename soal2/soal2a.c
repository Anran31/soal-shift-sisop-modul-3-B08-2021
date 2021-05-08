#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

void multiply(int matA[][3],int matB[][6], int result[][6])
{
   for (int i = 0; i < 4; ++i) {
      for (int j = 0; j < 6; ++j) {
         for (int k = 0; k < 3; ++k) {
            result[i][j] += matA[i][k] * matB[k][j];
         }
      }
   }
}

int main()
{
    int matA[4][3], matB[3][6];

    key_t key = 1234;
    int (*result)[6];

    int shmid = shmget(key, sizeof(int[4][6]), IPC_CREAT | 0666);
    result = shmat(shmid, NULL, 0);

    printf("Masukkan Matriks 4x3:\n");
    for(int i = 0; i<4; i++)
    {
            for(int j = 0; j<3; j++)
            {
                    scanf("%d", &matA[i][j]);
            }
    }

    printf("\nMasukkan Matriks 3x6:\n");
    for(int i = 0; i<3; i++)
    {
            for(int j = 0; j<6; j++)
            {
                    scanf("%d", &matB[i][j]);
            }
    }

    multiply(matA,matB,result);

    printf("\nHasil Perkalian Matriks:\n");
    for(int i = 0; i<4; i++)
    {
            for(int j = 0; j<6; j++)
            {
                    printf("%d ", result[i][j]);
            }
            printf("\n");
    }

    shmdt(result);
    //shmctl(shmid, IPC_RMID, NULL);
    return 0;
}