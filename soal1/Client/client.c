#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>
#define PORT 8080

void getCreds(int sock, char *buffer, char *cmd)
{
    memset(buffer,0,2048);
    memset(cmd,0,2048);
    for(int i = 0; i<2;i++)
    {
        read( sock , buffer, 2048);
        printf("%s\n",buffer );
        memset(buffer,0,2048);
        // fflush(stdin);
        // fgets(cmd,sizeof(cmd),stdin);
        // int n = strcspn(cmd,"\n");
        // cmd[n] = 0;
        // printf("%d\n",n);
        scanf("%s",cmd);
        send(sock , cmd , strlen(cmd) , 0 );
        memset(cmd,0,2048);
    }
}

void send_file(FILE *fp, int sockfd){
  int n;
  char data[PATH_MAX] = {0};

//   while(1) {
//     fgets(data, PATH_MAX, fp);
//     if (send(sockfd, data, sizeof(data), 0) == -1) {
//       perror("[-]Error in sending file.");
//       return;
//       exit(1);
//     }
//     bzero(data, PATH_MAX);
//     if(feof(fp)) 
//     {
//         printf("EOF\n");
//         char *null;
//         int value = send(sockfd,NULL, 0, 0);
//         printf("%d\n",value);
//         break;
//     }
//   }
//   return;
        bzero(data, PATH_MAX); 
		int fs_block_sz; 
		while((fs_block_sz = fread(data, sizeof(char), PATH_MAX, fp)) > 0)
		{
		    if(send(sockfd, data, fs_block_sz, 0) < 0)
		    {
		        fprintf(stderr, "ERROR: Failed to send file. (errno = )\n");
		        break;
		    }
		    bzero(data, PATH_MAX);
		}
		//printf("Ok File from Client was Sent!\n");
}

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
            // char ch;
            // while ((ch = getchar()) != '\n' && ch != EOF);
            // fflush(stdin);
            // fgets(cmd,sizeof(cmd),stdin);
            // int n = strcspn(cmd,"\n");
            // cmd[n] = 0;
            //printf("%d\n",n);
            send(sock , cmd , strlen(cmd) , 0 );
        }
        else if(i==3)
        {
            if(!strcmp(buffer,"Downloading"))
            {
                //printf("%s\n",cmd);
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

int receiveFile(int sock, char *fileName)
{
    //printf("[Client] Receiveing file from Server and saving it as final.txt...");
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
	   //printf("Ok received from server!\n");
	    fclose(fr);
        char stat[1024] = {0};
        strcpy(stat,"finish");
        send(sock,stat,sizeof(stat),0);
	}
}

void recSee(int sock)
{
    char bufRec[4096];
    int recvSize;
    bzero(bufRec, 4096) ;
    // while((recvSize=read(sock, bufRec, 4096))>0)
    // {
    //     printf("%d\n",recvSize);
    //     printf("%s\n", bufRec) ;
    //     bzero(bufRec, 4096) ;
    // }
    recvSize=read(sock, bufRec, 4096);
    //printf("%d\n",recvSize);
    printf("%s", bufRec) ;
    bzero(bufRec, 4096) ;
    // printf("%d\n",recvSize);
    // char Stat[4096] = {0};
    // strcpy(Stat,"Finish");
    // send(sock,Stat,sizeof(Stat),0);
    return;
}

int main(int argc, char const *argv[]) {
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char *hello = "Hello from client";
    char buffer[2048] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
  
    memset(&serv_addr, '0', sizeof(serv_addr));
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
      
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
  
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    char command[2048];
    int menu = 1;
    int loop = 0;
    int Auth=0;
    while (1)
    {
        //printf("Auth = %d\n",Auth);
        if(menu)
        {
            valread = read( sock , buffer, 2048);
            printf("%s\n",buffer );
            memset(buffer,0,2048);
        }
        menu = 1;
        memset(command,0,2048); 

        int cmdCount = 0;
        const char *p=" ";
        char *a,*b;
        char fullCommand[2048] = {0},tempCommand[2048] = {0}, file[1024]={0};
        //printf("first full = %s\n",fullCommand);
        memset(fullCommand,0,sizeof(fullCommand));
        // int ch;
        //if(loop)while ((ch = getchar()) != '\n' && ch != EOF);
        // fflush(stdin);
        // fgets(fullCommand,sizeof(fullCommand),stdin);
        // fullCommand[strcspn(fullCommand,"\n")] = 0;
        scanf(" %[^\n]",fullCommand);
        //printf("after fgets = %s\n",fullCommand);
        strcpy(tempCommand,fullCommand);

        for( a=strtok_r(tempCommand,p,&b) ; a!=NULL ; a=strtok_r(NULL,p,&b) ) {
            if(cmdCount==0) strcpy(command,a);
            else if(cmdCount == 1) strcpy(file,a);

            cmdCount++;
        }

        //scanf("%s",command);
        send(sock , fullCommand, strlen(fullCommand) , 0 );
        for(int i = 0 ; i<strlen(command); i++)
        {
            command[i] = tolower(command[i]);   
        }
        //printf("%s\n",command);


        if(!strcmp(command,"register") && !Auth)
        {
            getCreds(sock, buffer,command);
            //loop++;
            // read( sock , buffer, 2048);
            // memset(buffer,0,2048);
            // read( sock , buffer, 2048);
            // memset(buffer,0,2048);
        }
        else if(!strcmp(command,"login") && !Auth)
        {
            getCreds(sock, buffer,command);
            //loop++;
            read(sock,&Auth,sizeof(Auth));
            //read( sock , buffer, 2048);
            //memset(buffer,0,2048);
        }
        else if(!strcmp(command,"add")&& Auth)
        {
            if(!addFile(sock, buffer,command)) menu = 0;
        }
        else if(!strcmp(command,"download")&& Auth)
        {
            char sta[20] = {0},respond[1024] = {0};
            memset(sta,0,sizeof(sta));
            read( sock , sta, sizeof(sta));
            strcpy(respond,"ok");
            send(sock,respond,sizeof(respond),0);
            //printf("%s\n",sta);
            if(!strcmp(sta,"downloading"))receiveFile(sock,file);
        }
        else if(!strcmp(command,"delete")&& Auth)
        {

        }
        else if(!strcmp(command,"see")&& Auth)
        {
            recSee(sock);
        }
        else if(!strcmp(command,"find")&&Auth)
        {
            int Stat = 0;
            read(sock,&Stat,sizeof(Stat));
            if(Stat) recSee(sock);
        }
        //printf("command message sent\n");
        else if(!strcmp(command,"exit"))
        {
            printf("Bye!\n");
            break;
        }
        memset(buffer,0,2048);
        memset(command,0,2048);
        // valread = read( sock , buffer, 2048);
        // printf("%s\n",buffer );
    }
    
    return 0;
}