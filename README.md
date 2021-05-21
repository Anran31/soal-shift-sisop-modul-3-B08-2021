# soal-shift-sisop-modul-3-B08-2021

## Soal 1
Keverk adalah orang yang cukup ambisius dan terkenal di angkatannya. Sebelum dia menjadi ketua departemen di HMTC, dia pernah mengerjakan suatu proyek dimana keverk tersebut meminta untuk membuat server database buku. Proyek ini diminta agar dapat digunakan oleh pemilik aplikasi dan diharapkan bantuannya dari pengguna aplikasi ini. 
Di dalam proyek itu, Keverk diminta:

**a.** Pada saat client tersambung dengan server, terdapat dua pilihan pertama, yaitu register dan login. Jika memilih register, client akan diminta input id dan passwordnya untuk dikirimkan ke server. User juga dapat melakukan login. Login berhasil jika id dan password yang dikirim dari aplikasi client sesuai dengan list akun yang ada didalam aplikasi server. Sistem ini juga dapat menerima multi-connections. Koneksi terhitung ketika aplikasi client tersambung dengan server. Jika terdapat 2 koneksi atau lebih maka harus menunggu sampai client pertama keluar untuk bisa melakukan login dan mengakses aplikasinya. Keverk menginginkan lokasi penyimpanan id dan password pada file bernama akun.txt dengan format :
> akun.txt
> 
> id:password
> 
> id2:password2

**b.** Sistem memiliki sebuah database yang bernama files.tsv. Isi dari files.tsv ini adalah path file saat berada di server, publisher, dan tahun publikasi. Setiap penambahan dan penghapusan file pada folder file yang bernama  FILES pada server akan memengaruhi isi dari files.tsv. Folder FILES otomatis dibuat saat server dijalankan. 

**c.** Tidak hanya itu, Keverk juga diminta membuat fitur agar client dapat menambah file baru ke dalam server. Direktori FILES memiliki struktur direktori di bawah ini : 
Direktori FILES 
File1.ekstensi
File2.ekstensi

Pertama client mengirimkan input ke server dengan struktur sebagai berikut :
Contoh Command Client :		
> add
 
Output Client Console:		
> Publisher:
> 
> Tahun Publikasi:
> 
> Filepath:

Kemudian, dari aplikasi client akan dimasukan data buku tersebut (perlu diingat bahwa Filepath ini merupakan path file yang akan dikirim ke server). Lalu client nanti akan melakukan pengiriman file ke aplikasi server dengan menggunakan socket. Ketika file diterima di server, maka row dari files.tsv akan bertambah sesuai dengan data terbaru yang ditambahkan.

**d.** Dan client dapat mendownload file yang telah ada dalam folder FILES di server, sehingga sistem harus dapat mengirim file ke client. Server harus melihat dari files.tsv untuk melakukan pengecekan apakah file tersebut valid. Jika tidak valid, maka mengirimkan pesan error balik ke client. Jika berhasil, file akan dikirim dan akan diterima ke client di folder client tersebut. 
Contoh Command client
> download TEMPfile.pdf

**e.** Setelah itu, client juga dapat menghapus file yang tersimpan di server. Akan tetapi, Keverk takut file yang dibuang adalah file yang penting, maka file hanya akan diganti namanya menjadi ‘old-NamaFile.ekstensi’. Ketika file telah diubah namanya, maka row dari file tersebut di file.tsv akan terhapus.
Contoh Command Client:
> delete TEMPfile.pdf

**f.** Client dapat melihat semua isi files.tsv dengan memanggil suatu perintah yang bernama see. Output dari perintah tersebut keluar dengan format. 
Contoh Command Client :
> see

Contoh Format Output pada Client:
```
Nama:
Publisher:
Tahun publishing:
Ekstensi File : 
Filepath : 

Nama:
Publisher:
Tahun publishing:
Ekstensi File : 
Filepath : 
```

**g.** Aplikasi client juga dapat melakukan pencarian dengan memberikan suatu string. Hasilnya adalah semua nama file yang mengandung string tersebut. Format output seperti format output f.
Contoh Client Command:
find TEMP

**h.** Dikarenakan Keverk waspada dengan pertambahan dan penghapusan file di server, maka Keverk membuat suatu log untuk server yang bernama running.log. Contoh isi dari log ini adalah
running.log
> Tambah : File1.ektensi (id:pass)
> 
> Hapus : File2.ektensi (id:pass)




## Soal 2
Crypto (kamu) adalah teman Loba. Suatu pagi, Crypto melihat Loba yang sedang kewalahan mengerjakan tugas dari bosnya. Karena Crypto adalah orang yang sangat menyukai tantangan, dia ingin membantu Loba mengerjakan tugasnya. Detil dari tugas tersebut adalah:

### 2.1 
Membuat program perkalian matrix (4x3 dengan 3x6) dan menampilkan hasilnya. Matriks nantinya akan berisi angka 1-20 (tidak perlu dibuat filter angka).

#### Jawab
Pertama adalah membuat function untuk mengali yaitu `multiply` antar matriks yang akan di inputkan.
```c
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
```
Agar hasil dari soal bagian ini dapat digunakan pada soal selanjutnya, maka akan menggunakan shared memory. Karena yang disimpan dalam shared memory berbentuk 2 dimensi, maka akan menggunakan bentuk seperti ini `int (*result)[6];` kemudian setelah itu menggunakan argumen `shmid` dan `shmget` dimasukkan apa apa yang dibutuhkan ke dalam shared memory. seperti key, ukuran dari resultnya dan lain lain. 

Setelahnya seperti biasa, input value yang dibutuhkan, matriks `4x3` dan `3x6` lalu di masukkan pada function `multiply` dan di print hasilnya sesuai dengan yang diinginkan. setelah itu, baru `result` dimasukkan dalam `shmdt` untuk disalurkan/ melepaskan pada program lain yang membutuhkan.
```c
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
```

### 2.2
Membuat program dengan menggunakan matriks output dari program sebelumnya (program soal2a.c) (Catatan!: gunakan shared memory). Kemudian matriks tersebut akan dilakukan perhitungan dengan matrix baru (input user) sebagai berikut contoh perhitungan untuk matriks yang ada. Perhitungannya adalah setiap cel yang berasal dari matriks A menjadi angka untuk faktorial, lalu cel dari matriks B menjadi batas maksimal faktorialnya matri(dari paling besar ke paling kecil) (Catatan!: gunakan thread untuk perhitungan di setiap cel). 
Ketentuan
		
> If a >= b  -> a!/(a-b)!
> 
> If b > a -> a!
> 
> If 0 -> 0

Contoh :
| A | B | Angka-Angka Faktorial |
| --- | --- | :---: |
| 4 | 4 | 4 3 2 1 |
| 4 | 5 | 4 3 2 1 |
| 4 | 3 | 4 3 2 |
| 4 | 0 | 0 |
| 0 | 4 | 0 |
| 4 | 6 | 4 3 2 1 |

Contoh :
| Matriks A | Matriks B | Matriks Hasil |
| :---: | :---: | :---: |
| 0 \| 4 | 0 \| 4 | 0 \| 4* 3* 2* 1 |
| 4 \| 5 | 6 \| 2 | 4* 3* 2* 1 \| 5* 4 |
| 5 \| 6 | 6 \| 0 | 5* 4* 3* 2* 1 \| 0 |

#### Jawab
Pertama tama adalah untuk input Matriks B sebagai pembatas faktorial, menggunakan input sendiri. Disertai dengan deklarasi shared memory.
```c
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

```

Kemudian, karena disuruh menggunakan thread untuk perhitungan tiap cell, maka dibuat thread dengan ukuran 24, dari `6x4`. Karena di dalam thread menjalankan fungsi `solution` dan menyesuaikan bentuk argumen yang berupa `void pointer` sedangkan argumen dalam solution hanya ada 1, maka cara yang paling mudah adalah menggunakan bantuan dari `struct`.
```c
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
```
```c
typedef struct helper_t
{
    int A;
    int B;
    int row;
    int col;
} helper;
```
Isi dari struct itu sendiri adalah apa2 yang dibutuhkan untuk menemukan hasil perhitungan pada tiap cell, karean itulah struct `helper` dibuat sebesar jumlah cell yang ada. 
Cara memasukkan nilainya adalah menggunakan `for` untuk setiap index, dari `0 s/d 23`. `int A` berisi matriks B, `int B` berisi matriks B, `int row` berisi i, dan `int col` berisi j. 
Setelah itu dibuatlah thread masing2 cell. Setiap thread akan memanggil fungsi `solution` dengan argumen berupa alamat dari helper tersebut. Agarr semua elemen yang dibutuhkan ada.

```c
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
```
Pertama, pada fungsi `solution` dikarenakan argumennya berupa `void pointer` maka harus di custom dahulu agara sesuai dengan `helper` yaitu pada `helper *help = (helper *) ptr;` dan kemudian variablenya diganti atau disesuaikan dengan customnya yang baru. 
Sedangkan `if` setelahnya adalah kondisi yang diinginkan oleh soal. Serta jika memenuhi makan akan masuk ke fungsi `fact`.
```c
unsigned long long fact(int start, int end)
{
    if(start == end) return 1;
    else return start*fact(start-1,end);
}
```
Untuk resultnya sendiri sudah disetting ukuran awalnya pada awal program.
```c
int matB[4][6];
unsigned long long result[4][6];
void *solution( void *ptr );
unsigned long long fact( int start, int end);
```
Karena `for` pada pengisian `helper` akan berjalan sebanyak 24 kali, dan join thread juga akan berjalan sebanyak 24 kali. Maka mereka akan dijalankan secara berurutan. 
```c
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
```
Setelah selesai, selanjutnya adalah untuk memprint hasil matriks yang sudah dihitung faktorialnya tadi.
```c
printf("\nMatriks Hasil:\n");
    for(int i = 0; i<4; i++)
    {
            for(int j = 0; j<6; j++)
            {
                    printf("%llu ", result[i][j]);
            }
            printf("\n");
    }

```
Yang terakhir, karena menggunakan malloc, maka help harus di `free` kan agar memorinya bisa pulih kembali. lalu melepaskan atau `detach` matriks A dan terakhir menghancurkan shared memory yang ada.
```c    
    free(help);
    shmdt(matA);
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}
```

### 2.3
Karena takut lag dalam pengerjaannya membantu Loba, Crypto juga membuat program (soal2c.c) untuk mengecek 5 proses teratas apa saja yang memakan resource komputernya dengan command “ps aux | sort -nrk 3,3 | head -5” (Catatan!: Harus menggunakan IPC Pipes)
Note:
- Semua matriks berasal dari input ke program.
- Dilarang menggunakan system()

#### Jawab
Pertama yang dilakukan adalah membuat `pipe0` yang digunakan untuk standard input ke dalam `ls`
int main(int argc, char *argv[]) {
```c

    int pid;
    int pipe0[2];
    int pipe1[2];

    // create pipe0
    if (pipe(pipe0) == -1) {
        perror("bad pipe0");
        exit(1);
    }

    // fork (ps aux)
    if ((pid = fork()) == -1) {
        perror("bad fork0");
        exit(1);
    } 
    else if (pid == 0) {
        // stdin --> ls --> pipe0
        // input from stdin (already done), output to pipe0
        dup2(pipe0[1], 1);
        // close fds
        close(pipe0[0]);
        close(pipe0[1]);

        char *arg[]={"ps", "aux",NULL};
        execv("/bin/ps", arg);
        // exec didn't work, exit
        perror("bad exec ps");
        _exit(1);
    }
```
Setelah itu, membuat `fork` untuk command `ps aux`, jika input sudah dilakukan, maka `pipe0` akan di close, dan outputnya akan di`grep`.
Setelah membuat `pipe0`, `pipe1` sebagai parent process dibuat untuk mengambil input dari hasil output `pipe0` dan melanjutkannya ke tail dan ke standard output.
```c
// create pipe1
    if (pipe(pipe1) == -1) {
        perror("bad pipe1");
        exit(1);
    }

    // fork (sort -nrk 3,3)
    if ((pid = fork()) == -1) {
        perror("bad fork1");
        exit(1);
    } 
    else if (pid == 0) {
        // pipe0--> grep --> pipe1
        // input from pipe0
        dup2(pipe0[0], 0);
        // output to pipe1
        dup2(pipe1[1], 1);
        // close fds
        close(pipe0[0]);
        close(pipe0[1]);
        close(pipe1[0]);
        close(pipe1[1]);

        char *arg[]={"sort", "-nrk","3,3",NULL};
        execv("/bin/sort", arg);
        // exec didn't work, exit
        perror("bad exec sort");
        _exit(1);
    }
    // parent

    // close unused fds
    close(pipe0[0]);
    close(pipe0[1]);

    // pipe1--> tail --> stdout
    // input from pipe1
    dup2(pipe1[0], 0);
    // output to stdout (already done). Close fds
    close(pipe1[0]);
    close(pipe1[1]);

    char *arg[]={"head", "-5",NULL};
    execv("/bin/head", arg);
    // exec didn't work, exit
    perror("bad exec head");
    _exit(1);

    return 0;
}
```
di dalam parent, dibuat command untuk `-nrk, 3,3` dan argumen untuk mengecek resource 5 proses teratas.




## Soal 3
Seorang mahasiswa bernama Alex sedang mengalami masa gabut. Di saat masa gabutnya, ia memikirkan untuk merapikan sejumlah file yang ada di laptopnya. Karena jumlah filenya terlalu banyak, Alex meminta saran ke Ayub. Ayub menyarankan untuk membuat sebuah program C agar file-file dapat dikategorikan. Program ini akan memindahkan file sesuai ekstensinya ke dalam folder sesuai ekstensinya yang folder hasilnya terdapat di working directory ketika program kategori tersebut dijalankan.

Contoh apabila program dijalankan:
```
# Program soal3 terletak di /home/izone/soal3
$ ./soal3 -f path/to/file1.jpg path/to/file2.c path/to/file3.zip
#Hasilnya adalah sebagai berikut
/home/izone
|-jpg
|--file1.jpg
|-c
|--file2.c
|-zip
|--file3.zip
```

### 2.1 
Program menerima opsi -f seperti contoh di atas, jadi pengguna bisa menambahkan argumen file yang bisa dikategorikan sebanyak yang diinginkan oleh pengguna. 
Output yang dikeluarkan adalah seperti ini :
> File 1 : Berhasil Dikategorikan (jika berhasil)
> 
> File 2 : Sad, gagal :( (jika gagal)
> 
> File 3 : Berhasil Dikategorikan

#### Jawab
Pertama adalah 

**b.** Program juga dapat menerima opsi -d untuk melakukan pengkategorian pada suatu directory. Namun pada opsi -d ini, user hanya bisa memasukkan input 1 directory saja, tidak seperti file yang bebas menginput file sebanyak mungkin. Contohnya adalah seperti ini:
> $ ./soal3 -d /path/to/directory/

Perintah di atas akan mengkategorikan file di /path/to/directory, lalu hasilnya akan disimpan di working directory dimana program C tersebut berjalan (hasil kategori filenya bukan di /path/to/directory).
Output yang dikeluarkan adalah seperti ini :
> Jika berhasil, print “Direktori sukses disimpan!”
> 
> Jika gagal, print “Yah, gagal disimpan :(“

**c.** Selain menerima opsi-opsi di atas, program ini menerima opsi * , contohnya ada di bawah ini:
> $ ./soal3 \*

Opsi ini akan mengkategorikan seluruh file yang ada di working directory ketika menjalankan program C tersebut.

**d.** Semua file harus berada di dalam folder, jika terdapat file yang tidak memiliki ekstensi, file disimpan dalam folder “Unknown”. Jika file hidden, masuk folder “Hidden”.

**e.** Setiap 1 file yang dikategorikan dioperasikan oleh 1 thread agar bisa berjalan secara paralel sehingga proses kategori bisa berjalan lebih cepat.

Namun karena Ayub adalah orang yang hanya bisa memberi ide saja, tidak mau bantuin buat bikin programnya, Ayub meminta bantuanmu untuk membuatkan programnya. Bantulah agar program dapat berjalan!
Catatan: 
- Kategori folder tidak dibuat secara manual, harus melalui program C
- Program ini tidak case sensitive. Contoh: JPG dan jpg adalah sama
- Jika ekstensi lebih dari satu (contoh “.tar.gz”) maka akan masuk ke folder dengan titik terdepan (contoh “tar.gz”)
- Dilarang juga menggunakan fork-exec dan system()
- Bagian b dan c berlaku rekursif
