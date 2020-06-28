#include "FreeRTOS.h"
#include <stdlib.h>
#include "list.h"

/* 链表初始化 */
void vListInitialise(List_t * const pxList)
{
    /* 将链表索引指针指向最后一个节点 */
    pxList->pxIndex = (ListItem_t *)(&(pxList->xListEnd));
    /*将链表最后一个节点的辅助排序值设置为最大，确保该节点是链表中的最后一个 */
    pxList->xListEnd.xItemValue = portMAX_DELAY;
    /*将最后一个节点的pxNext、pxPrevious均指向节点本身，表示链表为空 */
    pxList->xListEnd.pxNext = (ListItem_t *)(&(pxList->xListEnd));
    pxList->xListEnd.pxPrevious = (ListItem_t *)(&(pxList->xListEnd));
    /* 初始化链表节点计数器的值为0，表示链表为空 */
    pxList->uxNumberOfItems = (UBaseType_t)0U;
}

/* 节点初始化 */
void vListInitialiseItem(ListItem_t * const pxItem)
{
    /* 初始化该节点所在的链表为空，表示该节点没有插入任何链表 */
    pxItem->pvContainer = NULL;
}

/* 节点插入到链表尾部 */
void vListInsertEnd(List_t * const pxList, ListItem_t * const pxNewtListItem)
{
    ListItem_t * const pxIndex = pxList->pxIndex;
    pxNewtListItem->pxNext = pxIndex;
    pxNewtListItem->pxPrevious = pxIndex->pxPrevious;
    pxIndex->pxPrevious->pxNext = pxNewtListItem;
    pxIndex->pxPrevious = pxNewtListItem;
    //pxIndex->pxNext 指向不变化
    
    /* 记住该节点所在的链表 */
    pxNewtListItem->pvContainer = (void *)pxList;
    
    /* 链表节点计数器++ */
    (pxList->uxNumberOfItems)++;
}

/* 将节点按照 xItemValue 升序插入到链表 */
void vListInsert(List_t * const pxList, ListItem_t * const pxNewListItem)
{
    /* 
     * 记录辅助值比 pxNewListItem-> xItemValue 小的的列表项 ItemX ，
     * ItemX再往后的列表项 ItemX++ 的辅助值会大于或等于 pxNewListItem-> xItemValue
     * 插入的位置即 ItemX 之后
    */
    ListItem_t *pxIterator = NULL;
    
    /* 获取节点排序辅助值 */
    const TickType_t xValueOfInsertion = pxNewListItem->xItemValue;
    
    /* 节点要插入到链表的尾部 */
    if(xValueOfInsertion == portMAX_DELAY)
    {
        pxIterator = pxList->xListEnd.pxPrevious;
    }
    else
    {
        /* 从前往后查找 */
        for(pxIterator = (ListItem_t *)(&(pxList->xListEnd));
            pxIterator->pxNext->xItemValue <= xValueOfInsertion;
            pxIterator = pxIterator->pxNext)
        {
            /*不做其他事情，只是为了找到要插入的位置 */
        }
    }
    
    pxNewListItem->pxNext = pxIterator->pxNext;
    pxNewListItem->pxNext->pxPrevious = pxNewListItem; //或者 pxIterator->pxNext->pxPrevious = pxNewListItem
    pxNewListItem->pxPrevious = pxIterator;
    pxIterator->pxNext = pxNewListItem;
    
    /* 记住该节点所在的链表 */
    pxNewListItem->pvContainer = (void *)pxList;
    
    /* 链表节点计数器++ */
    (pxList->uxNumberOfItems)++;
}

/* 将节点从链表中删除 */
UBaseType_t uxListRemove(ListItem_t * const pxItemToRemove)
{
    /* 获取节点所在列表 */
    List_t * const pxList = pxItemToRemove->pvContainer;
    pxItemToRemove->pxPrevious->pxNext = pxItemToRemove->pxNext;
    pxItemToRemove->pxNext->pxPrevious = pxItemToRemove->pxPrevious;
    
    /* 初始化节点所在的链表为空，表示节点还没有插入任何链表 */
    pxItemToRemove->pvContainer = NULL;
    
    /* 链表节点计数器--*/
    (pxList->uxNumberOfItems)--;
    
    /* 返回链表中剩余节点的个数 */
    return pxList->uxNumberOfItems;
}
