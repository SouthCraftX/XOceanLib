#pragma once
#define __XOCEAN_LIST_INLINE_IMPL_H__

#include "container_base.h"
#include "../memory/memory_oper.h"

#include "atomic.h"

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

struct _XOceanListNode
{
    XOceanListNode *    next;
    XOceanListNode *    prev;
    xocean_byte_t       data[];
};
typedef struct _XOceanListNode XOceanListNode;

struct _XOceanList
{
    xocean_ref_count_t  ref_count;
    xocean_bool_t       has_private_heap;
    xocean_size_t       element_size;
    xocean_size_t       element_count;
    XOceanListNode *    head;
    XOceanListNode *    tail;
    XOceanListNode *    cur;
    mi_heap_t *         heap;
    __xocean_list_destructor_f destructor;
};
typedef struct _XOceanList XOceanList;
typedef void(*__xocean_list_destructor_f)(XOceanList *);

XOCEAN_FORCE_INLINE
xocean_pointer_t
XOCEAN_IMPL(xocean_list_get_cur)(
    XOceanList * list
){
    return list->cur->data;
}

// @brief iterate the list forward from head
// @param list: the list to be iterated
// @param p: the pointer to be filled
#define XOCEAN_LIST_ITERATE_FORWARD(list , p) \
    for(XOceanListNode * ___cur_node = (list)->head ; \
        ___cur_node != NULL ; \
        (p) = ___cur_node->data , ___cur_node = ___cur_node->next)

// @brief iterate the list backward from tail
// @param list: the list to be iterated
// @param p: the pointer to be filled
#define XOCEAN_LIST_ITERATE_BACKWARD(list , p) \
    for(XOceanListNode * ___cur_node = (list)->tail ; \
        ___cur_node != NULL ; \
        (p) = ___cur_node->data , ___cur_node = ___cur_node->prev)

// @brief iterate the list forward from current node
// @param list: the list to be iterated
// @param p: the pointer to be filled
#define XOCEAN_LIST_CUR_ITERATE_FORWARD(list , p) \
    for(XOceanListNode * ___cur_node = (list)->cur ; \
        ___cur_node != NULL ; \
        (p) = ___cur_node->data , ___cur_node = ___cur_node->next)

// @brief iterate the list bacjward from current node 
// @param list: the list to be iterated
// @param p: the pointer to be filled
#define XOCEAN_LIST_CUR_ITERATE_BACKWARD(list , p) \
    for(XOceanListNode * ___cur_node = (list)->cur ; \
        ___cur_node != NULL ; \
        (p) = ___cur_node->data , ___cur_node = ___cur_node->prev)

// @brief iterate the list forward from head
// @param list: the list to be iterated
// @param cp: the pointer to be filled
#define XOCEAN_LIST_CONST_ITERATE_FORWARD(list , cp) \
    XOCEAN_LIST_ITERATE_FORWARD((list) , (const)cp)

// @brief iterate the list backward from tail
// @param list: the list to be iterated
// @param cp: the pointer to be filled
#define XOCEAN_LIST_CONST_ITERATE_BACKWARD(list , cp) \
    XOCEAN_LIST_ITERATE_BACKWARD((list) , (const)cp)

#define XOCEAN_LIST_ITERATE XOCEAN_LIST_ITERATE_FORWARD

void
__xocean_list_cleanup_for_private_heap(
    XOceanList * list
){
    mi_heap_delete(list->heap);
}

void
__xocean_list_cleanup_for_shared_heap(
    XOceanList * list
){
    xocean_byte_t * p;
    XOCEAN_LIST_ITERATE_FORWARD(list , p)
    {
        mi_free(p - sizeof(XOceanListNode));
    }
}

void
XOCEAN_IMPL(xocean_list_unref)(
    XOceanList * list
){
    list->ref_count--;
    if(list->ref_count)
        return;
    list->destructor(list);
}

XOCEAN_FORCE_INLINE
xocean_size_t
__xocean_list_init_clac_alloc_size(
    xocean_size_t element_size
){
    return sizeof(XOceanList) + 
            (element_size + sizeof(XOceanListNode *)) *
            xocean_get_container_init_alloc_element_count();
}

XOCEAN_FORCE_INLINE
void
__xocean_list_prealloc(
    XOceanList *    list ,
    xocean_byte_t * mem ,
    xocean_size_t   node_size
){
    const xocean_size_t count = xocean_get_container_init_alloc_element_count();
    XOceanListNode * prev_node = ((XOceanListNode *)mem) , * node;
    ((XOceanListNode *)mem)->prev = NULL;
    for (xocean_size_t i = 1; i < count; i++)
    {
        node = (XOceanListNode *)(mem + i * node_size);
        node->prev = prev_node;
        prev_node->next = node;
        prev_node = node;
    }
    node->next = NULL;
    list->head = (XOceanListNode *)mem;
    list->tail = node;
    list->cur  = (XOceanListNode *)mem;
}

xocean_stat_t
XOCEAN_IMPL(xocean_list_init)(
    XOceanList *    list ,
    xocean_size_t   element_size ,
    xocean_flag32_t mode
){
    mi_heap_t * heap = __xocean_container_acquire_heap(mode);
    if(heap)
    {
        const xocean_size_t prealloc_size = (element_size + sizeof(XOceanListNode) *
            _xocean_get_container_init_alloc_element_count());
        xocean_pointer_t mem = mi_heap_malloc(heap , prealloc_size);
        if(mem)
        {
            __xocean_list_prealloc(list , (xocean_byte_t *)mem , 
                                   element_size + sizeof(XOceanListNode *));
            list->heap = heap;
            list->element_size = element_size;
            list->has_private_heap = XOCEAN_HAS_FLAG(
                mode , 
                XOCEAN_CONTAINER_PRIVATE_HEAP
            );
            list->destructor = list->has_private_heap ? 
                __xocean_list_cleanup_for_private_heap :
                __xocean_list_cleanup_for_shared_heap;
            return XOCEAN_OK;   
        }
        return XOCEAN_OUT_OF_MEMORY;
    }
    return XOCEAN_OUT_OF_MEMORY;
}

XOCEAN_FORCE_INLINE
xocean_size_t
__xocean_list_node_clac_size(
    XOceanList * list
){
    return sizeof(XOceanListNode) + list->element_size;
}

XOCEAN_FORCE_INLINE
void
XOCEAN_IMPL(xocean_list_insert_node_ahead)(
    XOceanList *        list ,
    XOceanListNode *    node
){
    node->next = list->cur;
    node->prev = list->cur->prev;
    list->cur->prev->next = node;
    list->cur->prev = node;
}


XOCEAN_FORCE_INLINE
xocean_bool_t
XOCEAN_IMPL(xocean_list_insert_ahead)(
    XOceanList * list,
    xocean_pointer_t p_element
){
    XOceanListNode * new_node = (XOceanListNode *)mi_heap_malloc(list->heap , 
                                __xocean_list_node_clac_size(list));
    if (new_node)
    {
        if(p_element)
            xocean_memory_copy(p_element , new_node->data , list->element_size);
        new_node->next = list->cur->next;
        new_node->prev = list->cur;
        list->cur->next = new_node;
        return xocean_true;
    }
    return xocean_false;
}


XOCEAN_FORCE_INLINE
void
XOCEAN_IMPL(xocean_list_set)(
    XOceanList * list ,
    xocean_pointer_t p_element
){
    xocean_pointer_t p;
    XOCEAN_LIST_ITERATE(list , p)
    {
        xocean_memory_copy(p_element , p , list->element_size);
    }
}

XOCEAN_FORCE_INLINE
xocean_bool_t
XOCEAN_IMPL(xocean_list_insert_backward)(
    XOceanList * list ,
    xocean_pointer_t p_element
){
    XOceanListNode * new_node = (XOceanListNode *)mi_heap_malloc(list->heap , 
                                __xocean_list_node_clac_size(list));
    if (new_node)
    {
        if(p_element)
            xocean_memory_copy(p_element , new_node->data , list->element_size);
        new_node->next = list->cur;
        new_node->prev = list->cur->prev;
        list->cur->prev = new_node;
        return xocean_true;
    }
    return xocean_false;
}

XOCEAN_FORCE_INLINE
xocean_size_t
XOCEAN_IMPL(xocean_list_insert_multiple_ahead)(
    XOceanList *    list ,
    xocean_byte_t * p_elements ,
    xocean_size_t   count
){
    xocean_size_t inserted_count = 0;
    for ( ; inserted_count < count ; inserted_count++)
    {
        if(XOCEAN_INTERFACE(xocean_list_insert_ahead)(list , p_elements))
        {
            p_elements += list->element_size;
        }
        break;
    }
    list->element_count += inserted_count;
    return inserted_count;
}

XOCEAN_FORCE_INLINE
xocean_size_t
XOCEAN_IMPL(xocean_list_insert_multiple_backward)(
    XOceanList *    list ,
    xocean_byte_t * p_elements ,
    xocean_size_t   count
){
    xocean_size_t inserted_count = 0;
    for ( ; inserted_count < count ; inserted_count++)
    {
        if(XOCEAN_INTERFACE(xocean_list_insert_backward)(list , p_elements))
        {
            p_elements += list->element_size;
        }
        break;
    }
    list->element_count += inserted_count;
    return inserted_count;
}

XOCEAN_FORCE_INLINE
xocean_bool_t
XOCEAN_IMPL(xocean_list_insert_tail)(
    XOceanList *            list ,
    const xocean_pointer_t  element
){
    XOceanListNode * node = (XOceanListNode*)mi_heap_malloc(
        list->heap , 
        sizeof(XOceanListNode) + list->element_size
    );
    if(node)
    {
        list->tail->next = node;
        node->prev = list->tail;
        node->next = NULL;
        list->tail = node;
        list->element_count ++;
        xocean_memory_copy(element , node->data , list->element_size);
        return xocean_true;
    }
    return xocean_false;
}

XOCEAN_FORCE_INLINE
xocean_bool_t
XOCEAN_IMPL(xocean_list_insert_head)(
    XOceanList *            list ,
    const xocean_pointer_t  element
){
    XOceanListNode * node = (XOceanListNode*)mi_heap_malloc(
        list->heap ,
        sizeof(XOceanListNode) + list->element_size);
    if(XOCEAN_LIKELY(node))
    {
        list->head->prev = node;
        node->next = list->head;
        node->prev = NULL;
        list->head = node;
        list->element_count ++;
        xocean_memory_copy(element , node->data , list->element_size);
        return xocean_true;
    }
    return xocean_false;
}

XOCEAN_FORCE_INLINE
void
XOCEAN_IMPL(xocean_list_insert_node_to_head)(
    XOceanList *            list ,
    XOceanListNode *        node
){
    list->head->prev = node;
    node->next = list->head;
    node->prev = NULL;
    list->head = node;
}

XOCEAN_FORCE_INLINE
void
XOCEAN_IMPL(xocean_list_insert_node_to_tail)(
    XOceanList *            list ,
    XOceanListNode *        node
){
    list->tail->next = node;
    node->prev = list->tail;
    node->next = NULL;
    list->tail = node;
}

void
XOCEAN_IMPL(xocean_list_insert_node_to_tail_cas)(
    XOceanList *            list ,
    XOceanListNode *        new_tail_node
){
    XOceanListNode * old_tail = list->tail;
    while (xocean_atomic_cas(&list->tail , old_tail , new_tail_node))
    {
        old_tail = list->tail; 
        // Keep updating old_tail until the tail of list is no longer changed. 
    }
    
}

void
XOCEAN_IMPL(xocean_list_insert_node_to_head_cas)(
    XOceanList *        list ,
    XOceanListNode *    new_head_node
){
    XOceanListNode * old_head = list->head;
    while (xocean_atomic_cas_strong_noexplicit(&list->head , old_head , new_head_node))
    {
        old_head = list->head; 
        // Keep updating old_head until the head of list is no longer changed. 
    }
}   

XOCEAN_FORCE_INLINE
void
XOCEAN_IMPL(xocean_list_goto_next)(
    XOceanList * list
){
    list->cur = list->cur->next;
}

XOCEAN_FORCE_INLINE
void
XOCEAN_IMPL(xocean_list_goto_prev)(
    XOceanList * list
){
    list->cur = list->cur->prev;
}

XOCEAN_FORCE_INLINE
void
XOCEAN_IMPL(xocean_list_remove_cur)(
    XOceanList * list 
){
    list->cur->prev->next = list->cur->next;
    list->cur->next->prev = list->cur->prev;
    mi_free(list->cur);
    list->cur = list->cur->prev;
    list->element_count--;
}

xocean_size_t
XOCEAN_IMPL(xocean_list_remove_front)(
    XOceanList * list ,
    xocean_size_t count
){
    xocean_size_t remove_count = 0;
    XOceanListNode * now = list->head->prev , * prev;
    while (now && remove_count < count)
    {
        prev = now->prev;
        mi_free(now);
        now = prev;
    }
    if(now) // Is it not the first node?
    {
        list->head->prev = now;
        now->next = list->head;
    }
    else
    {
        list->head->prev = NULL;
    }
    list->element_count -= remove_count;
    return remove_count;
}

xocean_size_t
XOCEAN_IMPL(xocean_list_remove_backward)(
    XOceanList * list ,
    xocean_size_t count
){
    xocean_size_t remove_count = 0;
    XOceanListNode * now = list->cur->next , * next;
    while (now && remove_count < count)
    {
        next = now->next;
        mi_free(now);
        now = next;
    }
    if(now) // Is it not the last node?
    {
        list->cur->next = now;
        now->prev = list->cur;
    }
    else
    {
        list->cur->next = NULL;
    }
    list->element_count -= remove_count;
    return remove_count;
}

XOCEAN_FORCE_INLINE
void
XOCEAN_IMPL(xocean_list_remove_head)(
    XOceanList * list
){
    XOceanListNode * old_head_next = list->head->next;
    old_head_next->prev = NULL;
    mi_free(list->head);
    list->head = old_head_next;
    list->element_count--;
}

XOCEAN_FORCE_INLINE
xocean_size_t
XOCEAN_IMPL(xocean_list_get_element_count)(
    XOceanList * list
){
    return list->element_count;
}

XOCEAN_FORCE_INLINE
xocean_size_t
XOCEAN_IMPL(xocean_list_get_element_size)(
    XOceanList * list
){
    return list->element_size;
}

XOCEAN_FORCE_INLINE
xocean_size_t
XOCEAN_IMPL(xocean_list_get_data_size)(
    XOceanList * list
){
    return list->element_count * (list->element_size + sizeof(XOceanListNode));
}

xocean_size_t
XOCEAN_IMPL(xocean_list_move_ahead)(
    XOceanList * list ,
    xocean_size_t count
){
    xocean_size_t move_count = 0;
    while (list->cur->next && move_count < count)
    {
        list->cur = list->cur->next;
        move_count++;
    }
    return move_count;
}


xocean_size_t
XOCEAN_IMPL(xocean_list_move_backward)(
    XOceanList *    list ,
    xocean_size_t   count
){
    xocean_size_t move_count = 0;
    while (list->cur->prev && move_count < count)
    {
        list->cur = list->cur->prev;
        move_count++;
    }
    return move_count;
}

XOCEAN_FORCE_INLINE
void
XOCEAN_IMPL(xocean_list_goto_head)(
    XOceanList * list
){
    list->cur = list->head;
}

XOCEAN_FORCE_INLINE
void
XOCEAN_IMPL(xocean_list_goto_tail)(
    XOceanList * list
){
    list->cur = list->tail;
}

XOCEAN_FORCE_INLINE
void
XOCEAN_IMPL(xocean_list_remove_all)(
    XOceanList * list
){
    XOCEAN_IMPL(xocean_list_remove_backward)(list , XOCEAN_SIZE_T_MAX);
    XOCEAN_IMPL(xocean_list_remove_front)(list , XOCEAN_SIZE_T_MAX);
}

XOCEAN_FORCE_INLINE
void
XOCEAN_IMPL(xocean_list_destory)(
    XOceanList * list
){
    (*(list->destructor))(list);
}



#if defined(__cplusplus)
}
#endif // __cplusplus