#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <json/json.h>
#include <iostream>
using namespace std;

#define ERR_EXIT(m) \
    do { \
        perror(m);\
        exit(EXIT_FAILURE);\
    }while(0)

void do_service(int sockfd);
void print_json(Json::Value data);

int main(int argc, const char *argv[])
{
    int peerfd = socket(PF_INET, SOCK_STREAM, 0);
    if(peerfd == -1)
        ERR_EXIT("socket");

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("192.168.32.128"); //localhost
    //addr.sin_addr.s_addr = INADDR_ANY; //localhost
    addr.sin_port = htons(9999);
    socklen_t len = sizeof addr;
    if(connect(peerfd, (struct sockaddr*)&addr, len) == -1)
        ERR_EXIT("Connect");

	char buf[1024];
	memset(buf, 0, sizeof(buf));
	int num;
	read(peerfd, &num, 4);
	read(peerfd, buf, num);
	printf("%s\n", buf);

    do_service(peerfd);


    return 0;
}



void do_service(int sockfd)
{
    char recvbuf[1024] = {0};
    char sendbuf[1024] = {0};
	unsigned int num;
    while(1)
    {
        fgets(sendbuf, sizeof sendbuf, stdin);
        write(sockfd, sendbuf, strlen(sendbuf));

        //read
		int nnum = read(sockfd, &num, 4);
        if(nnum == -1)
        {
            if(errno == EINTR)
                continue;
            ERR_EXIT("read");
        }
        else if(nnum == 0)
        {
            printf("server close!\n");
            close(sockfd);
            exit(EXIT_SUCCESS);
		}

        int nread = read(sockfd, recvbuf, num);
        if(nread == -1)
        {
            if(errno == EINTR)
                continue;
            ERR_EXIT("read");
        }
        else if(nread == 0)
        {
            printf("server close!\n");
            close(sockfd);
            exit(EXIT_SUCCESS);
        }

		Json::Reader reader;  
		Json::Value value;
		Json::Value::Members mem;
		if (!reader.parse(recvbuf, value)){
			return;
		}
		for (const auto & val : value){
			cout << val.asString() << endl;
		}

		cout << "\n" << endl;

        memset(recvbuf, 0, sizeof recvbuf);
        memset(sendbuf, 0, sizeof sendbuf);
    }
}

void print_json(Json::Value data)
{  
	Json::Value::Members mem = data.getMemberNames();  
	for (auto iter = mem.begin(); iter != mem.end(); iter++)  
	{  
		cout << *iter << "\t: ";  
		if (data[*iter].type() == Json::objectValue)  
		{  
			cout << endl;  
			print_json(data[*iter]);  
		}  
		else if (data[*iter].type() == Json::arrayValue)  
		{  
			cout << endl;  
			auto cnt = data[*iter].size();  
			for (size_t i = 0; i < cnt; i++)  
			{  
				print_json(data[*iter][i]);  
			}  
		}  
		else if (data[*iter].type() == Json::stringValue)  
		{  
			cout << data[*iter].asString() << endl;  
		}  
		else if (data[*iter].type() == Json::realValue)  
		{  
			cout << data[*iter].asDouble() << endl;  
		}  
		else if (data[*iter].type() == Json::uintValue)  
		{  
			cout << data[*iter].asUInt() << endl;  
		}  
		else  
		{  
			cout << data[*iter].asInt() << endl;  
		}  
	}  
	return;  
}  



