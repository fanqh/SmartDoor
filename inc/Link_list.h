#ifndef _LINK_LIST_H
#define _LINK_LIST_H
#include "stdint.h"
#include "stdio.h"
/*-----------------------------------------------------
lklt_ linklist
-----------------------------------------------------*/

struct  node_struct_t
{ 
    unsigned int fclk;                  //函数执行频率   
    unsigned int back;                  //函数备份
    void *priv;                         // func函数的回调参数
    void (*func)(void *);
    struct node_struct_t *pre;          //前指针
    struct node_struct_t *next;         //后指针
};

#define TRAV_INTERVAL                           5   // 5ms执行一次链表遍历

#define LINKLIST_OK                             0x00
#define LINKLIST_FAILED                         0x01
#define LINKLIST_EMPTY                          0x02	
#define LINKLIST_FULL                           0x04		
#define LINKLIST_NO_EXIST                       0x07	

/*--------------1------------------------------------------------------------------------------------------
function        :   初始化链表
无返回值
---------------------------------------------------------------------------------------------------------*/
void lklt_init(void);

/*--------------2------------------------------------------------------------------------------------------
function        :   遍历链表，执行链表里函数指针指向的内容,备注在5ms中断里调用
无返回值
---------------------------------------------------------------------------------------------------------*/
void lklt_traversal(void);

/*--------------3------------------------------------------------------------------------------------------
function        :   尾差法，向链表插入。
返回值:
LINKLIST_OK     :   成功
---------------------------------------------------------------------------------------------------------*/
int16_t lklt_insert(struct node_struct_t *ns,void *func,void *priv,unsigned short uwIntvl);

/*--------------4------------------------------------------------------------------------------------------
function        :   删除链表中某已经注册的节点
返回值:
LINKLIST_OK     :   成功   
LINKLIST_EMPTY  :   空链表
LINKLIST_NO_EXIST:  不存在此节点
---------------------------------------------------------------------------------------------------------*/
int16_t  lklt_del(struct node_struct_t *ns);



#endif
