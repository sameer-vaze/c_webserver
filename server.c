#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#define MAX 100						//set maximum number of requests 
#define MAXBUF 500						//to read ws.conf			
#define BUFSIZE 1024 					//to send via socket

int sock, clients[MAX];
char *root;
char fourofour[] = "<html><head><title>404</title></head><body><h1>404</h1><p>404 Not Found Reason URL does not exist:</p></body></html>";
char post_content[] = "<html><head><title>404</title></head><body><h1>Post Data</h1><pre></pre>404 Not Found Reason URL does not exist:</p></body></html>";
char formatnotsupported[] = "<html><head><title>Sorry</title></head><body><h1>This format is not supported</h1><p>Files with extension are not supported</p></body></html>";
char notimplemented[] = "<html><head><title>Sorry</title></head><body><h1>501</h1><p>The method %s has not been implemented</p></body></html>";
struct stat st;
char res[10];
FILE *fp;
	
typedef struct index 						//structure to store index terms
{
	char *array[3];
}indices;

typedef struct ll 							//node for linked list to store formats supported
{
	char format_extension[6];
	char format_description[17];
	struct ll *next;
}node;

node *head = NULL;							//global variable to point to first item in linked list
node *current = NULL;						//global variable to traverse through the linked list

void insertToLL(char ext[6], char desc[17])				//function that adds items to linked list
{
	int l;
	node *link = (node*) malloc(sizeof(node));
	printf("ext value is %s\n", ext);
	printf("string length of ext is %d\n", strlen(ext));
	for (l = 0; l < strlen(ext); l++)
	{
		link->format_extension[l] = ext[l];				//insert extension value
	}
	printf("string length of ext is %d\n", strlen(desc));	
	for (l = 0; l < strlen(desc); l++)
	{
		link->format_description[l] = desc[l];			//insert description
	}
	if(head == NULL)							//if element added is first element
	{
		head = link;
		link->next = NULL;
		current = link;
	}
	else										//if element added is any subsequent element
	{
		current->next = link;
		link->next = NULL;
		current = current->next;
	}
}

void displayLL()						//function to display the contents of the linked list
{
	node *start;
	start = head;
	while(start != NULL)
	{
		printf("Values in list are: %s\t%s\n", start->format_extension, start->format_description);
		start = start->next;
	}
}

void getExtension(char *name)			//function that extracts extension from file name
{
	int res_indx = 0, indx = strlen(name);			//set result index as zero and index of filename as the last element
	while(name[indx] != '.')						//decrement index till "." is encountered
	{
		//printf("indx is %d\n", indx);
		indx--;
	}
	while(name[indx] != '\0')						//while string doesn't end
	{
		res[res_indx] = name[indx];
		res_indx++;
		indx++;
	}
	res[res_indx+1] = '\0';							//make extension a NULL character terminated string
}

int checkValidExtension(char *test)				//checks if extension is supported
{
	node *start;								//linked list pointer to traverse linked list
	start = head;	
	int check = 0;								//flag to check if the extension is supported
	printf("checking %s\n", test);
	while(start != NULL)						//traverse through linked list
	{
		printf("checking with %s\n", start->format_extension);
		if(strcmp(test,start->format_extension) == 0)			//if string matches
		{
			check++;											//set flag as 1
			printf("check is %d\n", check);
		}
		start = start->next;							//next node of linked list
	}
	return check;
}

int extractPort(char *port_string)				//get port number from ws.conf
{
	char *tok = strtok(port_string, " ");
	tok = strtok (NULL, " ");
	return(atoi(tok));
}

char *extractRoot(char *root_string)			//get file root address for file from ws.conf
{
	char *tok1 = strtok(root_string, " \"");
	tok1 = strtok (NULL, " \"");
	return tok1;
}

indices extractIndex(char *index_string)		//get default index file names
{
	indices ind1;
	char *tok2 = strtok(index_string, " ");
	int i = 0;
	while (tok2 != NULL)											
	{
		if(i != 0)
		{
			ind1.array[i-1] = tok2;
		}
		tok2 = strtok (NULL, " ");
		i++;
	}
	return ind1;
}


void extractFormat(char *format_string, int iteration)		//get file formats(extensions) supported by the web server as per the ws.conf file
{
	char *tok3 = strtok(format_string, " ");
	char arr1[10];
	char arr2[20];
	int i = 0;
	printf("iteration is %d\n", iteration);
	while (tok3 != NULL)											
	{
		if(i == 0)
		{
			strcpy(arr1,tok3);
			printf("arr1 is %s\n", arr1);
		}
		else
		{
			strcpy(arr2,tok3);
			printf("arr2 is %s\n", arr2);
			printf("string length of format_description is %d\n", strlen(tok3));
		}
		tok3 = strtok (NULL, " ");
		i++;
	}
	insertToLL(arr1, arr2);								//function call to add extension and description to linked list
}

void serverStart(int port)								//funtion that sets address, family, port number to socket. It creates socket, binds socket and listens to any incoming requests
{
	struct sockaddr_in sin;
	bzero((char *)&sin, sizeof(sin));
	sin.sin_family = AF_INET;							//set family as the internet family (AF_INET)
	sin.sin_addr.s_addr = INADDR_ANY;					//set any address as the address for the socket
	sin.sin_port = htons(port);							//set the port number to the socket
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)	//open socket  and check if error occurs
	{
		printf("Error in creating socket \n");
		exit(1);
	}
	int optval = 1;
  	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));		//prevents the "already in use" issue
	printf("Socket created\n");
	if ((bind(sock, (struct sockaddr *)&sin, sizeof(sin)))	 < 0)		//bind socket and check if error occurs
	{
		printf("%d\n", bind(sock, (struct sockaddr *)&sin, sizeof(sin)));
		//	printf("Error in binding socket\n");
		perror("Bind error: ");
		exit(1);
	}
	printf("Socket bound\n");	
	if(listen(sock, 4) != 0)							//listen for incoming requests on the socket we have created
	{
		printf("Error in listen\n");
		exit(1);
	}
	printf("Listening for connections\n");	
}

void parseRequest(int n)								//function that parses the requests and returns the necessary pages
{
	int rcv, bytes, char_index, x, content_length;
	FILE *fd;
	char msg[MAXBUF], content_type[] = "Content-Type:<>\r\n", invalid[500], httptext[50], incorrectmethod[100],*command, *httpversion, *filename, extension[10], path[MAXBUF], BUFFER[BUFSIZE];
	bzero(msg, sizeof(msg));
	rcv = recv(clients[n], msg, MAXBUF, 0);
	printf("Strlen is %d\n", strlen(msg));
	char copy[strlen(msg)];
	strcpy(copy, msg);
	if(rcv < 0)										//if there is an error in sending the request from the client side
	{
		printf("Error in recv()\n");
	}
	else if(rcv == 0)								//if there are no requests received from client
	{
		printf("No client\n");
	}
	else											//if a request message is received
	{
		printf("%s\n", msg);
		command = strtok(msg, " \t\n");				//extract the method (GET, PUT, POST, MOVE, COPY, DEFINE etc)
		printf("Value of command now is %s\n", command);
		if(strncmp(command, "GET\0", 4) == 0)		//if the method is GET
		{
			filename = strtok(NULL, " \t");			//extract filename
			httpversion = strtok(NULL, " \t\n");	//extract the HTTP version
			if(strncmp(httpversion, "HTTP/1.0", 8)!=0 && strncmp(httpversion, "HTTP/1.1", 8)!=0)		//handle invalid version
			{
				sprintf(invalid, "<html><head><title>Sorry</title></head><body><h1>Not supported</h1><p>This HTTP version is not supported</p></body></html>", httpversion);	
				//printf("invalidversion is %s\n", invalidversion);
				sprintf(content_type,"HTTP/1.0 400 Bad Request\r\nContent-Type:%s\r\nContent-Length:%d\n\n", ".html", strlen(invalid));
				write(clients[n], content_type, strlen(content_type));
				write(clients[n], invalid, strlen(invalid));
			}
			else
			{
				if(strncmp(filename, "/\0", 2) == 0)				//if no filename specified set filename to index.html
				{
					filename = "/index.html";
				}
				getExtension(filename);								//function call to extract extension from filename
				char_index = 0;
				while(res[char_index] != '\0')
				{
					extension[char_index] = res[char_index];		//used global char array as functions can't return arrays
					char_index++;
				}
				printf("Extension is %s\n", extension);
				printf("Filename is %s\n", filename);
				int x = checkValidExtension(extension);				//check if extension is supported
				printf("x is %d\n", x);
				strcpy(path, root);									//store root string into the variable path to be used henceforth
				strcpy(&path[strlen(root)],filename);				//append filename to path
				printf("%s\n", path);
				if(x != 1)											//if format is not supported
				{
					sprintf(formatnotsupported, "<html><head><title>Sorry</title></head><body><h1>This format is not supported</h1><p>Files with extension %s are not supported</p></body></html>", extension);
					if(strncmp(httpversion, "HTTP/1.0", 8) == 0)
					{
						sprintf(content_type,"HTTP/1.0 501 Not Implemented\r\nContent-Type:%s\r\nContent-Length:%d\n\n", ".html", strlen(formatnotsupported));
						write(clients[n], content_type, strlen(content_type));
						//write(clients[n], "HTTP/1.0 501 Not Implemented\n\n", 30);
						// write(clients[n], "HTTP/1.0 200 OK\n\n", 17);
					}
					else
					{
						sprintf(content_type,"HTTP/1.1 501 Not Implemented\r\nContent-Type:%s\r\nContent-Length:%d\n\n", ".html", strlen(formatnotsupported));
						write(clients[n], content_type, strlen(content_type));
						// write(clients[n], "HTTP/1.1 501 Not Implemented\n\n", 30);
						// write(clients[n], "HTTP/1.1 200 OK\n\n", 17);
					}
					write(clients[n], formatnotsupported, strlen(formatnotsupported));
					exit(1);
				}
				else												//if format is supported
				{ 
					if((fd = open(path, O_RDONLY))!= -1)			//open file
					{
						stat(path, &st);							//stat to determine file size
						content_length = st.st_size;
						printf("File open\n");
						if(strncmp(httpversion, "HTTP/1.0", 8) == 0)
						{
							sprintf(content_type,"HTTP/1.0 200 OK\r\nContent-Type:%s\r\nContent-Length:%d\n\n", extension, content_length);
							write(clients[n], content_type, strlen(content_type));
						}
						else
						{
							sprintf(content_type,"HTTP/1.1 200 OK\r\nContent-Type:%s\r\nContent-Length:%d\n\n", extension, content_length);
							write(clients[n], content_type, strlen(content_type));
						}
						while((bytes = read(fd, BUFFER, BUFSIZE)) > 0)
						{
							write(clients[n], BUFFER, bytes);
						}
						fclose(fd);
					}
					else											//handle file not found exception
					{
						sprintf(fourofour, "<html><head><title>404</title></head><body><h1>404</h1><p>404 Not Found Reason URL does not exist: %s</p></body></html>", filename);						
						if(strncmp(httpversion, "HTTP/1.0", 8) == 0)
						{
							sprintf(content_type,"HTTP/1.0 404 Not Found\r\nContent-Type:%s\r\nContent-Length:%d\n\n", ".html", strlen(fourofour));
							write(clients[n], content_type, strlen(content_type));
							// write(clients[n], "HTTP/1.0 404 Not Found\n\n", 24);
							// write(clients[n], "HTTP/1.0 200 OK\n\n", 17);
						}
						else
						{
							sprintf(content_type,"HTTP/1.1 404 Not Found\r\nContent-Type:%s\r\nContent-Length:%d\n\n", ".html", strlen(fourofour));
							write(clients[n], content_type, strlen(content_type));
							//write(clients[n], "HTTP/1.1 404 Not Found\n\n", 24);
							// write(clients[n], "HTTP/1.1 200 OK\n\n", 17);
						}
						write(clients[n], fourofour, strlen(fourofour));						
					}
				}
			}
		}
		else if(strncmp(command, "POST\0", 5) == 0)				//if method is POST, the parsing and reply is identical to GET with an addditional mention of the Post Data
		{
			sprintf(post_content, "<html><head></head><body><h1>Post Data</h1><pre>%s</pre></body></html>", copy);			//Post data addition				
			// printf("post_content is \n%s\n", copy);
			
			filename = strtok(NULL, " \t");			//extract filename
			httpversion = strtok(NULL, " \t\n");	//extract the HTTP version
			if(strncmp(httpversion, "HTTP/1.0", 8)!=0 && strncmp(httpversion, "HTTP/1.1", 8)!=0)		//handle invalid version
			{
				printf("httpversion is %s\n", httpversion);
				sprintf(invalid, "<html><head><title>Sorry</title></head><body><h1>Not supported</h1><p>This HTTP version is not supported</p></body></html>", httpversion);	
				printf("invalidversion is %s\n", invalid);
				sprintf(content_type,"HTTP/1.0 400 Bad Request\r\nContent-Type:%s\r\nContent-Length:%d\n\n", ".html", strlen(invalid));
				write(clients[n], content_type, strlen(content_type));
				//write(clients[n], "HTTP/1.0 400 Bad Request\n\n", 26);
				// write(clients[n], "HTTP/1.0 200 OK\n\n", 17);
				//write(clients[n], post_content, strlen(post_content));					//send POST data
				write(clients[n], invalid, strlen(invalid));
			}
			else
			{
				if(strncmp(filename, "/\0", 2) == 0)				//if no filename specified set filename to index.html
				{
					filename = "/index.html";
				}
				printf("This is filename %s\n", filename);
				getExtension(filename);								//function call to extract extension from filename
				char_index = 0;
				while(res[char_index] != '\0')
				{
					extension[char_index] = res[char_index];		//used global char array as functions can't return arrays
					char_index++;
				}
				printf("Extension is %s\n", extension);
				printf("Filename is %s\n", filename);
				int x = checkValidExtension(extension);				//check if extension is supported
				printf("x is %d\n", x);
				strcpy(path, root);									//store root string into the variable path to be used henceforth
				strcpy(&path[strlen(root)],filename);				//append filename to path
				printf("%s\n", path);
				if(x != 1)											//if format is not supported
				{
					sprintf(formatnotsupported, "<html><head><title>Sorry</title></head><body><h1>This format is not supported</h1><p>Files with extension %s are not supported</p></body></html>", extension);
					if(strncmp(httpversion, "HTTP/1.0", 8) == 0)
					{
						sprintf(content_type,"HTTP/1.0 501 Not Implemented\r\nContent-Type:%s\r\nContent-Length:%d\n\n", ".html", strlen(formatnotsupported));
						write(clients[n], content_type, strlen(content_type));
						//write(clients[n], "HTTP/1.0 501 Not Implemented\n\n", 30);
						// write(clients[n], "HTTP/1.0 200 OK\n\n", 17);
					}
					else
					{
						sprintf(content_type,"HTTP/1.1 501 Not Implemented\r\nContent-Type:%s\r\nContent-Length:%d\n\n", ".html", strlen(formatnotsupported));
						write(clients[n], content_type, strlen(content_type));
						// write(clients[n], "HTTP/1.1 501 Not Implemented\n\n", 30);
						// write(clients[n], "HTTP/1.1 200 OK\n\n", 17);
					}
					//write(clients[n], post_content, strlen(post_content));					//send POST data
					write(clients[n], formatnotsupported, strlen(formatnotsupported));
					exit(1);
				}
				else												//if format is supported
				{ 
					if((fd = open(path, O_RDONLY))!= -1)			//open file
					{
						stat(path, &st);							//stat to determine file size
						content_length = st.st_size;
						printf("File open\n");
						if(strncmp(httpversion, "HTTP/1.0", 8) == 0)
						{
							sprintf(content_type,"HTTP/1.0 200 OK\r\nContent-Type:%s\r\nContent-Length:%d\n\n", extension, content_length);
							write(clients[n], content_type, strlen(content_type));
						}
						else
						{
							sprintf(content_type,"HTTP/1.1 200 OK\r\nContent-Type:%s\r\nContent-Length:%d\n\n", extension, content_length);
							write(clients[n], content_type, strlen(content_type));
						}
						write(clients[n], post_content, strlen(post_content));					//send POST data
						while((bytes = read(fd, BUFFER, BUFSIZE)) > 0)
						{
							write(clients[n], BUFFER, bytes);
						}
						fclose(fd);
					}
					else											//handle file not found exception
					{
						sprintf(fourofour, "<html><head><title>404</title></head><body><h1>404</h1><p>404 Not Found Reason URL does not exist: %s</p></body></html>", filename);						
						if(strncmp(httpversion, "HTTP/1.0", 8) == 0)
						{
							sprintf(content_type,"HTTP/1.0 404 Not Found\r\nContent-Type:%s\r\nContent-Length:%d\n\n", ".html", strlen(fourofour));
							write(clients[n], content_type, strlen(content_type));
							// write(clients[n], "HTTP/1.0 404 Not Found\n\n", 24);
							// write(clients[n], "HTTP/1.0 200 OK\n\n", 17);
						}
						else
						{
							sprintf(content_type,"HTTP/1.1 404 Not Found\r\nContent-Type:%s\r\nContent-Length:%d\n\n", ".html", strlen(fourofour));
							write(clients[n], content_type, strlen(content_type));
							//write(clients[n], "HTTP/1.1 404 Not Found\n\n", 24);
							// write(clients[n], "HTTP/1.1 200 OK\n\n", 17);
						}
						//write(clients[n], post_content, strlen(post_content));					//send POST data
						write(clients[n], fourofour, strlen(fourofour));						
					}
				}
			}
		}
		else if(strncmp(command, "PUT\0", 4) == 0 || strncmp(command, "DELETE\0", 7) == 0 || strncmp(command, "HEAD\0", 5) == 0 || strncmp(command, "PATCH\0", 6) == 0 || strncmp(command, "OPTIONS\0", 8) == 0 || strncmp(command, "PROPFIND\0", 9) == 0 || strncmp(command, "COPY\0", 5) == 0 || strncmp(command, "MOVE\0", 5) == 0) //handle all other methods as not implemented
		{
			//Handle not implemented error
			sprintf(notimplemented, "<html>\n<head>\n<title>501</title>\n</head>\n<body>\n<h1>501</h1>\n<p>501 Not Implemented method %s has not been implemented</p>\n</body>\n</html>\n", command);
			sprintf(content_type,"HTTP/1.0 501 Not Implemented\r\nContent-Type:%s\r\nContent-Length:%d\n\n", ".html", strlen(notimplemented));
			//write(clients[n], "HTTP/1.0 501 Not Implemented\n\n", 30);
			// write(clients[n], "HTTP/1.0 200 OK\n\n", 17);
			write(clients[n], content_type, strlen(content_type));
			write(clients[n], notimplemented, strlen(notimplemented));
			exit(1);		
		}
		else					//handle incorrect method name
		{
			// write(clients[n], "HTTP/1.0 400 Bad Request\n\n", 26);
			// write(clients[n], "HTTP/1.0 200 OK\n\n", 17);
			sprintf(incorrectmethod, "<html><head><title>400</title></head><body><h1>400</h1><p>400 Bad Request method %s is incorrect</p></body></html>", command);
			sprintf(content_type,"HTTP/1.0 400 Bad Request\r\nContent-Type:%s\r\nContent-Length:%d\n\n", ".html", strlen(incorrectmethod));
			write(clients[n], content_type, strlen(content_type));
			write(clients[n], incorrectmethod, strlen(incorrectmethod));
			exit(1);
		}
	}
}

void main()
{
	char buf[MAXBUF], *buf1, *root1;
	socklen_t len;
	int opt = 0, port_no, slot = 0;
	indices ind;
	struct sockaddr_in clientaddr;
	if ((fp = fopen("ws.conf","r")) == NULL)					//check if ws.conf file exists
	{
		printf("Configuration file ws.conf not found.\nExiting\n");
		exit(1);
	}
	else														//extract data if ws.conf does exist
	{
		while(!feof(fp))
		{
			fgets(buf, MAXBUF, fp);
			strtok(buf, "\n");
			if (opt == 1)										//flag to check if port number is being read
			{
				printf("Entered port\n");
				port_no = extractPort(buf);						//function call to extract port number
				printf("Port number is: %d\n", port_no);
				if(port_no < 1024)								//if port number is less than 1024, handle exception
				{
					printf("Port number %d is reserved for other applications\n", port_no);
					exit(1);
				}
			}
			else if (opt == 2)									//flag to check if root address is being read
			{
				printf("Entered root\n");
				root1 = extractRoot(buf);						//function call to extract root address
				root = (char *)malloc(strlen(root1));
				strcpy(root, root1);
				printf("The directory is:%s\n", root);
			}
			else if (opt == 3)									//flag to check if index values are being read
			{
				printf("Entered index\n");
				ind = extractIndex(buf);						//function call to extract indices
				printf("The indices are: \n");
				printf("%s\n", ind.array[0]);
				printf("%s\n", ind.array[1]);
				printf("%s\n", ind.array[2]);
			}
			else if (opt == 4)									//flag to check if formats are being read
			{
				printf("Entered format\n");
				for (int j = 0; j < 10; j++)
				{
					printf("Value of buf is: %s\n", buf);
					extractFormat(buf,j);						//extract formats from ws.conf file
					fgets(buf, MAXBUF, fp);
				}
				displayLL();
			}

			if(strcmp(buf, "#serviceport number") == 0)			//set flag for port number read
			{
				opt = 1;
			}
			else if (strcmp(buf, "#document root") == 0)		//set flag for root address read
			{
				opt = 2;
			}
			else if (strcmp(buf, "#default web page") == 0)		//set flag for default page read
			{
				opt = 3;
			}
			else if (strcmp(buf, "#Content-Type which the server handles") == 0)	//set flag for file format read
			{
				opt = 4;
			}
			else												//otherwise
			{
				opt = 0;
			}
		}
	}
	
	for (int n = 0; n < MAX; n++)								//set all new socket values as -1
	{
		clients[n] = -1;
	}

	printf("port is: %d\n", port_no);

	serverStart(port_no);										//function call to start server

	while(1)
	{
		len = sizeof(clientaddr);
		clients[slot] = accept(sock, (struct sockaddr *)&clientaddr, &len);			//create new socket when new request is received
		printf("Value of slot is: %d\n", slot);
		if (clients[slot]<0)									//if the new socket is not created
		{
			printf("Error in accept\n");
		}
		else													//call fork
		{
			if(fork() == 0)										//when forked successfully
			{
				parseRequest(slot);								//set new socket descriptor to array clients at index number slot
				exit(1);
			}
		}
		close(clients[slot]);
		while (clients[slot]!=-1) 								//increment to next available slot
		{
			slot = (slot+1)%MAX;
		}
	}
}