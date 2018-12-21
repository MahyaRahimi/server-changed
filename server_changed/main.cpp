#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <mutex>
#include <condition_variable>

/************************************************************************************************************************************************/
/*                            This function listens to TCP, connects to the antenna,                                                                 */
/*                            receives the frames and sends them to inputbuffer                                                                 */
/************************************************************************************************************************************************/


 std::mutex m;                    //for exclusive access to memory
 std::condition_variable cv;      //blocks the calling thread until notified to resume
 int in_buffer_sign;                        /*a sign for transferring the input to inputbuffer (it is not possible
                                             to define friend classes to transfer in_buffer_sign or myvector)*/
 std::vector<unsigned char> myvector;      //each element of myvector is a byte of data


int main()
{

    int listenfd = 0,connfd = 0;

    struct sockaddr_in serv_addr;
    char buffer[100];
    char sendBuff[1025];
    int n;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    printf("socket retrieve success\n");

    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, 0, sizeof(sendBuff));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(4000);

    bind(listenfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr));

    if(listen(listenfd, 10) == -1){
        printf("Failed to listen\n");
        return -1;
    }
    connfd = accept(listenfd, (struct sockaddr*)NULL ,NULL); // accept awaiting request
    strcpy(sendBuff, "Message from server");
    write(connfd, sendBuff, strlen(sendBuff));
    /*--------------------------debug-----------------------------------------------------------------------------------------------------------------------------------------------------*/
    std::cout<<"1 do print::::\nbuffer is:";
    for (int i=0; i<100; i++)
        std::cout << int(buffer[i]) << " ";
    std::cout << "1 myvector befor filling is: ";
    int k=0;
    for (auto v : myvector)
    {
            std::cout << k << ":" << std::hex << int(v) << "  ";
            k++;
    }
    std::cout<<std::endl;
    /*--------------------------debug-----------------------------------------------------------------------------------------------------------------------------------------------------*/


    while(1)
    {
        /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
        /* lock to take the input: */
        {
            std::unique_lock<std::mutex> lk(m);

            memset(buffer, 0, sizeof(buffer));
            n = read(connfd, buffer ,100);
            if (n < 0)
            {
                printf("ERROR reading from socket\n");
                close(connfd);
            }
            else{
                printf("Here is the message: %s\n",buffer);
                n = write(connfd,"I got your message",18);
                if (n < 0) printf("ERROR writing to socket");
                std::copy(&buffer[0], &buffer[100], back_inserter(myvector));
            }
            in_buffer_sign = 1;         //if in_buffer_sign is equal to 1, after notification inputbuffer will be unlocked.

        }
        /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

        /* notify inputbuffer thread to finish waiting and start: */
        cv.notify_all();

        /*--------------------------debug-----------------------------------------------------------------------------------------------------------------------------------------------------*/
        std::cout << "new listened vector is: ";
        for (auto v : myvector)
                std::cout << std::hex << int(v) << "  ";
        std::cout << std::endl;
        /*--------------------------debug-----------------------------------------------------------------------------------------------------------------------------------------------------*/

        /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
        /*wait till inputbuffer thread saves myvector in myinputbuffer and change in_buffer_sign to 0: */
       /* {
            std::unique_lock<std::mutex> lk(m);
            while(in_buffer_sign) cv.wait(lk);
        }*/
        /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
        /*--------------------------debug-----------------------------------------------------------------------------------------------------------------------------------------------------*/
        std::cout<<"back to the listner" <<std::endl;
        /*--------------------------debug-----------------------------------------------------------------------------------------------------------------------------------------------------*/

        /* clear myvector to be able to fill in with new data: */
        myvector.clear();
    }
    close(connfd);
    return 0;

}
