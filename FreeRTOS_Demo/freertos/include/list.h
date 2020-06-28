#ifndef __LIST_H__
#define __LIST_H__

#include "FreeRTOS.h"

/*
************************************************************************
*                                �ṹ�嶨��
************************************************************************
*/
/* �ڵ�ṹ�� */
struct xLIST_ITEM
{
    TickType_t xItemValue;            /* ����ֵ�����ڰ����ڵ���˳������ */	
    struct xLIST_ITEM *pxNext;        /* ָ��������һ���ڵ� */
    struct xLIST_ITEM *pxPrevious;    /* ָ������ǰһ���ڵ� */
    void *pvOwner;                    /* ָ��ӵ�иýڵ���ں˶���ͨ����TCB */
    void *pvContainer;                /* ָ��ýڵ����ڵ����� */
};

typedef struct xLIST_ITEM ListItem_t;

/* mini�ڵ�ṹ�嶨�壬��Ϊ˫������Ľ�β
   ��Ϊ˫����������β�����ģ�ͷ����β��β����ͷ */
struct xMIN_LIST_ITEM
{
    TickType_t xItemValue;          /* ����ֵ�����ڰ����ڵ���˳������ */
    struct xLIST_ITEM *pxNext;      /* ָ��������һ���ڵ� */
    struct xLIST_ITEM *pxPrevious;  /* ָ������ǰһ���ڵ� */
};

typedef struct xMIN_LIST_ITEM MiniListItem_t;

/* ����ṹ�嶨�� */
typedef struct xLIST
{
    UBaseType_t uxNumberOfItems;    /* ����ڵ������ */
    ListItem_t *pxIndex;            /* ����ڵ�����ָ�룬����������һ���ڵ� */
    MiniListItem_t xListEnd;        /* �����а������xItemValue�Ľڵ㣬λ������ĩ�ˣ�������� marker */ 
} List_t;

/*
************************************************************************
*                                �궨��
************************************************************************
*/
/* ��ʼ���ڵ��ӵ���� */
#define listSET_LIST_ITEM_OWNER(pxListItem, pxOwner)  ((pxListItem)->pvOwner = (void *)pxOwner)
/* ��ȡ�ڵ�ӵ���� */
#define listGET_LIST_ITEM_OWNER(pxListItem)           ((pxListItem)->pvOwner)
/* ��ʼ���ڵ�������ֵ */
#define listSET_LIST_ITEM_VALUE(pxListItem, xValue)   ((pxListItem)->xItemValue = xValue)
/* ��ȡ�ڵ�������ֵ */
#define listGET_LIST_ITEM_VALUE(pxListItem)           ((pxListItem)->xItemValue)
/* ��ȡ������ڵ�Ľڵ��������ֵ */
#define listGET_ITEM_VALUE_OF_HEAD_ENTRY(pxList)      (((pxList)->xListEnd).pxNext->xItemValue)
/* ��ȡ�������ڽڵ� */
#define listGET_HEAD_ENTRY(pxList)                    (((pxList)->xListEnd).pxNext)
/* ��ȡ����ĵ�һ���ڵ� */
#define listGET_NEXT(pxListItem)                      (pxListItem->pxNext)
/* ��ȡ��������һ���ڵ� */
#define listGET_END_MARKER(pxList)                    ((ListItem_t const *)(&((pxList)->xListEnd)))
/* �ж������Ƿ�Ϊ�� */
#define listLIST_IS_EMPTY(pxList)                     ((BaseType_t)(((pxList)->uxNumberOfItems) == (UBaseType_t)0)
/* ��ȡ����ڵ��� */
#define listCURRENT_LIST_LENGTH(pxList)               ((pxList)->uxNumberOfItems)

/* ��ȡ��һ���ڵ��Onwer���� TCB */
#define listGET_OWNER_OF_NEXT_ENTRY(pxTCB, pxList)                           \
{                                                                            \
    List_t * const pxConstList = (pxList);                                   \
	/* Increment the index to the next item and return the item, */          \
    /* ensuring we don't return the marker used at the end of the list */    \							                                         \
    pxConstList->pxIndex = pxConstList->pxIndex->pxNext;                     \
    if((void *)(pxConstList->pxIndex) == (void *)( &(pConstList->xListEnd))) \
    {                                                                        \
        pxConstList->pxIndex = pxConstList->pxIndex->pxNext;                 \
    }                                                                        \
    pxTCB = pxConstList->pxIndex->pvOwner;                                   \
}

#define listGET_OWNER_OF_HEAD_ENTRY(pxList)   ((&((pxList)->xListEnd))->pxNext->pvOwner)

/*
************************************************************************
*                                ��������
************************************************************************
*/
void vListInitialise(List_t * const pxList);
void vListInitialiseItem(ListItem_t * const pxItem);
void vListInsertEnd(List_t * const pxList, ListItem_t * const pxNewtListItem);
void vListInsert(List_t * const pxList, ListItem_t * const pxNewListItem);
UBaseType_t uxListRemove(ListItem_t * const pxItemToRemove);

#endif /* __LIST_H__ */
