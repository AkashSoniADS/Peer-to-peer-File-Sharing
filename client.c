#include"unp.h"
#include<errno.h>
#include<dirent.h>
#include<semaphore.h>

int cnt;
sem_t bin_sem;

//client structure to store the list of files shared by the client.
struct client
{
	int id;
	int fileno;
	char ipaddress[20];
	char *name;
	char list[1000][30];
};

//Function to send the list of files shared by this client to the server when it is connected
void sendlist(int *fd)
{
        int myfd= *fd;
        char path[30];
	char s[17];
        DIR *dir;
	FILE *f;
        struct dirent *dt;
       //Complete path for the files to be shared
        strcpy(path,"/home/");
        strcpy(path,strcat(path,getenv("USER")));
        strcpy(path,strcat(path,"/Desktop/"));
        strcpy(path,strcat(path,"/Sample/"));
       
         system("bash ip.sh"); // Running ip.sh file to store the ip address of the client.
        f=fopen("./ipaddress","r"); // opening ipaddress.txt to get the ipaddress
        fscanf(f,"%s",s);
	printf("Client's IP Address is %s\n",s);// printing clients ipaddress
        write(myfd,(void *)s,17);
        dir=opendir(path);
        if(dir!=NULL)// checking if the directory is null or not
        {
		printf("Directory opened.\n");
                char filename[30];
		strcpy(filename,"STARTOFFILE");
		write(myfd,(const void *)filename,30);
               // printing the list of files shared by the client
		printf("\nThe files shared by the client are: -\n\n");
                while((dt=readdir(dir))!=NULL)
                {
                        strcpy(filename,dt->d_name);
			if(strcmp(filename,".")!=0 && strcmp(filename,"..")!=0)
			{
				printf("%s\n",filename);
				write(myfd,(const void *)filename,30);
			}
                }
		printf("\n");
                strcpy(filename,"ENDOFFILE");
		write(myfd,(const void *)filename,30);
        }
}

//Function to print the list of files of all clients connected.
void printlist(int *fd)
{
	int myfd=*fd;
	int i;
	char c[2];
	int ret=0;
	char handshake[15];
	struct client *list;
	c[0]='l';
	c[1]='\0';
	write(myfd,(const void *)c,2);
	printf("Loading List from the Server. \nPlease Wait..\n");
	sleep((cnt/3)+1);
        // Waiting for the handshake
	do
	{
		read(myfd,(void *)handshake,15);
	}
	while(strcmp(handshake,"CNTSTART")!=0);
          
        // Getting the count of number of clients 
	do 
	{	ret=read(myfd,(void *)&cnt,sizeof(int));
	} while(ret==-1);
	list=(struct client *)malloc(cnt*sizeof(struct client)); // allocating the size for the list.
	do
	{	ret=read(myfd,(void *)list,cnt*sizeof(struct client)); // reading the list of files that are shared in the server
	}while(ret==-1);
	// printing the list of files shared by different clients
	for(i=0;i<cnt;i++)
	{
		int j;
		if(list[i].fileno != 0)
		printf("\nFiles of client %d: -\n",i+1);
		for(j=0;j<list[i].fileno;j++)
		{
			printf("%s\n",list[i].list[j]);
		}
	}
	printf("\n");
}

//Function to download the file from another client.
int download(void* confd)
{
                void *s;
                int f;
		char c[2];
		void *filebuf;
                char *filename;
		char *path;
		struct sockaddr_in servaddr;
		char remoteip[20];
                int clientno,i=0;
		int myfd=*(int *)confd;
		int dfd,n;
		int filecnt=30;
struct hostent *server;
            

		filebuf=(void *)malloc(4096);
		path=(char *)malloc(30);
		filename=(char *)malloc(filecnt);
		c[0]='d';
        	c[1]='\0';
        	write(myfd,(const void *)c,2);
               //Entering the details of the client from whom the file is to be downloaded
            	printf("Enter client number:");
		scanf("%d",&clientno);
		printf("Enter filename:\n");
		scanf("%s",filename);
		printf("Downloading %s. Please Wait..\n",filename);
                s=(void*)malloc(1000);
                //Writing the client no. in order to obtain its ip address
                write(myfd,(const void *)&clientno,sizeof(int));
		sleep(1);
                // Obtaining the ipaddress of the requested client
		read(myfd,(void *)remoteip,20*sizeof(char));
                //Printing client's Ip address
                printf("Client's Ipaddress is:");
                for(int i = 0;i<20;i++)
                {
                printf("%c",remoteip[i]);
                }
                printf("\n ");
		//Using another port to initiate client-client file transfer.
		dfd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		bzero(&servaddr, sizeof(servaddr));
	        servaddr.sin_family=AF_INET;
               server = gethostbyname(remoteip);
                if(server == NULL)
                {
                printf(stderr,"no such host");
}

                //copying the h_addr to sin.adr.s_addr
bcopy((char *) server -> h_addr , (char *) &servaddr.sin_addr.s_addr, server -> h_length);
		
	        servaddr.sin_port=htons(15000);//Initiating the port for the p2p tcp connecton
		sleep(3);
                if(connect(dfd,(SA *) &servaddr,sizeof(servaddr))<0)      
			printf("Connection problem.\n");

		write(dfd,(const void *)filename,30);//requesting the file
                //Mentioning the  path to downlod the file
		strcpy(path,"/home/");
                strcpy(path,strcat(path,getenv("USER")));
                strcpy(path,strcat(path,"/Downloads/"));
                strcpy(path,strcat(path,filename));
		f=open(path,O_CREAT | O_RDWR, 0755);// returns  a file descriptor for the given path
        while(1)
        {
               	n=read(dfd,filebuf,4096); // file send by the client is read.
                write(f,filebuf,4096);// file obtained is given to the file descriptor created.
                cnt++;
                if(n<=0) break;
        }
        free(filebuf);
        close(f);//Closing the file descriptor
        close(dfd);//closing socket
        printf("\nDownload finished.\n\n");
        return 1;
}

//Function to upload the file requested by another client.
void downloadrequestcheck(int *fd)
{
	int myfd=*(int *)fd;
	int sockfd,n;
	char c;
	void *filebuf;
	int f;
	struct sockaddr_in servaddr,cliaddr;
	char filename[30];
	char *path;
        int len,confd;



         filebuf=(void *)malloc(4096);//allocating the memory for filebuf
	path=(char *)malloc(30);//allocating the memory for the path
	read(myfd,(void *)&c,sizeof(char));
	printf("Receive Download request.\n");
	sem_wait(&bin_sem);// wait till the file is uploaded
		//Using another port for file transfer, this client acting as a server.
		sockfd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        	bzero(&servaddr,sizeof(servaddr));
        	servaddr.sin_family=AF_INET;


        	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
        	servaddr.sin_port=htons(15000);// opening port for conection with other client
        	bind(sockfd,(SA *) &servaddr,sizeof(servaddr));//binding
        	listen(sockfd,LISTENQ);//listening
		len=sizeof(cliaddr);
  		confd=accept(sockfd,(SA *) &cliaddr,&len); //accepting other client's request
		read(confd,(void *)filename,30); //reading the requeste filename which is to be uploaded
                //Providing the path inorder to upload the file 
		strcpy(path,"/home/");
	        strcpy(path,strcat(path,getenv("USER")));
                strcpy(path,strcat(path,"/Desktop/"));
        	strcpy(path,strcat(path,"/Sample/"));
		strcpy(path,strcat(path,filename));
		
		f=open(path,O_RDONLY); // opening the file descriptor for reading
                if(f<0)
                {
                        printf("File couldn't be opened");
                        return;
                }
                printf("Uploading %s\n",path);
     		while(1)
                {
                        n=read(f,filebuf,4096); //copying the file into filebuf
                        cnt++;
                        write(confd,(const void *)filebuf,4096); //writing filebuf to the socket
                        if(n<=0) break;
                }
                printf("Uploading Finished\n");
                free(filebuf);
                close(f);//closing file descriptor
                close(confd);//closing the socket
		printf("\nEnter choice:\n'l' to get list of files\n'd' to download file\n'q' to quit\n>");

	sem_post(&bin_sem);
}
//Main Funcion
int main(int argc,char *argv[])
 
{
	int myfd;
	void *c;
	pthread_t tid;
	socklen_t len;
	FILE* f;
	int dreturn;
	int ret=0;
	char *answer;
	char path[100]; 
       char *ip_addr;

	struct sockaddr_in servaddr,myaddr;
struct hostent *server;


if(argc<0)
{
fprintf(stderr,"Error in ports");
exit(1);
}


	c=(void*)malloc(1000);
	sem_init(&bin_sem, 0, 0);//semaphore initialization
	myfd=socket(AF_INET,SOCK_STREAM,0);//opening socket
 
server = gethostbyname(argv[1]);
if(server == NULL)
{
printf(stderr,"no such host");

}
	bzero(&servaddr, sizeof(servaddr));
	bzero(&myaddr,sizeof(myaddr));
	servaddr.sin_family=AF_INET;
bcopy((char *) server -> h_addr , (char *) &servaddr.sin_addr.s_addr, server -> h_length);
         

	servaddr.sin_port=htons(13000);//opening port for tcp connection
	bind(myfd,(SA *) &myaddr,sizeof(myaddr));
	len=sizeof(servaddr);
	if(connect(myfd,(SA *) &servaddr,len)!=0)
	{	printf("Connection problem.\n");
		exit(-1);
	}
	printf("\nConnection Established !!\n");
	sendlist(&myfd);//sending list of files shared by the client
	answer=(char *)malloc(5*sizeof(char));
	while(1)
	{
		pthread_create(&tid,NULL,downloadrequestcheck,&myfd);// creating thread to handle asynchronous download request 
                                                                     // coming from server.
		sem_post(&bin_sem);
		printf("Enter choice:\n'l' to get list of files\n'd' to download file\n'q' to quit\n>");
		scanf("%s",answer);
		pthread_cancel(tid);
		sem_wait(&bin_sem);
		if(strcmp(answer,"l")==0)
		{
			printf("\nList Request Sent to the Server.\n");
			printlist(&myfd);// printing list of all the files shared by various clients
		}
		else if(strcmp(answer,"d")==0)
		{
			printf("\nDownload Request Sent to the Server.\n");
			dreturn=download(&myfd);//sending download request
		}
                // quit
		else if(strcmp(answer,"q")==0) 

		{
			char c[2];
			c[0]='q';
			c[1]='\0';
			write(myfd,(const void *)c,2);
			exit(0);
		}
		else 
		{
			printf("Invalid choice. Please enter your choice again\n");
			fflush(stdin);
		}
	}
		return 0;
}
