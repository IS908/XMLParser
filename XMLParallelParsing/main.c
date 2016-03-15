#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "myXml.h"

int main(int argc, char ** argv)
{
    pthread_t pt[THREADNUM];
    int result[THREADNUM];
    int readCount,blocknum=0;
    bufferarray *bufhead=NULL,*buf,*tmp;
    FILE *fp;
    fp = fopen("test.xml","r");

    //读取申请第一个缓冲块
    tmp = mallocBuffer();

    //循环读入数据到缓冲区并进行第一阶段的处理
    while((readCount=fread(tmp->buf,sizeof(char),BUFLEN,fp))>0){
        if(blocknum > 0)
        {       //中间的缓存块链表的链接
            buf->next = analizeBlock(tmp, blocknum++, readCount);
            buf = buf->next;
        }else{  //第一个缓冲区作为缓冲区链表头部的处理
            bufhead = analizeBlock(tmp, blocknum++, readCount);
            buf = bufhead;
        }
        tmp = mallocBuffer();
    }
    fclose(fp);
    printf("\n\t第一阶段预处理结束! 划分文件块%d个！\n\n", blocknum);

/*    //输出测试第一阶段的结果
    bcs *r;
    buf = bufhead;
    while(buf != NULL){
        r = buf->bcsarr;
        //printf("开始第%d块\n",buf->bufnum);
        while(r != NULL){
            printf("blocknumber [%d], offset [%d], Tagtype [%s]\n", buf->bufnum, r->offset, printEnum(r->bt));
            r = r->next;
        }
        buf = buf->next;
    }
//  */

/*  创建第三阶段的栈，以备第二阶段非空栈写入第三阶段的栈中     */
    arr* array = creatListNode(blocknum);
    arrhead = array;
/*  创建第二阶段处理的线程，进行第二阶段的处理  */

/*  单线程进行第二阶段处理
    buf = bufhead;
    result[0] = pthread_create(&(pt[0]), NULL, step2, (void*)buf);
    pthread_join(pt[0], NULL);
//    */

    //多线程进行第二阶段处理
    int i;
    buf = bufhead;
    for(i=0;i<THREADNUM;i++){                                                               //循环创建线程开始第二阶段的处理
        result[i] = pthread_create(&(pt[i]), NULL, step2, (void*)buf);
        buf = buf->next;
        if(result[i] != 0){
            printf("create pthread error!\n");
            exit(-1);
        }
    }
    for(i=0;i<THREADNUM;i++){                                                               //等待线程结束
        pthread_join(pt[i], NULL);
    }
    printf("\t第二阶段多线程处理完毕！ 并行线程数为%d个!\n\n", THREADNUM);
//  */

/*  进行第三阶段的处理   */
    if(step3(array, blocknum))
        printf("\t该XML文件符合规范！\n");
    else
        printf("\t该XML文件符合规范！\n");
//  */
    return 0;
}

//打印相应标签类型
char* printEnum(Bcstype type){
    switch(type){
        case Stag_start:
            return "Stag";
        case Etag_start:
            return "Etag";
        case PI_start:
            return "PI";
        case Content:
            return "content";
        case CDSECT_start:
            return "CDATA";
        case COMMENT_start:
            return "comment";
        default:
            return "Tag";
    }
}
