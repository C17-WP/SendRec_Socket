#include <iostream>
#include <winsock2.h>  
#include <stdio.h>  
#include <windows.h>
#include <thread>
#include <mutex>
#include <chrono>
#include <conio.h>
using namespace std;

#pragma comment(lib,"ws2_32.lib")//隐式加载库文件

#include "fc.h"

const int Platform = 1; //表示为服务器平台


#define MaxCon_Num 5        //最大连接数
#define Port_Con 15555      //连接端口
#define UDP_Broadcase 15556  //UDP广播端口

//#define A_TR_S Addr_TCP_Rec_Stand

//#define S_US_B Sock_UDP_Send_Broad
//#define A_US_B Addr_UDP_Send_Broad
//#define S_UR_B Sock_UDP_Rec_Broad
//#define A_UR_B Addr_UDP_Rec_Broad
//mutex mtx;

int main()
{
    SetConsoleOutputCP(65001);//兼容中文命令行程序

    printf("-----信息收发小程序 V0.1-----\n");

    char ID[16] = { 0 };
    printf("请创造本机用户名（16位）(不能含有空格,中文)\n>>");
    cin >> ID;
    cin.ignore(numeric_limits<streamsize>::max(),'\n');//清理缓冲区
    //-----------------------------------------------------------------------------------------------------------------------------

    //1-信息输入
        //本地监听IP地址
    u_long LocalIP = 0;
    LocalIP = IP_Input_Chack(Platform);//输入+IP地址转换
    printf("32位IP地址：%d\n",LocalIP);
    //1-信息输入结束

    //-----------------------------------------------------------------------------------------------------------------------------

    //2-创建socket前的一些检查工作，包括服务的启动  
    WORD myVersionRequest;
    WSADATA wsaData;  //创建一个WSAData结构体
    myVersionRequest = MAKEWORD(2, 2);  //myVersionRequest指明使用的版本号
                                     //MAKEWORD() 宏函数对版本号进行转换（主版本号，副版本号）
    int err;                         //错误信息变量
    err = WSAStartup(myVersionRequest, &wsaData);
    //err=WSAStartup( MAKEWORD(2, 2), &wsaData); //实际上就是这句
    if (!err)  //成功err为0
    {
        printf("已打开套接字\n");
    }
    else
    {
        printf("嵌套字未打开!\n");
        Socket_Errinof(err);//错误信息显示 
        printf("按任意键退出");
        getch();
        return 0;
    }
    //2-服务启动，检测工作结束

    //-----------------------------------------------------------------------------------------------------------------------------

    //本机信息-将链表第一个给本机信息
    SOCKADDR_IN A_TR_S;                  //参数类，下面会用bind进行绑定
    A_TR_S.sin_family = AF_INET;           //AF_INET表示为IPv4地址  
    A_TR_S.sin_addr.S_un.S_addr = LocalIP; //Socket监听的IP地址，inet_addr函数可以把IP地址字符串转化为网络地址
    //addr.sin_addr.S_un.S_addr=htonl(INADDR_ANY);//ip地址——此处INADDR_ANY表示的时0.0.0.0（不确定的地址）
    A_TR_S.sin_port = htons(Port_Con);     //绑定端口

    //-----------------------------------------------------------------------------------------------------------------------------

    dev_list Local_dev;//创建本机链表元素
    memcpy(Local_dev.ip, inet_ntoa(A_TR_S.sin_addr), strlen( inet_ntoa(A_TR_S.sin_addr)) );
    memcpy(Local_dev.info_Name, ID , strlen( ID ));
    Local_dev.Con[0] = 0;

    //-----------------------------------------------------------------------------------------------------------------------------

    lnkList<dev_list> Client_list(Local_dev);
    Client_list.append(Local_dev);
    //lnkList<dev_list>* Client_list_p;
    int control_r = 0 , send_p = 0;
    int control_s = 0;
    
    //control 0为休眠 1为发送信息 2为UDP广播  其它为终止
    thread send(Thread_send , LocalIP , ID , &control_s , &Client_list , &send_p );
    //control 0为休眠 1为监听 其它为终止
    thread rec(Thread_rec , LocalIP , ID , &control_r , &Client_list );


    int main_control = 1;
    while(main_control){
        cout << "----控制台已激活----" << endl;
        int commend = 0;//功能选择
        int com;//二级功能选择
        printf("1-线程控制\n2-消息发送 3-广播发送\n4-设备列举 5-关闭\n>>");
        scanf("%d",&commend);
        while ((getchar()) != '\n');//清理缓冲区
        //cin >> commend;
        switch(commend){
            case 1 :
                cout << "1-send线程 2-rec线程 :";
                cin >> com;
                if(com == 1){
                    printf("对send线程进行控制---\n0为休眠 1为发送信息 2为UDP广播 其它为终止:");
                    scanf("%d",&control_s);
                }else{
                    printf("对rec线程进行控制\n0为休眠 1为监听 其它为终止:");
                    scanf("%d",&control_r);
                }
                break;

            case 2 :
                Show_List(Client_list);//列出设备
                printf("想要发送给谁（序号）：");
                cin >> com;
                send_p = com;
                control_s = 1;
                while(control_s!=0){
                    Sleep(2*1000);
                }
                break;

            case 3 :
                control_s = 2;
                break;

            case 4 :
                Show_List(Client_list);
                break;

            case 5 :
                main_control = 0;
                break;

            default :
                printf("输入有误：\n");
                commend = 0;
                break;

        }

    }
    
    control_s = -1;
    control_r = -1;
    // Thread_rec(LocalIP , ID , &control , &Client_list );
    // Thread_send(LocalIP , ID , &control , &Cli ent_list , &send_p);
    send.join();
    rec.join();
    WSACleanup();
    printf("按任意键退出");
    getch();
    return 0;
}