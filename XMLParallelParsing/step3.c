#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "myXml.h"

arr* creatListNode(int number){                                                              //创建链表头节点
    arr* array = (arr*)malloc(sizeof(arr)*number);
    int i;
    for(i=0;i<number;i++){
        array[i].blocknum = -1;
    }
    return array;
}

lab* ListNode(int number, lable* lnode){                                            //创建非空栈转移的链表节点
    lab *node = (lab*)malloc(sizeof(lab));
    strcpy(node->tagname, lnode->tagname);
    node->bt = lnode->bt;
    node->next = NULL;
    arrhead[number].blocknum = number;
    arrhead[number].data = node;
    return node;
}

lab* addLabNode(lab* last, lable* l){                                                //创建栈转移数据节点数据
    lab* node = (lab*)malloc(sizeof(lab));
    strcpy(node->tagname, l->tagname);
    node->bt = l->bt;
    node->next = NULL;
    last->next = node;
    return node;
}

stacklab* creatLabStack(){
    stacklab *st = (stacklab*)malloc(sizeof(stacklab));
    st->top = -1;
    return st;
}

void Pushlab(stacklab *st, lab *node){                      //入栈操作
    if(st->top == (STACKSIZE * 1000) - 1)
    {
        printf("step3栈满，入栈失败！\n");                           //栈满，不能入栈
        exit(-1);
    }
    st->data[++(st->top)] = node;
}

lab* Poplab(stacklab *st){                                  //出栈操作(出栈后记得Free释放掉)
    if(st->top == -1){
        printf("step3栈空，出栈失败！\n");                           //栈空，不能出栈
        exit(-1);
    }
    return st->data[(st->top)--];
}

int step3(arr* listarr, int countblock){
    stacklab *st = creatLabStack();                         //step2.c函数，栈的深度可能不够
    //printf("开始第三阶段的剩余部分的匹配！\n");
    int i = 0, flag = 0;
    //flag%2为0，从链表头部正着进栈；flag%2为1，从链表尾部倒着进栈（保证最近匹配）
    while(i < countblock){                                  //找到第一个待处理的块号
        if(listarr[i].blocknum != -1)   break;
        i++;
    }
    lab *tmp = listarr[i].data;
    Pushlab(st, tmp);
    if(tmp->next != NULL){
        tmp = tmp->next;
        Pushlab(st, tmp);
    }
    int num = 0;
    while(i<countblock){
        while(i<countblock){                                //该块号之后找最近的一个待处理的块号
            if(listarr[i].blocknum != -1)   break;
            i++;
        }
        tmp = listarr[i].data;
        while(tmp != NULL){
            flag = 0;
            if(strcmp(tmp->tagname,st->data[st->top]->tagname) != 0) flag = 1;
            while(flag ==1 && tmp != NULL){
                Pushlab(st, tmp);
                tmp = tmp->next;
            }
            if(tmp != NULL){
                lab *del = Poplab(st);
                free(del);
                tmp = tmp->next;
            }
        }
        i++;
        num++;
    }
    printf("\t第三阶段处理了第二阶段的待处理块有%d个!\n\n", num);
    if(st->top == -1)
        return 1;
    else
        return 0;
}

