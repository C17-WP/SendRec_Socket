#pragma once
#ifndef _FC_h_
#define _FC_h_
#include <winsock2.h>  
#include <windows.h>

using namespace std;


//用于传输和记录的结构体
struct dev_list {
    char ip[16] = { 0 };   //IP地址的字符类型
    char info_Name[16] = { 0 }; //发送者ID
    int Con[2] = {0};        //用于发送的请求判断-非0表示为搜寻请求，为0表示为回应请求，在链表中应始终为0
};

//---------------模板开始------------------
template <class T>
class Link
{
public:
    T data;  //用于保存结点元素的内容
    int dev_num = 0;
    Link<T>* next;  //指向后继结点的指针
    Link(const T info, Link<T>* nextValue = NULL) {
        data = info;
        next = nextValue;
    }
    Link() :next(NULL) {}
};

//位置i从1开始
template <class T>
class lnkList :public Link<T> {
private:
    Link<T>* head, * tail;      //单链表的头尾指针
    Link<T>* setPos(const int i);    //返回第i个元素的指针值
public:
    //lnkList();   //构造函数
    lnkList(T Value);   //构造函数-直接构建新结点
    ~lnkList();     //析构函数
    bool isEmpty();     //判断链表是否为空
    void clear();       //置空线性表
    int length();    //返回此链表当前实际长度
    bool append(const T value);     //在表尾添加一个元素value,表的长度增1
    bool insert(const int i, const T value);    //在位置i上插入一个元素value，表的长度增1，0为在头结点前插入
    bool delet(const int i);        //删除位置i上的元素，表的长度减1
    bool getValue(const int i, T& value);   //把位置i元素值返回到变量value
    bool getPos(int& i, const T value, char output_switch);     //查找值为value的元素并返回其位置
};

// template <class T>
// lnkList<T>::lnkList() {
//     T Temp;
//     Link<T>* temp = new Link<T>(Temp, NULL);
//     head = temp;
//     tail = NULL;
// }

template <class T>
lnkList<T>::lnkList(T Value) {
    Link<T>* temp = new Link<T>(Value, NULL);
    head = temp;
    tail = NULL;
}

template <class T>
lnkList<T>::~lnkList() {
    if (head->next == NULL)//判断链表是否为空
    {
        delete head;
        return;
    }

    Link<T>* cur = head;//开始从头结点开始删除
    while (cur != NULL) {
        Link<T>* del = cur;
        cur = cur->next;
        delete del;
    }
    delete cur;
    head = NULL;
    tail = NULL;
}

template <class T>
Link<T>* lnkList<T>::setPos(const int i) {
    int count = 0;
    Link<T>* p = head;  //循链定位，若i为0则定位到第一个结点
    while (p != NULL && count < i) {
        p = p->next;
        count++;
    }
    return p;   //指向结点i
}

template <class T>
bool lnkList<T>::insert(const int i, const T value) {
    Link<T>* p, * q;
    // if (i == 0){    //想要在head前插入，i可等于0-假设head结点前还有一个结点
    //     q = new Link<T>(value, head->next);
    //     head->next = q;
    //     return true;
    // }
    if ((p = setPos(i - 1)) == NULL)   //位置为前驱位置X
    {
        cout << "Insertion point is illegal" << endl;
        return false;
    }
    q = new Link<T>(value, p->next);
    p->next = q;
    if (p == tail)  //插入点在链尾，插入结点成为新的链尾
    {
        tail = q;
    }
    head->dev_num++;
    return true;
}

template<class T>
bool lnkList<T>::delet(const int i) {//删除按照头结点为1算
    Link<T>* p, * del;
    // if (i==0){  //删除头结点
    //     del=head;
    //     head=head->next;
    //     delete del;
    //     return true;
    // }

    p = setPos(i - 1);//检查要删除的结点
    if (p == NULL || p == tail) {  //删除的结点不存在
        //cout << "deletion is illegal" << endl;
        printf("deletion is illegal\n");
        return false;
    }

    del = p->next;//del是真正待删除结点
    if (del == tail) {  //待删结点为尾结点，则修改尾指针
        tail = p;
        p->next = NULL;
    }
    else
        p->next = del->next;   //删除结点q并修改链指针
    delete del;
    head->dev_num--;
    return true;
}

template <class T>
bool lnkList<T>::isEmpty() {
    if (head->next == NULL) {
        return true;
    }
    else {
        return false;
    }
}

template <class T>
void lnkList<T>::clear() {
    if (head->next == NULL)
    {
        return;
    }
    Link<T>* cur = head->next;
    while (cur != NULL) {
        Link<T>* del = cur;
        cur = cur->next;
        delete del;
    }
    delete cur;
    head->next = NULL;
    tail = NULL;
}

template <class T>
int lnkList<T>::length() {
    int count=0;
    Link<T>* cur=head->next;
    while(cur) {
        count++;
        cur = cur->next;
    }
    return count;
    //return head->dev_num;
}

template <class T>
bool lnkList<T>::append(const T value) {
    Link<T>* newLink = new Link<T>(value);
    if (head->next == NULL) {
        head->next = newLink;
        tail = head->next;
    }
    else {
        tail->next = newLink;
        tail = newLink;
    }
    head->dev_num++;
    return true;
}

template <class T>
bool lnkList<T>::getPos(int& i, const T value, char output_switch) {
    Link<T>* cur = head->next;
    int count = 0;
    while (cur) {
        if (cur->data == value)
        {
            i = count;
            return true;
        }
        cur = cur->next;
        count++;
    }
    if (output_switch)
        cout << "can not find element: " << value << endl;
    return false;
}

template <class T>
bool lnkList<T>::getValue(const int i, T& value) {
    int count = 0;
    Link<T>* cur = head->next;  //循链定位，若i为0则定位到第一个结点
    while (cur != NULL && count < i) {
        cur = cur->next;
        count++;
    }
    if (cur == NULL) {
        return false;
    }
    else {
        value = cur->data;
        return true;
    }
}
//---------------模板结束------------------

u_long IP_Input_Chack(int Platform /*= 0*/);

bool Thread_rec(u_long LocalIP , char ID[16] , int *control , lnkList<dev_list> *Client_list);

bool Thread_send(u_long LocalIP , char ID[16] , int *control , lnkList<dev_list> *Client_list , int *send_p);

void Show_List(lnkList<dev_list>& palist);

void Socket_Errinof(int Errcode);


#endif