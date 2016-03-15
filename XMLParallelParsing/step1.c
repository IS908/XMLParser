#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "myXml.h"

bufferarray* mallocBuffer(){                                        //申请缓冲区并初始化
    bufferarray *p = (bufferarray*)malloc(sizeof(bufferarray));
    if(p == NULL){
        printf("申请内存失败！");
        exit(-1);
    }
    p->bcsarr = NULL;
    p->bufnum = -1;
    p->buflen = -1;
    p->START_STAGE1 = 0;
    p->START_STAGE2 = 0;
    p->START_STAGE3 = 0;
    p->FINISH_STAGE1 = 0;
    p->FINISH_STAGE2 = 0;
    p->FINISH_STAGE3 = 0;
    pthread_mutex_init(&(p->mutex), NULL);                          //初始化互斥锁
    p->next = NULL;
    return p;
}


int strCmp(char *str,int start,int len,char str2[]){                //字符串比较
	int i=0;
	for(i=0;i<len;++i)
		if(str[start + i]!=str2[i])     return 0;
	return 1;
}


bcs* bcsnode(Bcstype type,int offset){                              //创建标记信息链表
    bcs *bcslist = (bcs*)malloc(sizeof(bcs));
    if(bcslist == NULL){
        printf("申请内存失败！");
        exit(-1);
    }
    bcslist->offset = offset;
    bcslist->bt = type;
    bcslist->next = NULL;
    return bcslist;
}


bufferarray* analizeBlock(bufferarray *block, int blocknum, int buflen){         //分析数据块，找到个标签的位置并存入链表中
    bcs *p = NULL;
    int i = 0,j = 0;
    block->START_STAGE1 = 1;                                        //开始第1阶段的处理
    block->bufnum = blocknum;
    block->buflen = buflen;
    while(i<BUFLEN){
        if(block->buf[i++] != '<') continue;                                    //当前字符是'<'时判断下一个字符的类型
        if(block->buf[i]!='/' && block->buf[i]!='?' && block->buf[i]!='!'){     //该块为<xxxx/>或</xxxx>
            if(j != 0)                                                          //第一个以后的开始标签
            {
                j = i;
                while(i < BUFLEN){
                    if(block->buf[i] == '/' && block->buf[i+1] == '>'){         //<xxxx/>
                        break;
                    }else if(block->buf[i] == '>'){                             //</xxxx>
                        p->next = bcsnode(Stag_start,j);
                        p = p->next;
                        break;
                    }
                    i++;
                }
            }else{                                                              //从第一个开始标签开始记录
                j = i;
                while(i < BUFLEN){
                    if(block->buf[i] == '/' && block->buf[i+1] == '>'){         //<xxxx/>
                        j = 0;
                        break;
                    }else if(block->buf[i] == '>'){                             //</xxxx>
                        block->bcsarr = bcsnode(Stag_start,j);
                        p = block->bcsarr;
                        break;
                    }
                    i++;
                }
            }
        }else if(j == 0){                                                       //还没到该块的开始标签
            continue;
        }else if(block->buf[i] == '/'){                                         //</xxxx>
            p->next = bcsnode(Etag_start,i);
            p = p->next;
            while(i < BUFLEN){
                if(block->buf[i++] == '>')              break;
            }
        }else if(block->buf[i] == '?'){                                         //<?xxxx?>
            p->next = bcsnode(PI_start,i);
            p = p->next;
            while(i < BUFLEN){
                if(strCmp(block->buf,i++,2,"?>"))       break;
            }
        }else if(strCmp(block->buf,i,3,"!--")){                                 //<!--xxxx-->
            p->next = bcsnode(COMMENT_start,i);
            p = p->next;
            while(i < BUFLEN){
                if(strCmp(block->buf,i++,3,"-->"))      break;
            }
        }else if(strCmp(block->buf,i,8,"![CDATA[")){                            //<![CDATA［xxxx]]>
            p->next = bcsnode(CDSECT_start,i);
            p = p->next;
            while(i < BUFLEN){
                if(strCmp(block->buf,i++,3,"]]>"))      break;
            }
        }
        i++;
    }
    block->FINISH_STAGE1 = 1;                       //第1阶段处理结束
    block->START_STAGE1 = 0;                        //允许进行下一阶段的处理
    return block;
}

