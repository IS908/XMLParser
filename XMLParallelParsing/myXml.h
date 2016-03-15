#include <pthread.h>

#ifndef MYXML_H_INCLUDED
#define MYXML_H_INCLUDED

#define BUFLEN 10240                    //缓冲区字符区大小10K
#define APPEND 512                      //填充字符区大小
#define NAMELEN 256                     //标签名字最大长度
#define STACKSIZE 16                    //栈的深度
#define THREADNUM 10                    //第二阶段解析并行的线程数


typedef enum Bcstype_t{                 //枚举标签类型
    Stag_start,
    Etag_start,
    PI_start,
    Content,
    CDSECT_start,
    COMMENT_start
} Bcstype;

typedef struct bcs_t{                   //标签节点的数据结构
    int offset;                         //相对于该数据块的偏移
    int taglen;                         //该标签的长度（计算结束位置）
    Bcstype bt;                         //该标签的类型
    struct bcs_t *next;
} bcs;

typedef struct bufferarry_t{            //缓冲区块的数据结构
    char buf[BUFLEN + APPEND];          //存放读入的数据
    bcs *bcsarr;                        //识别标签后存放结果的链表
    int bufnum;                         //缓冲块块号
    int buflen;                         //缓冲区所存数据长度

    pthread_mutex_t mutex;              //互斥锁

    int START_STAGE1;                   //第1阶段开始处理该块时置1
    int FINISH_STAGE1;                  //第1阶段处理完置1

    int START_STAGE2;                   //第2阶段开始处理该块时置1
    int FINISH_STAGE2;                  //第2阶段处理完置1

    int START_STAGE3;                   //第3阶段开始处理该块时置1
    int FINISH_STAGE3;                  //第3阶段处理完置1

    struct bufferarry_t *next;          //下一个缓冲区块指针
} bufferarray;

////////////////////////////////////////第二阶段用到的结构体
typedef struct lable_t{                 //栈内数据的数据结构
    char tagname[NAMELEN];
    Bcstype bt;
} lable;

typedef struct stack_t{                 //栈的数据结构
    int top;                            //栈的指针
    lable *data[STACKSIZE];             //栈的数据(标签名字、标签类型)
} stack;

////////////////////////////////////////第三阶段用到的结构体
typedef struct lab_t{                   //链表节点内数据的数据结构
    char tagname[NAMELEN];
    Bcstype bt;
    struct lab_t *next;
} lab;

typedef struct restarr_t{               //数组内数据的数据结构
    int blocknum;                       //该标签所在的块号
    lab *data;                          //该标签类型
} arr;
arr* arrhead;                           //全局变量

typedef struct stack_lab{
    int top;                            //栈的指针
    lab *data[STACKSIZE * 30];          //栈的数据(标签名字、标签类型)
} stacklab;

/////////////////////////////////////////各阶段的函数声明

//打印相应标签类型
char* printEnum(Bcstype type);                                                      //main.c


//申请缓冲区并初始化
bufferarray* mallocBuffer();                                                        //step1.c
//字符串比较
int strCmp(char *str,int start,int len,char str2[]);                                //step1.c
//创建标记信息链表
bcs* bcsnode(Bcstype type,int offset);                                              //step1.c
//分析数据块，找到个标签的位置并存入链表中
bufferarray* analizeBlock(bufferarray *block, int blocknum, int buflen);            //step1.c


//创建一个栈数据结点并赋值
lable* creatData(char *tagName, Bcstype bt);                        //step2.c
//创建并初始化栈
stack* creatStack();                                                //step2.c
//入栈操作
int Push(stack *st, lable *node);                                   //step2.c
//出栈操作(出栈后记得Free释放掉)
lable* Pop(stack *st);                                              //step2.c
//提取名字
char* getname(char* strarr, char* str, char ch);                    //step2.c
//提取标签名
char* getTagName(char* strarr, char* str, Bcstype bt);              //step2.c
//与栈顶对比，判断进栈或出栈
void stackInOut(bufferarray *block, char* loc, Bcstype bt, stack* st, bcs* r);          //step2.c
//每个缓存块内的栈匹配
void stackMatching(bufferarray *block);                             //step2.c
//线程调用函数
void* step2(void* arg);                                             //step2.c


//创建链表头节点
arr* creatListNode(int blocknum);                                   //step3.c
//创建非空栈转移的链表节点
lab* ListNode(int blocknum, lable* lnode);                          //step3.c
//创建栈转移数据节点数据
lab* addLabNode(lab* last, lable* l);                               //step3.c
//创建第三阶段用到的栈
stacklab* creatLabStack();                                          //step3.c
//进栈操作
void Pushlab(stacklab *st, lab *node);                              //step3.c
//出栈操作
lab* Poplab(stacklab *st);                                          //step3.c
//第三阶段处理函数
int step3(arr* array, int countblock);                              //step3.c
//比较两个lab中的tagname是否相同
int isSameTag(lab* lab1, lab* lab2);                                //step3.c

#endif // MYXML_H_INCLUDED
