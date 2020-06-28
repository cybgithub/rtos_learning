#include "FreeRTOS.h"
#include <stdlib.h>
#include "list.h"

/* �����ʼ�� */
void vListInitialise(List_t * const pxList)
{
    /* ����������ָ��ָ�����һ���ڵ� */
    pxList->pxIndex = (ListItem_t *)(&(pxList->xListEnd));
    /*���������һ���ڵ�ĸ�������ֵ����Ϊ���ȷ���ýڵ��������е����һ�� */
    pxList->xListEnd.xItemValue = portMAX_DELAY;
    /*�����һ���ڵ��pxNext��pxPrevious��ָ��ڵ㱾����ʾ����Ϊ�� */
    pxList->xListEnd.pxNext = (ListItem_t *)(&(pxList->xListEnd));
    pxList->xListEnd.pxPrevious = (ListItem_t *)(&(pxList->xListEnd));
    /* ��ʼ������ڵ��������ֵΪ0����ʾ����Ϊ�� */
    pxList->uxNumberOfItems = (UBaseType_t)0U;
}

/* �ڵ��ʼ�� */
void vListInitialiseItem(ListItem_t * const pxItem)
{
    /* ��ʼ���ýڵ����ڵ�����Ϊ�գ���ʾ�ýڵ�û�в����κ����� */
    pxItem->pvContainer = NULL;
}

/* �ڵ���뵽����β�� */
void vListInsertEnd(List_t * const pxList, ListItem_t * const pxNewtListItem)
{
    ListItem_t * const pxIndex = pxList->pxIndex;
    pxNewtListItem->pxNext = pxIndex;
    pxNewtListItem->pxPrevious = pxIndex->pxPrevious;
    pxIndex->pxPrevious->pxNext = pxNewtListItem;
    pxIndex->pxPrevious = pxNewtListItem;
    //pxIndex->pxNext ָ�򲻱仯
    
    /* ��ס�ýڵ����ڵ����� */
    pxNewtListItem->pvContainer = (void *)pxList;
    
    /* ����ڵ������++ */
    (pxList->uxNumberOfItems)++;
}

/* ���ڵ㰴�� xItemValue ������뵽���� */
void vListInsert(List_t * const pxList, ListItem_t * const pxNewListItem)
{
    /* 
     * ��¼����ֵ�� pxNewListItem-> xItemValue С�ĵ��б��� ItemX ��
     * ItemX��������б��� ItemX++ �ĸ���ֵ����ڻ���� pxNewListItem-> xItemValue
     * �����λ�ü� ItemX ֮��
    */
    ListItem_t *pxIterator = NULL;
    
    /* ��ȡ�ڵ�������ֵ */
    const TickType_t xValueOfInsertion = pxNewListItem->xItemValue;
    
    /* �ڵ�Ҫ���뵽�����β�� */
    if(xValueOfInsertion == portMAX_DELAY)
    {
        pxIterator = pxList->xListEnd.pxPrevious;
    }
    else
    {
        /* ��ǰ������� */
        for(pxIterator = (ListItem_t *)(&(pxList->xListEnd));
            pxIterator->pxNext->xItemValue <= xValueOfInsertion;
            pxIterator = pxIterator->pxNext)
        {
            /*�����������飬ֻ��Ϊ���ҵ�Ҫ�����λ�� */
        }
    }
    
    pxNewListItem->pxNext = pxIterator->pxNext;
    pxNewListItem->pxNext->pxPrevious = pxNewListItem; //���� pxIterator->pxNext->pxPrevious = pxNewListItem
    pxNewListItem->pxPrevious = pxIterator;
    pxIterator->pxNext = pxNewListItem;
    
    /* ��ס�ýڵ����ڵ����� */
    pxNewListItem->pvContainer = (void *)pxList;
    
    /* ����ڵ������++ */
    (pxList->uxNumberOfItems)++;
}

/* ���ڵ��������ɾ�� */
UBaseType_t uxListRemove(ListItem_t * const pxItemToRemove)
{
    /* ��ȡ�ڵ������б� */
    List_t * const pxList = pxItemToRemove->pvContainer;
    pxItemToRemove->pxPrevious->pxNext = pxItemToRemove->pxNext;
    pxItemToRemove->pxNext->pxPrevious = pxItemToRemove->pxPrevious;
    
    /* ��ʼ���ڵ����ڵ�����Ϊ�գ���ʾ�ڵ㻹û�в����κ����� */
    pxItemToRemove->pvContainer = NULL;
    
    /* ����ڵ������--*/
    (pxList->uxNumberOfItems)--;
    
    /* ����������ʣ��ڵ�ĸ��� */
    return pxList->uxNumberOfItems;
}
