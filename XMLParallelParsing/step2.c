#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "myXml.h"

lable* creatData(char *tagName, Bcstype bt){        //创建一个栈数据结点并赋值
    lable *node = (lable*)malloc(sizeof(lable));
    strcpy(node->tagname, tagName);
    node->bt = bt;
    return node;
}

stack* creatStack(){                                //创建并初始化栈
    stack *st = (stack*)malloc(sizeof(stack));
    st->top = -1;
    return st;
}

int Push(stack *st, lable *node){                   //入栈操作
    if(st->top == STACKSIZE-1)
    {
        printf("step2栈满，入栈失败！\n");                   //栈已满，不能入栈
        exit(-1);
    }
    st->data[++(st->top)] = node;
    return 1;                                       //入栈成功
}

lable* Pop(stack *st){                              //出栈操作(出栈后记得Free释放掉)
    if(st->top == -1){
        printf("step2栈空，出栈失败！\n");                   //栈已空，不能出栈
        exit(-1);
    }
    return st->data[(st->top)--];
}

char* getname(char* strarr, char* str, char ch){                    //提取表签名
    int i;
    for(i=0;i<NAMELEN && str[i]!=ch && str[i]!=' ';i++) strarr[i] = str[i];
    strarr[i] = '\0';
    return strarr;
}

char* getTagName(char* strarr, char* str, Bcstype bt){              //提取标签名
    switch(bt){
        case Stag_start:
            getname(strarr, str,'>');
            break;
        case Etag_start:
            getname(strarr, str+1,'>');
            break;
/*  //其它标签检验内容的合规性
        case PI_start:

            break;
        case Content:

            break;
        case CDSECT_start:

            break;
        case COMMENT_start:

            break;
*/
        default:
            strarr[0] = '\0';
            break;
    }
    return strarr;
}

void stackInOut(bufferarray *block, char* loc, Bcstype bt, stack* st, bcs* r){
    char *str = (char*)malloc(NAMELEN * sizeof(char));
    strcpy(str, getTagName(str, loc, bt));
    if(st->top>-1 && strcmp(str, st->data[st->top]->tagname) == 0){
        //printf("块号：%d\t出栈前\t栈顶元素：%s\t栈内元素个数%d\n", block->bufnum, st->data[st->top]->tagname, st->top+1);
        lable *delnode = Pop(st);
        free(delnode);
        free(str);
    }else{
        if(str[0] == '\0')  return;
        lable* node = creatData(str, bt);
        Push(st, node);
        //printf("块号：%d\t入栈后\t栈顶元素：%s\t栈内元素个数%d\n", block->bufnum, str,st->top+1);
    }
}

void stackMatching(bufferarray *block){                                     //每个缓存块内的栈匹配
    if(block == NULL)   return;
    bcs *r = block->bcsarr;
    stack *st = creatStack();
    char *name = (char*)malloc(NAMELEN * sizeof(char));
    strcpy(name, getTagName(name, &(block->buf[r->offset]), r->bt));
    lable* node = creatData(name, r->bt);
    Push(st, node);
    r = r->next;
    while(r->next != NULL){                                                 //该块最后一个标签之前的表签名分析
        stackInOut(block, &(block->buf[r->offset]), r->bt, st, r);
        r = r->next;
    }
    if(block->next != NULL){
        int i, j;
        j = block->next->bcsarr->offset;
        if(r->bt == Stag_start){                                                    //最后一个标签为开始标签,直接填充下一块到开始标签的部分
            //printf("该块最后一个字符是：%s\n",&(block->buf[block->buflen-1));
            strncat(block->buf,block->next->buf,j-1);   //j-1是刚好没问题的   j处为"<"
            //printf("该块最后一个字符+追加的不部分%s\n",&(block->buf[block->buflen-j]));
            block->buflen = block->buflen + j-1;
            //printf("该块最后10个字符是%s\n",&(block->buf[block->buflen-10]));
        }else{                                                                      //最后一个不是开始标签，判断
            if(block->buf[block->buflen-1] != '>'){
                //printf("前%s\n",&(block->buf[r->offset]));       //打印最后该块最后一个标签之后的内容
                strncat(block->buf,block->next->buf,j-1);   //j-1是刚好没问题的   j处为"<"
                //printf("后%s\n",&(block->buf[r->offset-1]));       //打印最后该块最后一个标签之后的内容
                block->buflen = block->buflen + j-1;
            }
        }
////////////////////////////////////////填充结束////////////

////////////////////////////////////////下面开始识别标签/////
        i = r->offset;
        j = block->next->bcsarr->offset;

        int k, flag = 0;
        while(i < block->buflen){                           //该块最后一个标签的识别
            if(block->buf[i++] != '<')    continue;
            if(block->buf[i]!='/'&&block->buf[i]!='?'&&block->buf[i]!='!'){             //对<xxxx/>情况的过滤
                flag = 0;
                k = i;
                while(k < block->buflen){
                    if(strCmp(&(block->buf[k++]), 0, 2, "/>")){
                        i = k+1;
                        flag = 1;
                        break;
                    }
                }
                if(flag == 1){
                    i++;
                    continue;
                }
            }
            if(block->buf[i]!='/'&&block->buf[i]!='?'&&block->buf[i]!='!'){             //<xxxx>标签的处理
                stackInOut(block, &(block->buf[i]), Stag_start, st, r);
                continue;
            }else if(block->buf[i]!='/'){                                               //非结束标签则：continue
                i++;
                continue;
            }else{                                                                      //</xxxx>标签的处理
                stackInOut(block, &(block->buf[i]), Etag_start, st, r);
            }
        }//  */
    }

    pthread_mutex_lock(&(block->mutex));
    block->START_STAGE2 = 0;                                                    //访问修改均加锁进行
    block->FINISH_STAGE2 = 1;
    pthread_mutex_unlock(&(block->mutex));

    if(st->top != -1){                                                          //栈非空将数据写入第三阶段的数组中
/*      //该部分为加互斥锁追加到第三阶段的链表部分
        list* tmp = addListNode(block->bufnum, Pop(st));
        pthread_mutex_lock(&mutex);
        point->next = tmp;
        point = point->next;
        pthread_mutex_unlock(&mutex);
        lab *labnode = point->data;
        while(st->top > -1){
            labnode->next = addLabNode(Pop(st));
            labnode = labnode->next;
        }// */

        //printf("块号%d\t栈非空！\t栈内元素个数%d\n", block->bufnum, st->top+1);//st->data[st->top]->tagname);
        lab* tmp = ListNode(block->bufnum, st->data[0]);
        int m = 1;
        while(m <= st->top){
            tmp = addLabNode(tmp,st->data[m++]);
        }
        while(st->top > -1){
            node = Pop(st);
            free(node);
        }
    }else{
        //printf("块号%d\t栈空！\n", block->bufnum);
    }
    free(st);//  */
}

/*  第二阶段线程间处理各缓冲块的无锁方案
void* step2(void* arg){
    bufferarray *block = (bufferarray*)arg;
    int count;
    while(block != NULL){
        printf("块号%d\n", block->bufnum);
        stackMatching(block);
        for(count=0;count<THREADNUM&&block!=NULL;count++)
            block = block->next;
    }
    return (void*)NULL;
}//    */

//  第二阶段线程间处理各缓冲块的有锁方案
void* step2(void* arg){
    bufferarray *block = (bufferarray*)arg;
    int flag1, flag2;
    while(block != NULL){
        flag1 = 0;

        pthread_mutex_lock(&(block->mutex));
        if(block->START_STAGE2 == 0 && block->FINISH_STAGE2 == 0){                      //访问修改均加锁进行
            block->START_STAGE2 = 1;
            flag1 = 1;
        }
        pthread_mutex_unlock(&(block->mutex));

        if(flag1){
            stackMatching(block);
        }

        flag2 = 1;
        while(flag2 && block != NULL){

            pthread_mutex_lock(&(block->mutex));
            if(block->START_STAGE2 == 0 && block->FINISH_STAGE2 == 0)   flag2 = 0;      //访问修改均加锁进行
            pthread_mutex_unlock(&(block->mutex));

            block = block->next;
        }

    }
    return (void*)NULL;
}//  */
