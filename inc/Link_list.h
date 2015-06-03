#ifndef _LINK_LIST_H
#define _LINK_LIST_H
#include "stdint.h"
#include "stdio.h"
/*-----------------------------------------------------
lklt_ linklist
-----------------------------------------------------*/

struct  node_struct_t
{ 
    unsigned int fclk;                  //����ִ��Ƶ��   
    unsigned int back;                  //��������
    void *priv;                         // func�����Ļص�����
    void (*func)(void *);
    struct node_struct_t *pre;          //ǰָ��
    struct node_struct_t *next;         //��ָ��
};

#define TRAV_INTERVAL                           5   // 5msִ��һ����������

#define LINKLIST_OK                             0x00
#define LINKLIST_FAILED                         0x01
#define LINKLIST_EMPTY                          0x02	
#define LINKLIST_FULL                           0x04		
#define LINKLIST_NO_EXIST                       0x07	

/*--------------1------------------------------------------------------------------------------------------
function        :   ��ʼ������
�޷���ֵ
---------------------------------------------------------------------------------------------------------*/
void lklt_init(void);

/*--------------2------------------------------------------------------------------------------------------
function        :   ����������ִ�������ﺯ��ָ��ָ�������,��ע��5ms�ж������
�޷���ֵ
---------------------------------------------------------------------------------------------------------*/
void lklt_traversal(void);

/*--------------3------------------------------------------------------------------------------------------
function        :   β������������롣
����ֵ:
LINKLIST_OK     :   �ɹ�
---------------------------------------------------------------------------------------------------------*/
int16_t lklt_insert(struct node_struct_t *ns,void *func,void *priv,unsigned short uwIntvl);

/*--------------4------------------------------------------------------------------------------------------
function        :   ɾ��������ĳ�Ѿ�ע��Ľڵ�
����ֵ:
LINKLIST_OK     :   �ɹ�   
LINKLIST_EMPTY  :   ������
LINKLIST_NO_EXIST:  �����ڴ˽ڵ�
---------------------------------------------------------------------------------------------------------*/
int16_t  lklt_del(struct node_struct_t *ns);



#endif