#ifndef __LIST_H__
#define __LIST_H__

#include "FreeRTOS.h"

/*
************************************************************************
*                                结构体定义
************************************************************************
*/
/* 节点结构体 */
struct xLIST_ITEM
{
    TickType_t xItemValue;            /* 辅助值，用于帮助节点做顺序排列 */	
    struct xLIST_ITEM *pxNext;        /* 指向链表下一个节点 */
    struct xLIST_ITEM *pxPrevious;    /* 指向链表前一个节点 */
    void *pvOwner;                    /* 指向拥有该节点的内核对象，通常是TCB */
    void *pvContainer;                /* 指向该节点所在的链表 */
};

typedef struct xLIST_ITEM ListItem_t;

/* mini节点结构体定义，作为双向链表的结尾
   因为双向链表是首尾相连的，头即是尾，尾即是头 */
struct xMIN_LIST_ITEM
{
    TickType_t xItemValue;          /* 辅助值，用于帮助节点做顺序排列 */
    struct xLIST_ITEM *pxNext;      /* 指向链表下一个节点 */
    struct xLIST_ITEM *pxPrevious;  /* 指向链表前一个节点 */
};

typedef struct xMIN_LIST_ITEM MiniListItem_t;

/* 链表结构体定义 */
typedef struct xLIST
{
    UBaseType_t uxNumberOfItems;    /* 链表节点计数器 */
    ListItem_t *pxIndex;            /* 链表节点索引指针，是链表的最后一个节点 */
    MiniListItem_t xListEnd;        /* 链表中包含最大xItemValue的节点，位于链表末端，用作标记 marker */ 
} List_t;

/*
************************************************************************
*                                宏定义
************************************************************************
*/
/* 初始化节点的拥有者 */
#define listSET_LIST_ITEM_OWNER(pxListItem, pxOwner)  ((pxListItem)->pvOwner = (void *)pxOwner)
/* 获取节点拥有者 */
#define listGET_LIST_ITEM_OWNER(pxListItem)           ((pxListItem)->pvOwner)
/* 初始化节点排序辅助值 */
#define listSET_LIST_ITEM_VALUE(pxListItem, xValue)   ((pxListItem)->xItemValue = xValue)
/* 获取节点排序辅助值 */
#define listGET_LIST_ITEM_VALUE(pxListItem)           ((pxListItem)->xItemValue)
/* 获取链表根节点的节点计数器的值 */
#define listGET_ITEM_VALUE_OF_HEAD_ENTRY(pxList)      (((pxList)->xListEnd).pxNext->xItemValue)
/* 获取链表的入口节点 */
#define listGET_HEAD_ENTRY(pxList)                    (((pxList)->xListEnd).pxNext)
/* 获取链表的第一个节点 */
#define listGET_NEXT(pxListItem)                      (pxListItem->pxNext)
/* 获取链表的最后一个节点 */
#define listGET_END_MARKER(pxList)                    ((ListItem_t const *)(&((pxList)->xListEnd)))
/* 判断链表是否为空 */
#define listLIST_IS_EMPTY(pxList)                     ((BaseType_t)(((pxList)->uxNumberOfItems) == (UBaseType_t)0)
/* 获取链表节点数 */
#define listCURRENT_LIST_LENGTH(pxList)               ((pxList)->uxNumberOfItems)

/* 获取下一个节点的Onwer，即 TCB */
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
*                                函数声明
************************************************************************
*/
void vListInitialise(List_t * const pxList);
void vListInitialiseItem(ListItem_t * const pxItem);
void vListInsertEnd(List_t * const pxList, ListItem_t * const pxNewtListItem);
void vListInsert(List_t * const pxList, ListItem_t * const pxNewListItem);
UBaseType_t uxListRemove(ListItem_t * const pxItemToRemove);

#endif /* __LIST_H__ */
