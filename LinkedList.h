/*
 * LinkedList.h
 * 
 * Creator: Yin
 * Date: 2017. 7.4
 */
 
#ifndef LinkedList_h
#define LinkedList_h

#include <stddef.h>

template<class T>
struct ListNode
{
	T data;
	ListNode<T> *next;
};

template <typename T> class LinkedList
{
protected:
	int _size;
	ListNode<T> *root;
	ListNode<T>	*last;

	ListNode<T> *lastNodeGot;
	int lastIndexGot;

	bool isCached;

	ListNode<T>* getNode(int index);

	ListNode<T>* findEndOfSortedString(ListNode<T> *p, int (*cmp)(T &, T &));

public:
	LinkedList();
	~LinkedList();

	virtual int size();
	virtual bool add(int index, T);
	virtual bool add(T);
	virtual bool unshift(T);
	virtual bool set(int index, T);
	virtual T remove(int index);
	virtual T pop();
	virtual T shift();
	virtual T get(int index);
	virtual void clear();
	virtual void sort(int (*cmp)(T &, T &));
};

template<typename T> LinkedList<T>::LinkedList()
{
  root=NULL;
  last=NULL;
  _size=0;

  lastNodeGot = root;
  lastIndexGot = 0;
  isCached = false;
}

template<typename T> LinkedList<T>::~LinkedList()
{
  ListNode<T>* tmp;
  while (root!=NULL)
  {
    tmp=root;
    root=root->next;
    delete tmp;
  }
  last = NULL;
  _size=0;
  isCached = false;
}

template<typename T> ListNode<T>* LinkedList<T>::getNode(int index){

  int pos = 0;
  ListNode<T>* current = root;

  if (isCached && lastIndexGot <= index)
  {
    pos = lastIndexGot;
    current = lastNodeGot;
  }

  while (pos < index && current)
  {
    current = current->next;
    pos++;
  }

  if (pos == index)
  {
    isCached = true;
    lastIndexGot = index;
    lastNodeGot = current;
    return current;
  }
  return false;
}

template<typename T> int LinkedList<T>::size()
{
  return _size;
}

template<typename T> bool LinkedList<T>::add(int index, T t)
{
  if (index >= _size)
    return add(t);

  if (index == 0)
    return unshift(t);

  ListNode<T> *tmp = new ListNode<T>(), *prev = getNode(index-1);
  tmp->data = t;
  tmp->next = prev->next;
  prev->next = tmp;

  _size++;
  isCached = false;

  return true;
}

template<typename T> bool LinkedList<T>::add(T t)
{
  ListNode<T> *tmp = new ListNode<T>();
  tmp->data = t;
  tmp->next = NULL;
  
  if (root)
  {
    last->next = tmp;
    last = tmp;
  }
  else
  {
    root = tmp;
    last = tmp;
  }

  _size++;
  isCached = false;

  return true;
}

template<typename T> bool LinkedList<T>::unshift(T t)
{
  if (_size == 0)
    return add(t);

  ListNode<T> *tmp = new ListNode<T>();
  tmp->next = root;
  tmp->data = t;
  root = tmp;
  
  _size++;
  isCached = false;
  
  return true;
}

template<typename T> bool LinkedList<T>::set(int index, T t)
{
  if (index < 0 || index >= _size)
    return false;

  getNode(index)->data = t;
  return true;
}

template<typename T> T LinkedList<T>::pop()
{
  if (_size <= 0)
    return T();
  
  isCached = false;

  if (_size >= 2)
  {
    ListNode<T> *tmp = getNode(_size - 2);
    T ret = tmp->next->data;
    delete(tmp->next);
    tmp->next = NULL;
    last = tmp;
    _size--;
    return ret;
  }
  else
  {
    // Only one element left on the list
    T ret = root->data;
    delete(root);
    root = NULL;
    last = NULL;
    _size = 0;
    return ret;
  }
}

template<typename T> T LinkedList<T>::shift()
{
  if (_size <= 0)
    return T();

  if (_size > 1)
  {
    ListNode<T> *_next = root->next;
    T ret = root->data;
    delete(root);
    root = _next;
    _size--;
    isCached = false;

    return ret;
  }
  else
  {
    // Only one left, then pop()
    return pop();
  }

}

template<typename T> T LinkedList<T>::remove(int index)
{
  if (index < 0 || index >= _size)
    return T();

  if (index == 0)
    return shift();
  
  if (index == _size-1)
  {
    return pop();
  }

  ListNode<T> *tmp = getNode(index - 1);
  ListNode<T> *toDelete = tmp->next;
  T ret = toDelete->data;
  tmp->next = tmp->next->next;
  delete toDelete;
  _size--;
  isCached = false;
  return ret;
}

template<typename T> T LinkedList<T>::get(int index)
{
  ListNode<T> *tmp = getNode(index);

  return (tmp ? tmp->data : T());
}

template<typename T> void LinkedList<T>::clear()
{
  while (size() > 0)
    shift();
}

template<typename T> void LinkedList<T>::sort(int (*cmp)(T &, T &))
{
  if (_size < 2) return; // trivial case;

  for(;;)
  {
    ListNode<T> **joinPoint = &root;

    while (*joinPoint)
    {
      ListNode<T> *a = *joinPoint;
      ListNode<T> *a_end = findEndOfSortedString(a, cmp);
  
      if (!a_end->next)
      {
        if (joinPoint == &root)
        {
          last = a_end;
          isCached = false;
          return;
        }
        else
          break;
      }

      ListNode<T> *b = a_end->next;
      ListNode<T> *b_end = findEndOfSortedString(b, cmp);

      ListNode<T> *tail = b_end->next;

      a_end->next = NULL;
      b_end->next = NULL;

      while (a && b)
      {
        if (cmp(a->data, b->data) <= 0)
        {
          *joinPoint = a;
          joinPoint = &a->next;
          a = a->next;  
        }
        else
        {
          *joinPoint = b;
          joinPoint = &b->next;
          b = b->next;  
        }
      }

      if (a)
      {
        *joinPoint = a;
        while (a->next) a = a->next;
        a->next = tail;
        joinPoint = &a->next;
      }
      else
      {
        *joinPoint = b;
        while (b->next) b = b->next;
        b->next = tail;
        joinPoint = &b->next;
      }
    }
  }
}

template<typename T> ListNode<T>* LinkedList<T>::findEndOfSortedString(ListNode<T> *p, int (*cmp)(T &, T &))
{
  while (p->next && cmp(p->data, p->next->data) <= 0)
    p = p->next;
  
  return p;
}

#endif
