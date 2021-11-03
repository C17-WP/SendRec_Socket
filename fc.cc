#include <iostream>
#include <winsock2.h>  
#include <stdio.h>  
#include <windows.h>
#include <thread>
#include <mutex>          // std::mutex
#include <chrono>
#include "fc.h"
#include <conio.h>
#define IPErr 4294967295

using namespace std;

mutex mtx;  

#define MaxClient 5 //最大客户端存活数量
const int Platform = 1; //表示为服务器平台

#define MaxCon_Num 5        //最大连接数
#define Port_Con 15555      //连接端口
#define UDP_Broadcase 15556  //UDP广播端口
#define _WINSOCK_DEPRECATED_NO_WARNINGS

u_long IP_Input_Chack(int Platform = 0) {//IP地址输入函数-参数1_服务器监听IP-2_客户端目标服务器IP-3_常规IP输入

    char InputIP_T[16];
    u_long ip_d;
    while (1) {
        switch (Platform) {
        case 1:cout << "请输入监听IP地址(例如：192.168.1.1)\n>>"; break;
        case 2:cout << "请输入目标服务器IP地址(例如：192.168.1.1)\n>>"; break;
        default:cout << "请输入IP地址(例如：192.168.1.1)\n>>"; break;
        }

        cin >> InputIP_T;//储存输入文字
        cin.ignore(numeric_limits<streamsize>::max(),'\n');//清理缓冲区
        ip_d = inet_addr(InputIP_T);//将IP地址转化为无符号整数
        if (INADDR_NONE == ip_d) {//IP地址有效判断
            cout << "ERROR:地址无效" << endl;
            continue;
        }
        break;
    }
    return ip_d;
}



//多线程：监听函数 参数：TCP监听套接字 TCP连接信息 UDP监听套接字 客户端链表 ID 控制变量
bool Thread_rec(u_long LocalIP , char ID[16] , int *control , lnkList<dev_list> *Client_list) {

    int err;

//3-创建TCP服务器套接字
    SOCKET TCP_Local_Rec = socket(AF_INET, SOCK_STREAM, 0);//服务器TCP套接字  
    if (TCP_Local_Rec == INVALID_SOCKET)//套接字创建失败——输出错误信息-结束程序
    {
        err = WSAGetLastError();
        printf("socket error:%d\n", err);
        closesocket(TCP_Local_Rec);
        WSACleanup();
        return -1;
    }

    //需要绑定的参数，主要是本地的socket的一些信息。  
    SOCKADDR_IN A_TR_S;                  //参数类，下面会用bind进行绑定
    A_TR_S.sin_family = AF_INET;           //AF_INET表示为IPv4地址  
    A_TR_S.sin_addr.S_un.S_addr = LocalIP; //Socket监听的IP地址，inet_addr函数可以把IP地址字符串转化为网络地址
    //A_TR_S.sin_addr.S_un.S_addr=htonl(INADDR_ANY);//ip地址——此处INADDR_ANY表示的时0.0.0.0（不确定的地址）
    A_TR_S.sin_port = htons(Port_Con);     //绑定端口  

    bind(TCP_Local_Rec, (SOCKADDR*)&A_TR_S, sizeof(SOCKADDR_IN) );//绑定完成-(SOCKADDR*)&addr强制类型转换
//3-创建TCP服务器套接字结束

//UDP广播接收套接字创建
    SOCKET S_UR_B = socket(AF_INET, SOCK_DGRAM, 0);//服务器UDP套接字
    if (INVALID_SOCKET == S_UR_B)
    {
        err = WSAGetLastError();
        printf("socket error:%d\n", err);
        WSACleanup();
        return -1;
    }
    //创建本地地址信息并绑定
    SOCKADDR_IN A_UR_B;
    A_UR_B.sin_family = PF_INET;
    A_UR_B.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    A_UR_B.sin_port = htons(UDP_Broadcase);
    if (bind(S_UR_B, (SOCKADDR*)&A_UR_B, sizeof(SOCKADDR_IN)) != 0)
    {
        err = WSAGetLastError();
        printf("bind error:%d\n", err);
        closesocket(S_UR_B);
        WSACleanup();
        return -1;
    }

//4-UDP/接收套接字结束

    listen(TCP_Local_Rec, MaxCon_Num);      //开始监听连接，其中第二个参数代表能够接收的最多的连接数

    //与客户端连接过程
    SOCKADDR_IN A_TU_Client;   //创建客户端的地址结构体 -空白 
    int len = sizeof(SOCKADDR);   //储存SOCKADDR的大小
    int lenU = sizeof(SOCKADDR_IN);

    struct dev_list clt; //交换结构体变量

    char recv_buf[1031]; //缓冲区变量

    //lnkList<dev_list> Client_list ; //客户端存放链表


    fd_set fdRead = { 0 };
    timeval timeout = { 0 };
    timeout.tv_sec = 0;

    int nRet; //错误信息

    while (1)
    {
        if (*control == 1) {
            FD_ZERO(&fdRead);
            FD_SET(TCP_Local_Rec, &fdRead);
            FD_SET(S_UR_B, &fdRead);
            int nRet2;//select返回量
            nRet2 = select(0, &fdRead, NULL, NULL, &timeout);
            if (SOCKET_ERROR == nRet2)
            {
                nRet = WSAGetLastError();
                break;
            }
            else if (0 == nRet2)
            {
                continue;
            }
            else
            {
                if (FD_ISSET(TCP_Local_Rec, &fdRead)) {
                    SOCKET serConn = accept(TCP_Local_Rec, (SOCKADDR*)&A_TU_Client, &len);//如果这里不是accept而是conection的话。。就会不断的监听  

                    memset(recv_buf, '\0' , 1031);//清空缓存
                    // memset(&clt,0,sizeof(clt));//清空结构体
                    recv(serConn, recv_buf, 1031, 0);//读取数据
                    // memcpy(&clt,recv_buf,sizeof(clt));//把接收到的信息转换成结构体
                    cout << "接受来自" << inet_ntoa(A_TU_Client.sin_addr) << "的信息：" << endl << recv_buf << endl;
                    closesocket(serConn);
                }

                if (FD_ISSET(S_UR_B, &fdRead)) {
                    SOCKADDR_IN A_UU_Client;
                    char recu_buf[34];
                    memset(recu_buf, '\0' , 34);//清空缓存
                    //recvfrom(S_UR_B, recu_buf, 34 , 0 ,  (SOCKADDR*)&A_UU_Client, &lenU);//输入缓冲
                    recvfrom(S_UR_B, recu_buf, 34 , 0 ,  (SOCKADDR*)&A_UU_Client, &lenU);//输入缓冲
                    memset(&clt, 0, sizeof(clt));//清空结构体
                    memcpy(&clt, recu_buf, 34 );//把接收到的信息转换成结构体
                                    
                    dev_list locateDev;
                    Client_list->getValue(0, locateDev);

                    if ( clt.Con[0] == 0  ) {//0为 来自其它设备的请求 /* && !strcmp(clt.ip , locateDev.ip) */
                        if( !strcmp(clt.ip , locateDev.ip) ){ //本机请求忽略
                            continue ;
                        }
                        memcpy(clt.ip, inet_ntoa(A_TR_S.sin_addr), strlen(inet_ntoa(A_TR_S.sin_addr)) );
                        memcpy(clt.info_Name , ID , strlen(ID) );
                        clt.Con[0] = 'w';
                        clt.Con[1] = 'w';
                        memcpy(recu_buf, &clt, sizeof(clt)+1);
                        A_UU_Client.sin_port = htons(UDP_Broadcase);//发送端口号修改为接收端口-重要
                        sendto(S_UR_B, recu_buf,  34  , 0 , (struct sockaddr*)&A_UU_Client, sizeof(A_UU_Client));
                    }
                    else {//非0为其它设备 回应 本地设备的请求-记录下其它设备的信息
                        clt.Con[0] = 0;
                        Client_list->append(clt);
                        // cout << recu_buf;
                    }

                }
            }
        }
        else {

            if (*control == 0){
                //cout << "rec 正在休眠";
                Sleep(5 * 1000);//休眠5秒
            }
                
            else {
                break;
            }
        }
    }
    closesocket(TCP_Local_Rec);
    closesocket(S_UR_B);

    cout << "接收线程已结束";
    return true;
}
/*SOCKET* TCP_Local_Rec, SOCKADDR_IN* A_TR_S, SOCKET* S_UR_B, lnkList<dev_list>* Client_list, char ID[], int* control*/

//多线程：发送函数 参数：TCP监听套接字  TCP连接信息   UDP监听套接字            客户端链表               ID 控制变量
bool Thread_send(u_long  LocalIP  , char ID[16] , int *control , lnkList<dev_list> *Client_list , int *send_p) {
    int err = 0;//错误信息存放变量
    while(1){
        if(*control == 1){//发送消息
            //int err;
            char str[255];
            cout <<"要发送的文字（255）：";
            cin >> str;
            cin.ignore(numeric_limits<streamsize>::max(),'\n');//清理缓冲区
            //创建套接字
            dev_list send_dev;

            if(!Client_list->getValue( *send_p , send_dev )){
                cout << "位置错误";
                *control = 0;
                continue;
            }

            u_long send_ip = inet_addr(send_dev.ip);
            SOCKET S_TS_S = socket(PF_INET, SOCK_STREAM, 0);
            if (S_TS_S == INVALID_SOCKET)
            {
                err = WSAGetLastError();
                printf("socket error:%d\n", err);
                closesocket(S_TS_S);
                //WSACleanup();
                return -1;
            }
            //向服务器发起请求
            sockaddr_in A_TS_S;
            memset(&A_TS_S, 0, sizeof(A_TS_S));  //每个字节都用0填充
            A_TS_S.sin_family = PF_INET;
            A_TS_S.sin_addr.s_addr = send_ip;
            A_TS_S.sin_port = htons(Port_Con);

            if( connect(S_TS_S, (SOCKADDR*)&A_TS_S, sizeof(A_TS_S) ) != 0){
                err = WSAGetLastError();
                printf("Connect error:%d\n", err);
                closesocket(S_TS_S);
                *control = 0;
                continue;
            }

            //向客户端发送数据
            err = send(S_TS_S, str, strlen(str)+1 , 0 );
            if(err == SOCKET_ERROR){
                err = WSAGetLastError();
                printf("发送失败 ");
                Socket_Errinof(err);
                closesocket(S_TS_S);
                *control = 0;
                continue;
            }
            
            cout << "发送成功" <<endl ;
            closesocket(S_TS_S);
            *control = 0;

            continue;
        }

        if(*control == 2){//UDP广播
            SOCKET S_US_B = socket(AF_INET, SOCK_DGRAM, 0);//服务器UDP套接字
            if (S_US_B == INVALID_SOCKET)
            {
                err = WSAGetLastError();
                printf("socket error:%d\n", err);
                closesocket(S_US_B);
                //WSACleanup();
                return -1;
            }
            bool bOpt = true;
            //打开广播选项
            setsockopt(S_US_B, SOL_SOCKET, SO_BROADCAST, (char*)&bOpt, sizeof(bOpt));

            // 设置发往的地址
            SOCKADDR_IN A_US_B;
            memset(&A_US_B, 0, sizeof(A_US_B));
            A_US_B.sin_family = AF_INET;
            A_US_B.sin_addr.S_un.S_addr = htonl(INADDR_BROADCAST);
            A_US_B.sin_port = htons(UDP_Broadcase);
            int nSize = sizeof(SOCKADDR_IN);

            //准备广播
            char send_buf[34];
            dev_list cache_list;
            //int temp;
            Client_list->getValue( 0 , cache_list);
            cache_list.Con[0] = 0;
            memset(send_buf, '\0', sizeof(char)*34 );//清空缓存
            memcpy( send_buf, &cache_list, sizeof(cache_list)+1 );
            //temp = strlen(send_buf);
            err = sendto( S_US_B , send_buf , 34 , 0 , (struct sockaddr*)&A_US_B , sizeof(SOCKADDR_IN) );
            
            if(err == SOCKET_ERROR){
                err = WSAGetLastError();
                cout << endl << "发送失败，错误代码：" << err << endl;
                closesocket(S_US_B);
                *control = 0;
                continue;
            }
            cout << endl << "发送成功" <<endl ;
            //cout << send_buf;
            *control = 0;
            closesocket(S_US_B);
            continue;
        }

        if(*control == 0){
            //cout << "Send 正在休眠";
            Sleep(1000);
            continue;
        }

        break;
    }
    cout << "Send线程结束"<< endl;
    return true;
}

void Show_List(lnkList<dev_list>& palist){
    int l = palist.length();
    dev_list temp;
    cout << "所有搜寻到的设备："<<endl;
    for(int i = 0; i<l ;i++){
        palist.getValue(i, temp);
        cout << i << ":" << temp.ip << " " << temp.info_Name << endl;
    }
}


void Socket_Errinof(int Errcode) {
    switch (Errcode) {
    case 0:     printf(" Directly send error \n"); break;
    case 10004: printf(" 10004 - Interrupted function call一个封锁操作被对 WSACancelBlockingCall 的调用中断。\n"); break;
    case 10013: printf(" 10013 - Permission denied以一种访问权限不允许的方式做了一个访问套接字的尝试。\n"); break;
    case 10014: printf(" 10014 - Bad address系统检测到在一个调用中尝试使用指针参数时的无效指针地址。 \n"); break;
    case 10022: printf(" 10022 - Invalid argument提供了一个无效的参数。 \n"); break;
    case 10024: printf(" 10024 - Too many open files打开的套接字太多。 \n"); break;
    case 10035: printf(" 10035 - Resource temporarily unavailable无法立即完成一个非阻止性套接字操作。 \n"); break;
    case 10036: printf(" 10036 - Operation now in progress 目前正在执行一个阻止性操作。\n"); break;
    case 10037: printf(" 10037 - Operation already in progress 在一个非阻止性套接字上尝试了一个已经在进行的操作。\n"); break;
    case 10038: printf(" 10038 - Socket operation on non-socket 在一个非套接字上尝试了一个操作。\n"); break;
    case 10039: printf(" 10039 - Destination address required 请求的地址在一个套接字中从操作中忽略。\n"); break;
    case 10040: printf(" 10040 - Message too long 一个在数据报套接字上发送的消息大于内部消息缓冲区或其他一些网络限制，或该用户用于接收数据报的缓冲区比数据报小。\n"); break;
    case 10041: printf(" 10041 - Protocol wrong type for socket 在套接字函数调用中指定的一个协议不支持请求的套接字类型的语法。\n"); break;
    case 10042: printf(" 10042 - Bad protocol option在 getsockopt 或 setsockopt 调用中指定的一个未知的、无效的或不受支持的选项或层次。 \n"); break;
    case 10043: printf(" 10043 - Protocol not supported请求的协议还没有在系统中配置，或者没有它存在的迹象。 \n"); break;
    case 10044: printf(" 10044 - Socket type not supported 在这个地址家族中不存在对指定的插槽类型的支持。\n"); break;
    case 10045: printf(" 10045 - Operation not supported 参考的对象类型不支持尝试的操作。\n"); break;
    case 10046: printf(" 10046 - Protocol family not supported 协议家族尚未配置到系统中或没有它的存在迹象。\n"); break;
    case 10047: printf(" 10047 - Address family not supported by protocol family 使用了与请求的协议不兼容的地址。\n"); break;
    case 10048: printf(" 10048 - Address already in use通常每个套接字地址(协议/网络地址/端口)只允许使用一次。 \n"); break;
    case 10049: printf(" 10049 - Cannot assign requested address 在其上下文中，该请求的地址无效。\n"); break;
    case 10050: printf(" 10050 - Network is down套接字操作遇到了一个已死的网络 \n"); break;
    case 10051: printf(" 10051 - Network is unreachable 向一个无法连接的网络尝试了一个套接字操作。\n"); break;
    case 10052: printf(" 10052 - Network dropped connection on reset 当该操作在进行中，由于保持活动的操作检测到一个故障，该连接中断。\n"); break;
    case 10053: printf(" 10053 - Software caused connection abort 您的主机中的软件中止了一个已建立的连接。\n"); break;
    case 10054: printf(" 10054 - Connection reset by peer 远程主机强迫关闭了一个现有的连接。\n"); break;
    case 10055: printf(" 10055 - No buffer space available 由于系统缓冲区空间不足或队列已满，不能执行套接字上的操作。\n"); break;
    case 10056: printf(" 10056 - Socket is already connected 在一个已经连接的套接字上做了一个连接请求。\n"); break;
    case 10057: printf(" 10057 - Socket is not connected 由于套接字没有连接并且(当使用一个 sendto 调用发送数据报套接字时)没有提供地址，发送或接收数据的请求没有被接受。\n"); break;
    case 10058: printf(" 10058 - Cannot send after socket shutdown 由于以前的关闭调用，套接字在那个方向已经关闭，发送或接收数据的请求没有被接受。\n"); break;
    case 10060: printf(" 10060 - Connection timed out 由于连接方在一段时间后没有正确答复或连接的主机没有反应，连接尝试失败。\n"); break;
    case 10061: printf(" 10061 - Connection refused 由于目标计算机积极拒绝，无法连接。\n"); break;
    case 10064: printf(" 10064 - Host is down 由于目标主机坏了，套接字操作失败。\n"); break;
    case 10065: printf(" 10065 - No route to host 套接字操作尝试一个无法连接的主机。\n"); break;
    case 10067: printf(" 10067 - Too many processes 一个 Windows 套接字操作可能在可以同时使用的应用程序数目上有限制。\n"); break;
    case 10091: printf(" 10091 - Network subsystem is unavailable 因为它使用提供网络服务的系统目前无效，WSAStartup 目前不能正常工作。\n"); break;
    case 10092: printf(" 10092 - WINSOCK.DLL version out of range 不支持请求的 Windows 套接字版本。\n"); break;
    case 10093: printf(" 10093 - Successful WSAStartup not yet performed 应用程序没有调用 WSAStartup，或者 WSAStartup 失败。\n"); break;
    case 10094: printf(" 10094 - Graceful shutdown in progress \n"); break;
    case 11001: printf(" 11001 - Host not found 不知道这样的主机。\n"); break;
    case 11002: printf(" 11002 - Non-authoritative host not found 这是在主机名解析时通常出现的暂时错误，它意味着本地服务器没有从权威服务器上收到响应。\n"); break;
    case 11003: printf(" 11003 - This is a non-recoverable error 在数据库查找中出现一个不可恢复的错误。\n"); break;
    case 11004: printf(" 11004 - Valid name, no data record of requested type请求的名称有效，但是找不到请求的类型的数据。\n"); break;
    default: printf(" Errcode \n"); break;
    }
}
