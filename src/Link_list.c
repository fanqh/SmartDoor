#include"Link_list.h"


struct node_struct_t head;

void lklt_init(void)
{

    head.fclk   =   0;
    head.back   =   0;
    head.func   =   NULL;
    head.pre    =   NULL;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       
    head.next   =   NULL;
}

void lklt_traversal(void)
{
    struct node_struct_t *pnode;
    pnode=head.next;
    while(pnode)
    {
        if(pnode->fclk==0)
        {
            pnode->fclk =pnode->back;   //��ԭ��ȥ
            if(pnode->func)
                pnode->func(pnode->priv);
        }
        else
            pnode->fclk--;
        pnode=pnode->next;
    }
}

int16_t lklt_insert(struct node_struct_t *ns,void *func,void *priv,unsigned short uwIntvl)
{
    struct node_struct_t *pNext;

    ns->func=(void (*)(void *))func;
    ns->priv=priv;
    ns->back=uwIntvl/TRAV_INTERVAL;
    ns->fclk=0;             // ��һ�α�����ʱ������ִ��
    
    pNext   =  head.next;
    if(head.next == NULL)   // ���β���
    {
        head.next   =   ns;
        ns->pre  =   &head;
        ns->next =   NULL;
        return LINKLIST_OK;
    }
    while(pNext->next)        // ����
    {
        pNext   =   pNext->next;
    }
    pNext->next =   ns;
    ns->pre     =   pNext;
    ns->next    =   NULL;
    return LINKLIST_OK;
}

int16_t lklt_del(struct node_struct_t *ns)
{
    struct node_struct_t *pNext;
    int16_t   result;

    pNext   =  head.next;
    if(head.next == NULL)   //ֻ��ͷ�ڵ�
    {
        result=LINKLIST_EMPTY;
        goto LKLT_RETURN; 
    }
    while(pNext)        //����
    {
        if(pNext->func != ns->func)
            pNext=pNext->next;
        else 
        {
            pNext->pre->next   =   pNext->next;
            if(pNext->next)
            { 
                pNext->next->pre    =   pNext->pre;               
            }
            result=LINKLIST_OK;
            goto LKLT_RETURN; 
        }
    }
    if(pNext->func != ns->func)
    {
        result=LINKLIST_NO_EXIST;
        goto LKLT_RETURN; 
    }
  
   
    pNext->pre->next  =   NULL;
    result=LINKLIST_OK;
LKLT_RETURN:
    return result;
}

uint16_t  lklt_getlen(void)
{
    struct node_struct_t *pnode;
    uint16_t  i;
    i=0;
    pnode=head.next;
    while(pnode)
    {
        pnode=pnode->next;
        i++;
    }
    return i;
}


