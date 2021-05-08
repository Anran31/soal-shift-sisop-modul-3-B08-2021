#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>
#include <limits.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <errno.h>
#define PORT 8080

void *connection_handler(void *socket_desc);
int tryLogin(int *auth, int sock, char *authUser, char *authPass);
int tryRegister(int sock);
int tryAdd(int sock, char *authUser, char *authPass);
void makeFilesDir();
void addDB(char *fileName,char *Publisher,char *Tahun,char *Filepath,char *exten);
void makeLog(char *fileName, char *User, char *Pass, char *status);
int tryDownload(int sock, char *bookName);
int tryDelete(char *bookName, char *user, char *pass);
int trySee(int sock);
int tryFind(int sock,char *fileName);

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

// void checkDestDir(char *destDir)
// {
//     int check = mkdir(destDir,0777);
// }

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
    //printf("%s\n",destination);
    // fp = fopen(destination, "w");
    // while (1) {
    //     n = read(sockfd, buffer, sizeof(buffer));
    //     printf("%d\n",n);
    //     if (n <= 0){
    //         break;
    //         //return;
    //     }
    //     fprintf(fp, "%s", buffer);
    //     bzero(buffer, PATH_MAX);
    // }
    // fclose(fp);
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
            //printf("muncul gak?\n"); 
			//printf("Ok received from client!\n");
			fclose(fr);
            return 1;
		}
    return 1;
}


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
    while ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)))
    {
            //puts("Connection accepted");
            //char *message;
            //Reply to the client
            //message = "Hello Client , I have received your connection. And now I will assign a handler for you\n";
            //write(new_socket , message , strlen(message));
            
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
            //puts("Handler assigned");
    }
    

    if ((new_socket)<0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    

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
void *connection_handler(void *socket_desc)
{
    //puts("In Connection Handler");
	// //Get the socket descriptor
	int sock = *(int*)socket_desc;
	int read_size;
	char *message , client_message[2000], cmd[2048];
	char *start = "Login or Register or Exit";
    char *authenticated = "Add or Download or Delete or See or Find or Exit";
    char loginMsg[PATH_MAX], loginStat[PATH_MAX], regMsg[PATH_MAX];
    char *regStat = "Register Successful";

    int auth = 0;
    char authUser[PATH_MAX], authPass[PATH_MAX],fullCommand[2048] = {0};
    int status;
    memset(cmd,0,sizeof(cmd));
    memset(fullCommand,0,sizeof(fullCommand));
    send(sock , start , strlen(start),0);
    while((read_size = read(sock , fullCommand , 2048))>0)
    {
        int cmdCount = 0;
        const char *l=" ";
        char *c,*d;
        //printf("%d\n",read_size);
        char tempCommand[2048] = {0},command[1024]={0}, file[1024]={0};
        strcpy(tempCommand,fullCommand);

        for( c=strtok_r(tempCommand,l,&d) ; c!=NULL ; c=strtok_r(NULL,l,&d) ) {
            if(cmdCount==0) strcpy(cmd,c);
            else if(cmdCount == 1) strcpy(file,c);

            cmdCount++;
        }

        //scanf("%s",command);
        for(int i = 0 ; i<strlen(cmd); i++)
        {
            cmd[i] = tolower(cmd[i]);   
        }

        //printf("%s\n",cmd);
        //printf("%s\n",cmd);

        if(!auth)
        {
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
            }
            else if(!strcmp(cmd,"register"))
            {
                if(tryRegister(sock)>0) 
                {
                    sprintf(regMsg,"%s\n\n%s",regStat,start);
                    send(sock , regMsg , strlen(regMsg),0);
                    //send(sock , start , strlen(start),0);
                }
                else
                {
                    read_size = 0;
                    break;
                }
            }
            else if(!strcmp(cmd,"exit"))
            {
                read_size = 0;
                break;
            }
            else{
                char *cmdNotFound = "Invalid Command";
                char invalCmd[PATH_MAX];
                sprintf(invalCmd,"%s\n\n%s",cmdNotFound,start);
                send(sock , invalCmd , strlen(invalCmd),0);
            }
            //send(sock , cmd , strlen(cmd),0);
            memset(cmd,0,sizeof(cmd));
            memset(fullCommand,0,sizeof(fullCommand));
        }
        else
        {
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
            else if(!strcmp(cmd,"delete"))
            {
                char *delStat[] = {"File Not Found\n", "Delete Success\n"};
                //read(sock,bookName,2048);
                //printf("%s\n",file);
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
            else if(!strcmp(cmd,"find"))
            {
                char *findStat[] = {"No Book Found", "All Book in Database"};
                //read(sock,bookName,2048);
                //printf("%s\n",file);
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
            else if(!strcmp(cmd,"exit"))
            {
                read_size = 0;
                break;
            }
            else{
                char *cmdNotFound = "Invalid Command";
                char invalCmd[PATH_MAX];
                sprintf(invalCmd,"%s\n\n%s",cmdNotFound,authenticated);
                send(sock , invalCmd , strlen(invalCmd),0);
            }
            //send(sock , cmd , strlen(cmd),0);
            memset(cmd,0,sizeof(cmd));
            memset(fullCommand,0,sizeof(fullCommand));
        }
    }

    if(read_size == 0)
	{
		puts("Client disconnected");
		fflush(stdout);
	}
	else if(read_size == -1)
	{
		perror("recv failed");
	}
		
	//Free the socket pointer
	free(socket_desc);
	
	return 0;
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
        //printf("%d\n",read_val);
        if(!read_val) return -1;
    }

    sprintf(namePass,"%s:%s\n",user,pwd);
    //printf("%s\n",namePass);

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
        //printf("Retrieved line of PATH_MAX %zu:\n", read);
        //printf("%s %s\n", creds,namePass);
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
        //printf("%d\n",read_val);
        if(!read_val) return -1;
    }

    sprintf(namePass,"%s:%s\n",user,pwd);
    //printf("%s\n",namePass);

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
//     Nama:
// Publisher:
// Tahun publishing:
// Ekstensi File : 
// Filepath : 
    // char nameCopy[1000] = {0};
    // strcpy(nameCopy,fileName);
    // printf("%s\n",nameCopy);
    // char *justName = strtok(nameCopy,'.');

    // const char *p=".";
    // char *a, *b;
    // char nameCopy[PATH_MAX];
    // strcpy(nameCopy,fileName);
    // char justName[100];
    // a=strtok_r(nameCopy,p,&b);
    // strcpy(justName,a);
    // printf("%s\n",justName);


    // char namaFile[PATH_MAX] = {0},pub[PATH_MAX] = {0}, thn[PATH_MAX] = {0},fpth[PATH_MAX] = {0},extensio[PATH_MAX] = {0};
    // sprintf(namaFile,"Nama: %s\n",justName);
    // fputs(namaFile, fPtr);
    // sprintf(pub,"Publisher: %s\n",Publisher);
    // fputs(pub, fPtr);
    // sprintf(thn,"Tahun publishing: %s\n",Tahun);
    // fputs(thn, fPtr);
    // sprintf(extensio,"Ekstensi File: %s\n",exten);
    // fputs(extensio, fPtr);
    // sprintf(fpth,"Filepath: %s\n\n",Filepath);
    // fputs(fpth, fPtr);
    char bookPath[1024] = {0}, log[PATH_MAX] = {0};
    sprintf(bookPath,"%s/FILES/%s",cwd,fileName);
    sprintf(log,"%s\t%s\t%s\n",Publisher,Tahun,bookPath);
    fputs(log,fPtr);
    fclose(fPtr);
}

void makeLog(char *fileName,char *User,char *Pass,char *Status)
{
    char cwd[PATH_MAX] = {0};
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd() error");
        return -1;
    }

    char filesPath[PATH_MAX] = {0};
    sprintf(filesPath,"%s/%s",cwd,"running.log");
    //printf("%s\n",filesPath);
    FILE * fPtr;

    fPtr = fopen(filesPath, "a");

    if(fPtr == NULL)
    {
        printf("Unable to create file.\n");
        return -1;
    }
//     Nama:
// Publisher:
// Tahun publishing:
// Ekstensi File : 
// Filepath : 

    char log[PATH_MAX] = {0};
    sprintf(log,"%s : %s (%s:%s)\n",Status,fileName,User,Pass);
    fputs(log, fPtr);
    fclose(fPtr);
}

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
            //printf("%s\n",Publisher);
        }
        else if (i == 1) 
        {
            read_val = read(sock , Tahun , sizeof(Tahun));
            //printf("%s\n",Tahun);
        }
        else
        {
            read_val = read(sock , Filepath , sizeof(Filepath));
            //printf("%s\n",Filepath);
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

        //printf("%s\n",fileName);
        //printf("%s\n",exten);
        int ret = write_file(sock,fileName);
        //printf("%d\n",ret);
        if(ret)
        {
            //printf("masuk\n");
            addDB(fileName,Publisher,Tahun,Filepath,exten);
            char status[10];
            strcpy(status,"Tambah");
            makeLog(fileName,authUser,authPass,status);
            //printf("return\n");
            return 1;
        }

    }
    else
    {
        // strcpy(msg,status[1]);
        // send(sock , msg , sizeof(msg),0);
        // memset(msg,0,sizeof(msg));        
        return -1;
    }
    
    
}

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

void sendFile(int sock, char *filePath)
{
    char sdbuf[PATH_MAX]; // Send buffer
    //printf("[Server] Sending %s to the Client...\n", filePath);
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
    //printf("Ok sent to client!\n");
}


int tryDownload(int sock, char *bookName)
{
    char filePath[2048] = {0};
    char clientRes[1024] = {0};
    if(checkFile(bookName,filePath))
    {
        char *suc = "downloading";
        send(sock,suc,strlen(suc),0);
        //printf("%s\n",filePath);
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
        //printf("%s\n",name);
        if (!strcmp(bookName,name)) {
            //printf("%s",line);
            // sprintf(filePath,"%s/FILES/%s",cwd,name);
            // fclose(f);
            // return 1;
            continue;
        }
        else fputs(line,fNew);
    }
    fclose(f);
    fclose(fNew);
    remove(tsvPath);
    rename(tempTSV,tsvPath);
}

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
        //printf("NamaFileExt:%s\n",NamaFileExt);
        getFileExt(NamaFileExt,Ekstensi);

        const char *q=".";
        char *c, *d;
        char nameCopy[1024];
        strcpy(nameCopy,NamaFileExt);
        c=strtok_r(nameCopy,q,&d);
        strcpy(Nama,c);
        // char NamaFile[1024] = {0};
        // strcpy(NamaFile,NamaFileExt);
        // *Nama = strtok(NamaFile,".");
        // *Ekstensi = strchr(NamaFileExt,'.')+1;
        char Format[4096] = {0};
        sprintf(Format,"Nama: %s\nPublisher: %s\nTahun publishing: %s\nEkstensi file : %s\nFilepath : %s\n",Nama,Publisher,Tahun,Ekstensi,Filepath);
        strcat(All,Format);
        strcat(All,"\n");
    }
    All[strlen(All)] = '\0';
    send(sock, All, sizeof(All), 0) ;
    bzero(All, 4096) ;
    // char cliRes[4096] ={0};
    // read(sock,cliRes,sizeof(cliRes));
    fclose(f);
    return 1;
}

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
        //printf("NamaFileExt:%s\n",NamaFileExt);
        getFileExt(NamaFileExt,Ekstensi);

        const char *q=".";
        char *c, *d;
        char nameCopy[1024];
        strcpy(nameCopy,NamaFileExt);
        c=strtok_r(nameCopy,q,&d);
        strcpy(Nama,c);
        // char NamaFile[1024] = {0};
        // strcpy(NamaFile,NamaFileExt);
        // *Nama = strtok(NamaFile,".");
        // *Ekstensi = strchr(NamaFileExt,'.')+1;
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
    // char cliRes[4096] ={0};
    // read(sock,cliRes,sizeof(cliRes));
    fclose(f);
    return 1;
}
