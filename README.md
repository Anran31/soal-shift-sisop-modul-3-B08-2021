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

#### Jawab
Pertama, supaya server bisa tersambung dengan banyak client, tetapi client yang lain harus menunggu client pertama untuk selesai maka dapat menggunakan thread untuk per client, kemudian menggunakan `pthread_join()` untuk menunggu thread yang paling pertama selesai.

##### Server
```c
    while ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)))
    {
            int *new_sock;
            pthread_t sniffer_thread;
            new_sock = malloc(sizeof(int));
            *new_sock = new_socket;
            
            if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
            {
                perror("could not create thread");
                return 1;
            }
            
            //Now join the thread , so that we dont terminate before the thread
            pthread_join( sniffer_thread , NULL);
    }
```
##### Client
```c
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }
```

Kedua, untuk melakukan registrasi, maka server akan mengirimkan permintaan kepada client untuk menginputkan username dan password yang diinginkan kemudian server akan menyimpannya di dalam `akun.txt` dengan format `username:password`.

##### Server
```c
int main(int argc, char const *argv[]) {
    
    .....

    else if(!strcmp(cmd,"register"))
    {
        if(tryRegister(sock)>0) 
        {
            sprintf(regMsg,"%s\n\n%s",regStat,start);
            send(sock , regMsg , strlen(regMsg),0);
        }
        else
        {
            read_size = 0;
            break;
        }
    }

    .....
}

    //Fungsi tryRegister
    int tryRegister(int sock)
    {
        char msg[20] = { 0 };
        char user[50] = {0};
        char pwd[50] = {0};
        char *tmp[] = {"Enter Username : ", "Enter Password : "};
        int read_val;
        char namePass[PATH_MAX];
        for(int i = 0; i<2;i++)
        {
            strcpy(msg,tmp[i]);
            send(sock , msg , strlen(msg),0);
            memset(msg,0,sizeof(msg));
            if(!i) 
            {
                read_val = read(sock , user , 50);
            }
            else read_val = read(sock , pwd , 50);
            if(!read_val) return -1;
        }

        sprintf(namePass,"%s:%s\n",user,pwd);

        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd() error");
            return -1;
        }

        char akunPath[PATH_MAX];
        sprintf(akunPath,"%s/%s",cwd,"akun.txt");

        FILE * fPtr;

        fPtr = fopen(akunPath, "a");

        if(fPtr == NULL)
        {
            printf("Unable to create file.\n");
            return -1;
        }
        fputs(namePass, fPtr);
        fclose(fPtr);

        return 1;
    }
```

##### Client
```c
int main(int argc, char const *argv[]) {

    .....

    if(!strcmp(command,"register") && !Auth)
    {
        getCreds(sock, buffer,command);
    }

    .....
}

    //Fungsi getCreds
    void getCreds(int sock, char *buffer, char *cmd)
    {
        memset(buffer,0,2048);
        memset(cmd,0,2048);
        for(int i = 0; i<2;i++)
        {
            read( sock , buffer, 2048);
            printf("%s\n",buffer );
            memset(buffer,0,2048);
            scanf("%s",cmd);
            send(sock , cmd , strlen(cmd) , 0 );
            memset(cmd,0,2048);
        }
    }
```
Kemudian, untuk melakukan login, maka server akan mengirimkan permintaan kepada client untuk menginputkan username dan password kemudian server akan membandingkannya dengan seluruh `username:password` di dalam `akun.txt`. Jika ada yang cocok, maka user akan terautentikasi dan dapat menjalankan command lainnya. Tetapi jika tidak, server akan mengirim pesan error ke client.

##### Server
```c
int main(int argc, char const *argv[]) {

    .....

    if(!strcmp(cmd,"login"))
    {
        if(tryLogin(&auth,sock,authUser,authPass)>0) 
        {
            if(auth) 
            {
                int Stat = 1;
                send(sock,&Stat,sizeof(Stat),0);
                strcpy(loginStat, "Login Success");
                sprintf(loginMsg,"%s\n\n%s",loginStat,authenticated);
            }
            else 
            {
                int Stat = 0;
                send(sock,&Stat,sizeof(Stat),0);
                strcpy(loginStat, "Wrong Username or Password");
                sprintf(loginMsg,"%s\n\n%s",loginStat,start);
            }
            send(sock , loginMsg , strlen(loginMsg),0);
        }
        else 
        {
            read_size = 0;
            break;
        }
    .....
}

int tryLogin(int *auth, int sock, char *authUser, char *authPass)
{
    char msg[20] = { 0 };
    char user[50] = {0};
    char pwd[50] = {0};
    char *tmp[] = {"Enter Username : ", "Enter Password : "};
    char namePass[PATH_MAX];
    int read_val;
    for(int i = 0; i<2;i++)
    {
        strcpy(msg,tmp[i]);
        send(sock , msg , strlen(msg),0);
        memset(msg,0,sizeof(msg));
        if(!i) 
        {
            read_val = read(sock , user , 50);
        }
        else read_val = read(sock , pwd , 50);
        if(!read_val) return -1;
    }

    sprintf(namePass,"%s:%s\n",user,pwd);

    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd() error");
        return -1;
    }

    char akunPath[PATH_MAX];
    sprintf(akunPath,"%s/%s",cwd,"akun.txt");

    FILE * fPtr;

    fPtr = fopen(akunPath, "r");

    if(fPtr == NULL)
    {
        printf("Unable to create file.\n");
        return -1;
    }

    char *creds;
    size_t len = 0;
    while ((getline(&creds, &len, fPtr)) != -1) {
        if(!strcmp(creds,namePass))
        {
            //printf("auth %s %s\n", creds,namePass);
            *auth = 1;
            strcpy(authUser,user);
            strcpy(authPass,pwd);
        }
    }

    memset(akunPath,0,sizeof(akunPath));
    fclose(fPtr);

    return 1;
}
```

##### Client
```c
    //Digunakan untuk mengecek apakah client terautentikasi atau tidak
    int auth = 0;

    ....

    else if(!strcmp(command,"login") && !Auth)
    {
        //Fungsi getCreds sama seperti yang di atas.
        getCreds(sock, buffer,command);
        read(sock,&Auth,sizeof(Auth));
    }
```


**b.** Sistem memiliki sebuah database yang bernama files.tsv. Isi dari files.tsv ini adalah path file saat berada di server, publisher, dan tahun publikasi. Setiap penambahan dan penghapusan file pada folder file yang bernama  FILES pada server akan memengaruhi isi dari files.tsv. Folder FILES otomatis dibuat saat server dijalankan.

#### Jawab

Pada saat Server dijalankan dan sebelum menerima koneksi dari client, server akan membuat sebuah direktori bernama `FILES` terlebih dahulu

##### Server
```c
int main(int argc, char const *argv[]) {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
   
      
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
      
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
      
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    makeFilesDir();

    .....

    return 0;
}

void makeFilesDir()
{
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd() error");
    }
    char DirPath[PATH_MAX];
    sprintf(DirPath,"%s/%s",cwd,"FILES");
    mkdir(DirPath,0777);

    return;
}
```

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

#### Jawab
Ketika client memasukkan command `add`, maka server akan meminta client memasukkan data yang dibutuhkan. Kemudian akan mengecek path yang diberikan oleh client, jika ada, maka client akan mengirimkan file tersebut dan server akan menerimanya. Jika tidak ada, maka akan menampilkan pesan error. Setelah file diterima oleh server, maka files.tsv akan diupdate.

##### Server
```c
    if(!strcmp(cmd,"add"))
    {
        if(tryAdd(sock,authUser,authPass)>0) 
        {
            send(sock , authenticated , strlen(authenticated),0);
        }
        else 
        {
            char *notExist = "\nFile Didn't Exist or Path Not Found\n";
            char fileNotFound[PATH_MAX];
            sprintf(fileNotFound,"%s\n%s",notExist,authenticated);
            send(sock , fileNotFound , strlen(fileNotFound),0);
        }
    }

    //Fungsi tryAdd
    int tryAdd(int sock, char *authUser, char *authPass)
    {
        char *list[]={"Publisher: ","Tahun Publikasi: ","Filepath: "};
        char *status[] = {"Downloading","File Not Found"};
        char msg[20] = {0};
        char Publisher[2048] = {0}, Tahun[2048] = {0}, Filepath[2048]={0};
        int read_val;
        for(int i = 0; i<3;i++)
        {
            strcpy(msg,list[i]);
            send(sock , msg , strlen(msg),0);
            memset(msg,0,sizeof(msg));
            if(i==0)  
            {
                read_val = read(sock , Publisher , sizeof(Publisher));
            }
            else if (i == 1) 
            {
                read_val = read(sock , Tahun , sizeof(Tahun));
            }
            else
            {
                read_val = read(sock , Filepath , sizeof(Filepath));
            } 

            if(!read_val) return -1;
        }

        if(checkExistAndRegFile(Filepath))
        {
            strcpy(msg,status[0]);
            send(sock , msg , strlen(msg),0);
            memset(msg,0,sizeof(msg));
            const char *p="/";
            char *a,*b;
            char fullPath[PATH_MAX];
            strcpy(fullPath,Filepath);
            char fileName[100];

            for( a=strtok_r(fullPath,p,&b) ; a!=NULL ; a=strtok_r(NULL,p,&b) ) {
                memset(fileName,0,sizeof(fileName));
                strcpy(fileName,a);
            }

            char exten[100];
            getFileExt(Filepath,exten);

            int ret = write_file(sock,fileName);

            if(ret)
            {
                addDB(fileName,Publisher,Tahun,Filepath,exten);
                char status[10];
                strcpy(status,"Tambah");
                makeLog(fileName,authUser,authPass,status);
                return 1;
            }

        }
        else
        {      
            return -1;
        }
    }

    //Fungsi untuk mengecek apakah filenya ada
    bool checkExistAndRegFile(char *basePath)
    {
        struct stat buffer;
        int exist = stat(basePath,&buffer);
        if(exist == 0)
        {
            if( S_ISREG(buffer.st_mode) ) return true;
            else return false;
        }
        else  
            return false;
    }

    //Fungsi untuk mendownload file dari client
    int write_file(int sockfd, char *fileName){
        int n;
        FILE *fp;
        char buffer[PATH_MAX];
            char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd() error");
            return -1;
        }

        char destination[PATH_MAX];
        sprintf(destination,"%s/%s/%s",cwd,"FILES",fileName);
        FILE *fr = fopen(destination, "wb");
            if(fr == NULL)
                printf("File %s Cannot be opened file on server.\n", destination);
            else
            {
                bzero(buffer, PATH_MAX); 
                int fr_block_sz = 0;
                while((fr_block_sz = recv(sockfd, buffer, PATH_MAX, 0)) > 0) 
                {
                    int write_sz = fwrite(buffer, sizeof(char), fr_block_sz, fr);
                    if(write_sz < fr_block_sz)
                    {
                        error("File write failed on server.\n");
                    }
                    bzero(buffer, PATH_MAX);
                    if (fr_block_sz == 0 || fr_block_sz != PATH_MAX) 
                    {
                        break;
                    }
                }
                if(fr_block_sz < 0)
                {
                    if (errno == EAGAIN)
                    {
                        printf("recv() timed out.\n");
                    }
                    else
                    {
                        fprintf(stderr, "recv() failed due to errno = %d\n", errno);
                        exit(1);
                    }
                }
                fclose(fr);
                return 1;
            }
        return 1;
    }

    //Fungsi addDB untuk mengupdate files.tsv
    void addDB(char *fileName,char *Publisher,char *Tahun,char *Filepath,char *exten)
    {
        char cwd[PATH_MAX] = {0};
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd() error");
            return -1;
        }

        char filesPath[PATH_MAX] = {0};
        sprintf(filesPath,"%s/%s",cwd,"files.tsv");

        //printf("%s\n",filesPath);
        FILE * fPtr;

        fPtr = fopen(filesPath, "a");

        if(fPtr == NULL)
        {
            printf("Unable to create file.\n");
            return -1;
        }
        char bookPath[1024] = {0}, log[PATH_MAX] = {0};
        sprintf(bookPath,"%s/FILES/%s",cwd,fileName);
        sprintf(log,"%s\t%s\t%s\n",Publisher,Tahun,bookPath);
        fputs(log,fPtr);
        fclose(fPtr);
    }
```

##### Client
```c
    else if(!strcmp(command,"add")&& Auth)
    {
        if(!addFile(sock, buffer,command)) menu = 0;
    }

    //Fungsi addFile untuk mengirim file ke server
    int addFile(int sock, char *buffer, char *cmd)
    {
        for(int i = 0; i<4;i++)
        {
            memset(buffer,0,2048);
            read( sock , buffer, 2048);
            if(i != 3)printf("%s\n",buffer );
            if(i !=3)
            {
                memset(cmd,0,2048);
                scanf(" %[^\n]s",cmd);
                send(sock , cmd , strlen(cmd) , 0 );
            }
            else if(i==3)
            {
                if(!strcmp(buffer,"Downloading"))
                {
                    FILE *fp;
                    fp = fopen(cmd, "r");
                    if (fp == NULL) {
                        perror("[-]Error in reading file.");
                        continue;
                    }
                    send_file(fp, sock);
                    fclose(fp);
                    memset(buffer,0,2048);
                    return 1;
                }
                else{
                    memset(buffer,0,2048);
                    memset(cmd,0,2048);
                    return 0;
                    break;
                }
            }
        } 
        memset(buffer,0,2048);
        memset(cmd,0,2048);
    }
```

**d.** Dan client dapat mendownload file yang telah ada dalam folder FILES di server, sehingga sistem harus dapat mengirim file ke client. Server harus melihat dari files.tsv untuk melakukan pengecekan apakah file tersebut valid. Jika tidak valid, maka mengirimkan pesan error balik ke client. Jika berhasil, file akan dikirim dan akan diterima ke client di folder client tersebut. 
Contoh Command client
> download TEMPfile.pdf

#### Jawab
Jika client mengirim command `download` dengan nama file yang ingin di download, maka server akan mengecek database yang ada di `files.tsv` apakah terdapat buku dengan nama file yang sesuai. Jika ada, maka server akan mengirim file tersebut ke client, tetapi jika tidak ada maka akan mengirimkan pesan error.

##### Server
```c
    else if(!strcmp(cmd,"download"))
    {
            char bookName[2048] = {0};
            char *dlStat[] = {"File Not Found\n", "Download Success\n"};
            //read(sock,bookName,2048);
            //printf("%s\n",file);
            if(tryDownload(sock,file))
            {
                char dlSuc[2048];
                sprintf(dlSuc,"%s\n%s",dlStat[1],authenticated);
                send(sock, dlSuc, strlen(dlSuc),0);
            }
            else{
                char dlFail[2048];
                sprintf(dlFail,"%s\n%s",dlStat[0],authenticated);
                send(sock, dlFail, strlen(dlFail),0);
            }
    }

    //Fungsi tryDownload
    int tryDownload(int sock, char *bookName)
    {
        char filePath[2048] = {0};
        char clientRes[1024] = {0};
        if(checkFile(bookName,filePath))
        {
            char *suc = "downloading";
            send(sock,suc,strlen(suc),0);
            read(sock,clientRes,sizeof(clientRes));
            sendFile(sock,filePath);
            return 1;
        }
        else{
            char *fail = "failed";
            send(sock,fail,strlen(fail),0);
            read(sock,clientRes,sizeof(clientRes));
            return 0;
        }
    }

    //Funsi checkFile
    int checkFile(char *bookName, char *filePath)
    {
        FILE *f;
        char tsvPath[2048] = {0}, cwd[1024] = {0};
        getcwd(cwd,sizeof(cwd));
        sprintf(tsvPath,"%s/files.tsv",cwd);
        f = fopen(tsvPath, "r"); 
        char line[1024];
        while (fgets(line, sizeof(line), f)) {
            char name[1024] = {0};
            char *lastName = strrchr(line,'/');
            strcpy(name,lastName+1);
            name[strcspn(name,"\n")] = '\0';
            if (!strcmp(bookName,name)) {
                //printf("%s",line);
                sprintf(filePath,"%s/FILES/%s",cwd,name);
                fclose(f);
                return 1;
            }
        }
        fclose(f);
        return 0;
    }

    //Fungsi sendFile
    void sendFile(int sock, char *filePath)
    {
        char sdbuf[PATH_MAX]; // Send buffer
        FILE *fs = fopen(filePath, "rb");
        if(fs == NULL)
        {
            fprintf(stderr, "ERROR: File %s not found on server. (errno = %d)\n", filePath, errno);
            exit(1);
        }

        bzero(sdbuf, PATH_MAX); 
        int fs_block_sz; 
        while((fs_block_sz = fread(sdbuf, sizeof(char), PATH_MAX, fs))>0)
        {
            if(send(sock, sdbuf, fs_block_sz, 0) < 0)
            {
                fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n", filePath, errno);
                exit(1);
            }
            bzero(sdbuf, PATH_MAX);
        }
        fclose(fs);
        char stat[1024] = {0};
        read(sock,stat,sizeof(stat));
    }
```

##### Client
```c
    else if(!strcmp(command,"download")&& Auth)
    {
        char sta[20] = {0},respond[1024] = {0};
        memset(sta,0,sizeof(sta));
        read( sock , sta, sizeof(sta));
        strcpy(respond,"ok");
        send(sock,respond,sizeof(respond),0);
        if(!strcmp(sta,"downloading"))receiveFile(sock,file);
    }

    //Fungsi receiveFile
    int receiveFile(int sock, char *fileName)
    {
        char cwd[1024] = {0}, destName[2048] = {0}, revbuf[PATH_MAX] = {0};
        getcwd(cwd,sizeof(cwd));
        sprintf(destName,"%s/%s",cwd,fileName);
        FILE *fr = fopen(destName, "wb");
        if(fr == NULL)
            printf("File %s Cannot be opened.\n", destName);
        else
        {
            bzero(revbuf, PATH_MAX); 
            int fr_block_sz = 0;
            while((fr_block_sz = recv(sock, revbuf, PATH_MAX, 0)) > 0)
            {
                int write_sz = fwrite(revbuf, sizeof(char), fr_block_sz, fr);
                if(write_sz < fr_block_sz)
                {
                    error("File write failed.\n");
                }
                bzero(revbuf, PATH_MAX);
                if (fr_block_sz == 0 || fr_block_sz != PATH_MAX) 
                {
                    break;
                }
            }
            if(fr_block_sz < 0)
            {
                if (errno == EAGAIN)
                {
                    printf("recv() timed out.\n");
                }
                else
                {
                    fprintf(stderr, "recv() failed due to errno = %d\n", errno);
                }
            }
            fclose(fr);
            char stat[1024] = {0};
            strcpy(stat,"finish");
            send(sock,stat,sizeof(stat),0);
        }
    }
```
**e.** Setelah itu, client juga dapat menghapus file yang tersimpan di server. Akan tetapi, Keverk takut file yang dibuang adalah file yang penting, maka file hanya akan diganti namanya menjadi ‘old-NamaFile.ekstensi’. Ketika file telah diubah namanya, maka row dari file tersebut di file.tsv akan terhapus.
Contoh Command Client:
> delete TEMPfile.pdf

#### Jawab
Saat client mengirimkan command `delete` beserta nama file, maka server akan mengecek apakah file tersebut ada di `files.tsv`. Jika ada, maka file tersebut di folder `FILES` akan direname menjadi ``old-namaFile`` kemudian di `files.tsv`, data file tersebut akan dihapus dengan cara membuat file temp terlebih dahulu kemudian memindahkan per line kecuali yang berisi `namaFile` yang dihapus.

##### Server
```c
    else if(!strcmp(cmd,"delete"))
    {
        char *delStat[] = {"File Not Found\n", "Delete Success\n"};
        if(tryDelete(file,authUser,authPass))
        {
            char delSuc[2048];
            sprintf(delSuc,"%s\n%s",delStat[1],authenticated);
            send(sock, delSuc, strlen(delSuc),0);
        }
        else{
            char delFail[2048];
            sprintf(delFail,"%s\n%s",delStat[0],authenticated);
            send(sock, delFail, strlen(delFail),0);
        }                
    }

    //Fungsi tryDelete
    int tryDelete(char *bookName, char *user, char *pass)
    {
        char filePath[2048] = {0};
        if(checkFile(bookName,filePath))
        {
            deleteFile(filePath,bookName);
            char status[10];
            strcpy(status,"Hapus");
            makeLog(bookName,user,pass,status);
            return 1;
        }
        else{

            return 0;
        }
    }

    //Fungsi checkFile
    int checkFile(char *bookName, char *filePath)
    {
        FILE *f;
        char tsvPath[2048] = {0}, cwd[1024] = {0};
        getcwd(cwd,sizeof(cwd));
        sprintf(tsvPath,"%s/files.tsv",cwd);
        f = fopen(tsvPath, "r"); 
        char line[1024];
        while (fgets(line, sizeof(line), f)) {
            char name[1024] = {0};
            char *lastName = strrchr(line,'/');
            strcpy(name,lastName+1);
            name[strcspn(name,"\n")] = '\0';
            //printf("%s\n",name);
            if (!strcmp(bookName,name)) {
                //printf("%s",line);
                sprintf(filePath,"%s/FILES/%s",cwd,name);
                fclose(f);
                return 1;
            }
        }
        fclose(f);
        return 0;
    }

    //Fungsi deleteFile
    void deleteFile(char *filePath, char *bookName)
    {
        char deletedFileName[2048] = {0} ,cwd[1024] = {0};
        getcwd(cwd,sizeof(cwd));
        sprintf(deletedFileName,"%s/FILES/old-%s",cwd,bookName);
        rename(filePath,deletedFileName);

        FILE *f,*fNew;
        char tsvPath[2048] = {0}, tempTSV[2048] = {0};
        sprintf(tsvPath,"%s/files.tsv",cwd);
        sprintf(tempTSV,"%s/temp-files.tsv",cwd);
        f = fopen(tsvPath, "r"); 
        fNew = fopen(tempTSV, "w"); 
        char line[1024];
        while (fgets(line, sizeof(line), f)) {
            char name[1024] = {0};
            char *lastName = strrchr(line,'/');
            strcpy(name,lastName+1);
            name[strcspn(name,"\n")] = '\0';
            if (!strcmp(bookName,name)) {
                continue;
            }
            else fputs(line,fNew);
        }
        fclose(f);
        fclose(fNew);
        remove(tsvPath);
        rename(tempTSV,tsvPath);
    }
```
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

#### Jawab
Ketika Server menerima command `see` dari client, maka server akan mengirimkan seluruh baris dari `files.tsv` yang diformat sesuai format ke client.

#### Server
```c
    else if(!strcmp(cmd,"see"))
    {
        char *seeStat[] = {"Database is Empty\n", "All Book in Database\n"};
        //read(sock,bookName,2048);
        //printf("%s\n",file);
        if(trySee(sock))
        {
            char seeSuc[2048];
            sprintf(seeSuc,"%s\n%s",seeStat[1],authenticated);
            send(sock, seeSuc, strlen(seeSuc),0);
        }
        else{
            char seeFail[2048];
            sprintf(seeFail,"%s\n%s",seeStat[0],authenticated);
            send(sock, seeFail, strlen(seeFail),0);
        }  
    }

    int trySee(int sock)
    {
        FILE *f,*fNew;
        char tsvPath[2048] = {0}, tempTSV[2048] = {0},cwd[1024] = {0};
        getcwd(cwd,sizeof(cwd));
        sprintf(tsvPath,"%s/files.tsv",cwd);
        f = fopen(tsvPath, "r");

        if(f == NULL) return 0;
        char line[1024], All[4096] ={0};
        while (fgets(line, sizeof(line), f)) {
            char Nama[1024] = {0}, Publisher[1024] = {0}, Tahun[1024] = {0}, Ekstensi[1024] = {0}, Filepath[1024] = {0};
            
            char fullLine[1024] = {0};

            strcpy(fullLine,line);
            fullLine[strcspn(fullLine,"\n")] = '\0';

            const char *p="\t";
            char *a,*b;
            int i = 0;

            for( a=strtok_r(fullLine,p,&b) ; a!=NULL ; a=strtok_r(NULL,p,&b) ) {
                if(i == 0) strcpy(Publisher,a);
                else if(i == 1) strcpy(Tahun,a);
                else strcpy(Filepath,a);

                i++;
            }      
            
            char NamaFileExt[1024] = {0};

            char *PtrFileExt = strrchr(Filepath,'/');
            strcpy(NamaFileExt,PtrFileExt+1);
            getFileExt(NamaFileExt,Ekstensi);

            const char *q=".";
            char *c, *d;
            char nameCopy[1024];
            strcpy(nameCopy,NamaFileExt);
            c=strtok_r(nameCopy,q,&d);
            strcpy(Nama,c);
            char Format[4096] = {0};
            sprintf(Format,"Nama: %s\nPublisher: %s\nTahun publishing: %s\nEkstensi file : %s\nFilepath : %s\n",Nama,Publisher,Tahun,Ekstensi,Filepath);
            strcat(All,Format);
            strcat(All,"\n");
        }
        All[strlen(All)] = '\0';
        send(sock, All, sizeof(All), 0) ;
        bzero(All, 4096) ;
        fclose(f);
        return 1;
    }
```

##### Client
```c
    else if(!strcmp(command,"see")&& Auth)
    {
        recSee(sock);
    }

    //Fungsi recSee
    void recSee(int sock)
    {
        char bufRec[4096];
        int recvSize;
        bzero(bufRec, 4096) ;
        recvSize=read(sock, bufRec, 4096);
        printf("%s", bufRec) ;
        bzero(bufRec, 4096) ;
        return;
    }
```
**g.** Aplikasi client juga dapat melakukan pencarian dengan memberikan suatu string. Hasilnya adalah semua nama file yang mengandung string tersebut. Format output seperti format output f.
Contoh Client Command:
find TEMP

#### Jawab
Untuk command `find`, hampir sama seperti command `see` tetapi server hanya mengirim baris dari `files.tsv`  yang diformat seperti di poin sebelumnya.

##### Server
```c
    else if(!strcmp(cmd,"find"))
    {
        char *findStat[] = {"No Book Found", "All Book in Database"};
        if(tryFind(sock,file))
        {
            char findSuc[2048];
            sprintf(findSuc,"%s that contains %s\n\n%s",findStat[1],file,authenticated);
            send(sock, findSuc, strlen(findSuc),0);
        }
        else{
            char findFail[2048];
            sprintf(findFail,"%s that contains %s\n\n%s",findStat[0],file,authenticated);
            send(sock, findFail, strlen(findFail),0);
        }               
    }

    //Fungsi tryFind
    int tryFind(int sock,char *fileName)
    {
        int Count = 0;
        FILE *f,*fNew;
        char tsvPath[2048] = {0}, tempTSV[2048] = {0},cwd[1024] = {0};
        getcwd(cwd,sizeof(cwd));
        sprintf(tsvPath,"%s/files.tsv",cwd);
        f = fopen(tsvPath, "r");

        if(f == NULL) return 0;
        char line[1024], All[4096] ={0};
        while (fgets(line, sizeof(line), f)) {
            char Nama[1024] = {0}, Publisher[1024] = {0}, Tahun[1024] = {0}, Ekstensi[1024] = {0}, Filepath[1024] = {0};
            
            char fullLine[1024] = {0};

            strcpy(fullLine,line);
            fullLine[strcspn(fullLine,"\n")] = '\0';

            const char *p="\t";
            char *a,*b;
            int i = 0;

            for( a=strtok_r(fullLine,p,&b) ; a!=NULL ; a=strtok_r(NULL,p,&b) ) {
                if(i == 0) strcpy(Publisher,a);
                else if(i == 1) strcpy(Tahun,a);
                else strcpy(Filepath,a);

                i++;
            }      
            
            char NamaFileExt[1024] = {0};

            char *PtrFileExt = strrchr(Filepath,'/');
            strcpy(NamaFileExt,PtrFileExt+1);
            getFileExt(NamaFileExt,Ekstensi);

            const char *q=".";
            char *c, *d;
            char nameCopy[1024];
            strcpy(nameCopy,NamaFileExt);
            c=strtok_r(nameCopy,q,&d);
            strcpy(Nama,c);
            char *found;
            found = strstr(Nama, fileName);
            if(found == NULL) continue;
            char Format[4096] = {0};
            sprintf(Format,"Nama: %s\nPublisher: %s\nTahun publishing: %s\nEkstensi file : %s\nFilepath : %s\n",Nama,Publisher,Tahun,Ekstensi,Filepath);
            if(!Count)strcat(All,"aaaa");
            strcat(All,Format);
            strcat(All,"\n");
            Count++;
        }
        All[strlen(All)] = '\0';
        if(!Count)
        {
            int Stat = 0;
            send(sock,&Stat,sizeof(Stat),0);
            return 0;
        }
        send(sock, All, sizeof(All), 0) ;
        bzero(All, 4096) ;
        fclose(f);
        return 1;
    }
```

##### Client
```c
    else if(!strcmp(command,"find")&&Auth)
    {
        int Stat = 0;
        read(sock,&Stat,sizeof(Stat));
        if(Stat) recSee(sock);
    }

    //Fungsi recSee sama seperti di poin sebelumnya.
```
**h.** Dikarenakan Keverk waspada dengan pertambahan dan penghapusan file di server, maka Keverk membuat suatu log untuk server yang bernama running.log. Contoh isi dari log ini adalah
running.log
> Tambah : File1.ektensi (id:pass)
> 
> Hapus : File2.ektensi (id:pass)

#### Jawab
Untuk poin ini, ketika ada command `add`atau `delete` maka di `running.log` akan ditambahkan baris sesuai format menggunakan fungsi `makeLog()` yang dipanggil di akhir fungsi `tryAdd()` atau `tryDelete()`.

##### Server
```c
    void makeLog(char *fileName,char *User,char *Pass,char *Status)
    {
        char cwd[PATH_MAX] = {0};
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd() error");
            return -1;
        }

        char filesPath[PATH_MAX] = {0};
        sprintf(filesPath,"%s/%s",cwd,"running.log");
        FILE * fPtr;

        fPtr = fopen(filesPath, "a");

        if(fPtr == NULL)
        {
            printf("Unable to create file.\n");
            return -1;
        }

        char log[PATH_MAX] = {0};
        sprintf(log,"%s : %s (%s:%s)\n",Status,fileName,User,Pass);
        fputs(log, fPtr);
        fclose(fPtr);
    }
```


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

Pertama adalah hand;ing exception untuk untuk semua perintah yang akan dijalankan. pada main digunakan argumen `argc` dan `argv`. `argc` digunakan untuk mengecek jika perintah yang dimasukkan kurang dari 2, atau berarti hanya mengisi nama file tanpa argumen. Dan membuat error, maka akan di handling seperti berikut.
```c
    if(argc<2)
    {
        printf ("Need Arguments\nList of Valid Arguments:\n-f\n-d\n*\n");
        return 0;
    }
```

### 3.1 
Program menerima opsi -f seperti contoh di atas, jadi pengguna bisa menambahkan argumen file yang bisa dikategorikan sebanyak yang diinginkan oleh pengguna. 
Output yang dikeluarkan adalah seperti ini :
> File 1 : Berhasil Dikategorikan (jika berhasil)
> 
> File 2 : Sad, gagal :( (jika gagal)
> 
> File 3 : Berhasil Dikategorikan

#### Jawab
Untuk membuat opsi -f yang dapat menerima argumen files sebanyak yg ingin dikategorikan pengguna, maka akan menggunakan handling untuk jika `argc` kurang dari 3 maka akan butuh path lagi karena dia akan menerima path ke file yang akan dikategorikan sesuai jenis file/ extensionnya seperti `.jpg``.txt``.png` dan sebagainya.

```c
    else if(!strcmp(argv[1],"-f")) 
    {

        //printf("%d\n", argc);
        if(argc<3)
        {
            printf ("Need Minimal 1 Path to File\n");
            exit(1);
            return 0;
        }
```

Lalu pada thread, `argc` dikurangi 2 karena 2 pertama pada argumen adalah nama file dan argumen `-f` yang kita gunakan. Maka supaya mendapatkan path file yang kita inginkan maka `argc` dikurangi 2.
Setelah itu, dibuat looping `pthread` untuk memasukkan argumen `argv` yang sudah menyimpan path dari file yang kita inginkan serta memasukkannya dalam fungsi `moveFile`.
```c

        pthread_t tid[argc-2];

        for(int i = 2; i<argc; i++)
        {
            pthread_create( &(tid[i-2]), NULL, moveFile, (void*) argv[i]);
        }
```

Dalam fungsi `moveFile`, pertama adalah menyimpan path dari pointer argumen dalam `basePath`. Kemudian, adalah mengecek apakah file yang kita inginkan itu berupa `file register` atau file yang benar dan bukan `directory` atau semacamnya dengan menggunakan fungsi `checkExistAndRegFile`. 
Dalam fungsi ini, akan dicek apakah dalam `basePath` ada path yang terlampir, jika ada maka akan mengecek apakah path itu menggunakan file yang benar atau tidak.  return value adalah berupa boolean. 
Jika return false, maka tidak perlu untuk menjalankan fungsi di `moveFiles`. dan akan langsung me-return nilai 0.
```c
bool checkExistAndRegFile(char *basePath)
{
    struct stat buffer;
    int exist = stat(basePath,&buffer);
    if(exist == 0)
    {
        if( S_ISREG(buffer.st_mode) ) return true;
        else return false;
    }
    else  
        return false;
}
```

Dan jika bernilai true, maka akan menjalankan argumen2 dalam fungsi `moveFiles`. 
Pertama adalah memisahkan path menggunakan `strtok` berdasarkan tanda `/` pada path filenya. Kemudian di gunakan `memset` dari 0 dan `strcpy` selama belum null. Karena pasti nama file yang akan kita pindah itu ada di paling ujung path, maka kita tinggal mengambil yang paling terakhir saja.

```c
void *moveFile( void *arg )
{
    char basePath[PATH_MAX];
    strcpy(basePath,(char *) arg);

    if(checkExistAndRegFile(basePath))
    {
        //printf("%s\n",(char*)arg);
        const char *p="/";
        char *a,*b;
        char fullPath[PATH_MAX];
        strcpy(fullPath,(char *) arg);

        char fileName[100];

        for( a=strtok_r(fullPath,p,&b) ; a!=NULL ; a=strtok_r(NULL,p,&b) ) {
            memset(fileName,0,sizeof(fileName));
            strcpy(fileName,a);
        }
```
Setelah itu kita mengecek extension dari filenya dengan masuk ke fungsi `getFileExt`. Di dalamnya, digunakan `strchr` untuk mengambil char dari huruf yang dipilih ke belakang. Maka untuk soal ini yang kita butuhkan adalah huruf dari belakang `.`. Nah, jika `ext` nilai NULL maka tidak ditemukan `.` sama sekali di path dan akan mereturn `Unknown`. 
Jika ditemukan sama dengan `fileName` atau `.` ada di paling depan maka akan direturn `Hidden`.
Dan jika ditemukan akan di printf jenis extensionnya dan di copy charnya hanya dari belakang `.` saja dengan menggunakan `ext+1`.
```c
void getFileExt(char* fileName, char *exten)
{
    char *ext = strchr(fileName, '.');
    if (ext == NULL) {
        strcpy(exten,"Unknown");
    } 
    else if (ext == fileName){
        strcpy(exten,"Hidden");
    }
    else
    {
        //printf("file extension is %s\n",ext+1);
        strcpy(exten,ext+1);
    }
}
```
Selanjutnya, dalam `moveFile` karena dikatakan tidak case sensitive, maka jika `ext` bukan `Hidden` atau `Unknown` setiap hurufnya akan di `tolower` agar seragam.
Kemudian memakai `cwd` untuk mendapatkan walking directory atau path dimana program itu dijalankan. Jika tidak ada, maka akan handling error. 
lalu untuk alamat tujuan `destDir` menggunakan `sprintf` untuk menggabungkan `cwd` dengan `/` dan hasil extension yang telah di lowercase tadi dan dibuat directorynya dengan `mkdir`.
Terakhir adalah memindahkan file dengan `rename` yang memiliki fungsi sama dengan move. Dipindahkannya dari `basePath` ke `dest` menggunakan cwd tadi. Dan karena sudah sukses maka akan return 1, atau true.
```c
        char ext[PATH_MAX];
        getFileExt(fileName,ext);

        if(strcmp(ext,"Hidden") != 0 && strcmp(ext,"Unknown") != 0)
        {
            for(int i = 0; i<strlen(ext); i++)
            {
                ext[i] = tolower(ext[i]);
            }
        }

        //printf("%s\n",ext);

        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd() error");
            return (void *) 0;
        }

        //printf("%s\n",cwd);

        char destDir[PATH_MAX];
        sprintf(destDir,"%s/%s",cwd,ext);
        // checkDestDir(destDir);
        mkdir(destDir,0777);

        char dest[PATH_MAX];
        sprintf(dest,"%s/%s/%s",cwd,ext,fileName);
        //printf("%s %s\n",basePath,dest);
        rename(basePath,dest);
        //pthread_exit(&retStatus);
        return (void *) 1;
    }
    //else pthread_exit(&retStatus);
    else return (void *) 0;
}
```

Terakhir, pada `main`, akan dicek terlebih dahulu return value yang datang, jika 1 maka akan di print `Berhasil Dikategorikan` dan jika 0 maka print `Sad, gagal :(`
```c

        for (int i = 0; i < argc-2; i++)
        {
            int returnValue;
            void *ptr;
            pthread_join( tid[i], &ptr);
            returnValue = (int) ptr;
            //printf("%d %d\n",i, returnValue);
            if(returnValue) printf("File %d : Berhasil Dikategorikan\n", i+1);
            else printf("File %d : Sad, gagal :(\n", i+1);
        }
        
        return 0;
    }
```
Dan jika sudah selesai atau bukan `-f` maka akan return 0.


### 3.2
Program juga dapat menerima opsi -d untuk melakukan pengkategorian pada suatu directory. Namun pada opsi -d ini, user hanya bisa memasukkan input 1 directory saja, tidak seperti file yang bebas menginput file sebanyak mungkin. Contohnya adalah seperti ini:
> $ ./soal3 -d /path/to/directory/

Perintah di atas akan mengkategorikan file di /path/to/directory, lalu hasilnya akan disimpan di working directory dimana program C tersebut berjalan (hasil kategori filenya bukan di /path/to/directory).
Output yang dikeluarkan adalah seperti ini :
> Jika berhasil, print “Direktori sukses disimpan!”
> 
> Jika gagal, print “Yah, gagal disimpan :(“

#### Jawab
Pertama adalah mengecek directory yang dibutuhkan ada atau tidak. Dengan `strcpy` yang disimpan adalah directory yang diinput. 
```c
    else if(!strcmp(argv[1],"-d")) 
    {
        if(argc != 3)
        {
            printf ("Only Need 1 Path to Directory\n");
            exit(1);
            return 0;
        } 
        strcpy(baseDir,argv[2]);
    }
    else printf ("Invalid Argument\nList of Valid Arguments:\n-f\n-d\n*\n");

    //printf("%s\n",baseDir);
```

Kemudian karena soal 2.2 dan 2.3 rekursif, maka akan dicek terlebih dahulu berapa banyak file yang ada dengan menggunakan `listFileRecursively`. 
```c
    int fileCount = 0;
    if(!listFilesRecursively(baseDir, &fileCount))
    {
        printf("Yah, gagal disimpan :(\n");
        return 0;
    }

```

Di `listFilesRecursively` pertama adalah mencoba membuka directorynya degan `opendir` untuk mengecek apakah dia termasuk directory apa bukan. Jika bukan, maka akan direturn 0. 
Kemudian pada `while` untuk menulis nama file secara recursive. Yang pertama adalah mengecek nama file apakah termasuk file atau tidak dan apakah merupakan sebuah file reguler dengan memasukkannya ke dalam fungsi `checkExistAndRegFile`. Jika iya maka directorynya akan disimpan pada array global `char fileList[2048][PATH_MAX];` yang telah di buat sebelumnya. setelah disimpan, `fileCount` juga akan ditambah 1.
Maka setelahnya yang tersisa adalah fungsi recursivenya dan membuat path baru berdasarkan base path yang sebelumnya. Setelah semua selesai, maka akn di `closedir` dan return 1.
```c
int listFilesRecursively(char *basePath, int *fileCount)
{
    char path[PATH_MAX];
    struct dirent *dp;
    DIR *dir = opendir(basePath);

    if (!dir)
        return 0;

    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            char fullPath[PATH_MAX];
            sprintf(fullPath,"%s/%s",basePath,dp->d_name);
            //printf("%s\n", fullPath);
            if(checkExistAndRegFile(fullPath))
            {
                sprintf(fileList[*fileCount],"%s",fullPath);
                *fileCount += 1;
            }
            // Construct new path from our base path
            strcpy(path, basePath);
            strcat(path, "/");
            strcat(path, dp->d_name);

            listFilesRecursively(path,fileCount);
        }
    }

    closedir(dir);
    return 1;
}
```

Setelah mendapatkan fileCount dan merecursive, maka dibuat thread untuk mengelompokkan dan memindahkan file. Directorynya dimasukkan ke dalam `moveFile` dan dibandingkan dengan isi dari `fileList` yang ada sebelumnya. Dan setelahnya adalah tinggal menunggu hasilnya.
```c
    int fileCount = 0;
    if(!listFilesRecursively(baseDir, &fileCount))
    {
        printf("Yah, gagal disimpan :(\n");
        return 0;
    }

    //printf("file count = %d\n",fileCount);
    pthread_t tid[fileCount];
    for(int i = 0; i<fileCount; i++)
    {
        //printf("%s\n",fileList[i]);
        pthread_create( &(tid[i]), NULL, moveFile, (void*) fileList[i]);
    }

    for (int i = 0; i < fileCount; i++)
    {
        void *ptr;
        pthread_join( tid[i], &ptr);
    }

    if(!strcmp(argv[1],"-d")) printf("Direktori sukses disimpan!\n");
    return 0;
}
```

### 3.3 
Selain menerima opsi-opsi di atas, program ini menerima opsi * , contohnya ada di bawah ini:
> $ ./soal3 \*

Opsi ini akan mengkategorikan seluruh file yang ada di working directory ketika menjalankan program C tersebut.

#### Jawab
Pertama adalah mengecek apakah directory yang dibutuhkan ada. kemudian dengan menggunakan `getcwd` di ambil path yang telah disimpan dalam `baseDir` sebelumnya.
```c
    if(!strcmp(argv[1],"*"))
    {
        if(argc != 2)
        {
            printf ("* Didn't Need Another Argument\n");
            exit(1);
            return 0;
        }
        if (getcwd(baseDir, sizeof(baseDir)) == NULL) {
            perror("getcwd() error");
            return 0;
        }
    }
```
### 3.4
Semua file harus berada di dalam folder, jika terdapat file yang tidak memiliki ekstensi, file disimpan dalam folder “Unknown”. Jika file hidden, masuk folder “Hidden”.
#### Jawab 
Sudah ada dalam penjelasan soal 3.1.

### 3.5
Setiap 1 file yang dikategorikan dioperasikan oleh 1 thread agar bisa berjalan secara paralel sehingga proses kategori bisa berjalan lebih cepat.
#### Jawab
Sudah ada dalam penjelasan dalam soal2 sebelumnya.

Namun karena Ayub adalah orang yang hanya bisa memberi ide saja, tidak mau bantuin buat bikin programnya, Ayub meminta bantuanmu untuk membuatkan programnya. Bantulah agar program dapat berjalan!
Catatan: 
- Kategori folder tidak dibuat secara manual, harus melalui program C
- Program ini tidak case sensitive. Contoh: JPG dan jpg adalah sama
- Jika ekstensi lebih dari satu (contoh “.tar.gz”) maka akan masuk ke folder dengan titik terdepan (contoh “tar.gz”)
- Dilarang juga menggunakan fork-exec dan system()
- Bagian b dan c berlaku rekursif
