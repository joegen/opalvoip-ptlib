/*
 * $Id: collect.cxx,v 1.8 1994/10/23 03:41:31 robertj Exp $
 *
 * Portable Windows Library
 *
 * PContainer Class Implementation
 *
 * Copyright 1993 Equivalence
 *
 * $Log: collect.cxx,v $
 * Revision 1.8  1994/10/23 03:41:31  robertj
 * Fixed dictionary functions that should work by index not key.
 *
 * Revision 1.7  1994/09/25  10:49:09  robertj
 * Removed redundent PAssertNULL.
 *
 * Revision 1.6  1994/08/21  23:43:02  robertj
 * Fixed bug in lists when inserting element.
 *
 * Revision 1.5  1994/07/27  05:58:07  robertj
 * Synchronisation.
 *
 * Revision 1.4  1994/07/17  10:46:06  robertj
 * Fixed searching in sorted lists.
 *
 * Revision 1.3  1994/07/02  03:03:49  robertj
 * Added container searching facilities..
 *
 * Revision 1.2  1994/06/25  11:55:15  robertj
 * Unix version synchronisation.
 *
// Revision 1.1  1994/04/20  12:17:44  robertj
// Initial revision
//
 */

#include <contain.h>



///////////////////////////////////////////////////////////////////////////////

ostream & PCollection::PrintOn(ostream &strm) const
{
  for (PINDEX  i = 0; i < GetSize(); i++) {
    PObject * obj = GetAt(i);
    if (obj != NULL)
      strm << *obj;
  }
  return strm;
}


void PCollection::RemoveAll()
{
  while (GetSize() > 0)
    RemoveAt(0);
}


///////////////////////////////////////////////////////////////////////////////

void PArrayObjects::DestroyContents()
{
  if (reference->deleteObjects) {
    for (PINDEX i = 0; i < theArray.GetSize(); i++) {
      if (theArray[i] != NULL)
        delete theArray[i];
    }
  }
}


void PArrayObjects::CloneContents(const PArrayObjects * array)
{
  for (PINDEX i = 0; i < array->GetSize(); i++) {
    PObject * ptr = array->GetAt(i);
    if (ptr != NULL)
      SetAt(i, ptr->Clone());
  }
}


PObject::Comparison PArrayObjects::Compare(const PObject & obj) const
{
  const PArrayObjects & other = (const PArrayObjects &)obj;
  for (PINDEX i = 0; i < GetSize(); i++) {
    if (i >= other.GetSize() || *theArray[i] < *other.theArray[i])
      return LessThan;
    if (*theArray[i] > *other.theArray[i])
      return GreaterThan;
  }
  return i < other.GetSize() ? GreaterThan : EqualTo;
}


PINDEX PArrayObjects::GetSize() const
{
  return theArray.GetSize();
}


BOOL PArrayObjects::SetSize(PINDEX newSize)
{
  return theArray.SetSize(newSize);
}


PINDEX PArrayObjects::Append(PObject * obj)
{
  PINDEX where = GetSize();
  SetAt(where, obj);
  return where;
}


PINDEX PArrayObjects::Insert(const PObject & before, PObject * obj)
{
  PINDEX where = GetObjectsIndex(&before);
  InsertAt(where, obj);
  return where;
}


BOOL PArrayObjects::Remove(const PObject * obj)
{
  PINDEX i = GetObjectsIndex(obj);
  if (i == P_MAX_INDEX)
    return FALSE;
  RemoveAt(i);
  return TRUE;
}


PObject * PArrayObjects::GetAt(PINDEX index) const
{
  return theArray[index];
}


BOOL PArrayObjects::SetAt(PINDEX index, PObject * obj)
{
  if (!theArray.MakeUnique() ||
                   (index >= theArray.GetSize() && !theArray.SetSize(index+1)))
    return FALSE;
  PObject * oldObj = theArray.GetAt(index);
  if (oldObj != NULL && reference->deleteObjects)
    delete oldObj;
  theArray[index] = obj;
  return TRUE;
}


PINDEX PArrayObjects::InsertAt(PINDEX index, PObject * obj)
{
  for (PINDEX i = index; i < GetSize(); i++)
    theArray.SetAt(i+1, theArray[i]);
  SetAt(index, obj);
  return index;
}


PObject * PArrayObjects::RemoveAt(PINDEX index)
{
  PObject * obj = theArray[index];
  for (PINDEX i = index; i < GetSize(); i++)
    theArray[i] = theArray[i+1];
  SetSize(GetSize()-1);
  if (obj != NULL && reference->deleteObjects) {
    delete obj;
    obj = NULL;
  }
  return obj;
}


PINDEX PArrayObjects::GetObjectsIndex(const PObject * obj) const
{
  for (PINDEX i = 0; i < GetSize(); i++) {
    if (theArray[i] == obj)
      return i;
  }
  return P_MAX_INDEX;
}


PINDEX PArrayObjects::GetValuesIndex(const PObject & obj) const
{
  for (PINDEX i = 0; i < GetSize(); i++) {
    if (*theArray[i] == obj)
      return i;
  }
  return P_MAX_INDEX;
}


BOOL PArrayObjects::Enumerate(PEnumerator func, PObject * info) const
{
  for (PINDEX i = 0; i < GetSize(); i++) {
    if (!func(*theArray[i], info))
      return FALSE;
  }
  return TRUE;
}


///////////////////////////////////////////////////////////////////////////////

void PAbstractList::DestroyContents()
{
  RemoveAll();
  delete info;
}


void PAbstractList::CopyContents(const PAbstractList & list)
{
  info = list.info;
}


void PAbstractList::CloneContents(const PAbstractList * list)
{
  info = new ListInfo;
  PAssertNULL(info);
  for (PListElement * element = list->info->head;
                                      element != NULL; element = element->next)
    Append(element->data->Clone());
}


PObject::Comparison PAbstractList::Compare(const PObject & obj) const
{
  PListElement * elmt1 = info->head;
  PListElement * elmt2 = ((const PAbstractList &)obj).info->head;
  while (elmt1 != NULL && elmt2 != NULL) {
    if (elmt1 == NULL)
      return LessThan;
    if (elmt2 == NULL)
      return GreaterThan;
    if (*elmt1->data < *elmt2->data)
      return LessThan;
    if (*elmt1->data > *elmt2->data)
      return GreaterThan;
    elmt1 = elmt1->next;
    elmt2 = elmt2->next;
  }
  return EqualTo;
}


BOOL PAbstractList::SetSize(PINDEX)
{
  return TRUE;
}


PINDEX PAbstractList::Append(PObject * obj)
{
  PListElement * element = new PListElement(PAssertNULL(obj));
  if (info->tail != NULL)
    info->tail->next = element;
  element->prev = info->tail;
  element->next = NULL;
  if (info->head == NULL)
    info->head = element;
  info->tail = element;
  info->lastElement = element;
  info->lastIndex = GetSize();
  reference->size++;
  return info->lastIndex;
}


PINDEX PAbstractList::Insert(const PObject & before, PObject * obj)
{
  PAssertNULL(obj);
  
  PINDEX where = GetObjectsIndex(&before);
  InsertAt(where, obj);
  return where;
}


PINDEX PAbstractList::InsertAt(PINDEX index, PObject * obj)
{
  PAssertNULL(obj);

  if (index >= GetSize())
    return Append(obj);

  PAssert(SetCurrent(index), PInvalidArrayIndex);

  PListElement * newElement = new PListElement(obj);
  if (info->lastElement->prev != NULL)
    info->lastElement->prev->next = newElement;
  else
    info->head = newElement;
  newElement->prev = info->lastElement->prev;
  newElement->next = info->lastElement;
  info->lastElement->prev = newElement;
  info->lastElement = newElement;
  info->lastIndex = index;
  reference->size++;
  return index;
}


BOOL PAbstractList::Remove(const PObject * obj)
{
  PINDEX i = GetObjectsIndex(obj);
  if (i == P_MAX_INDEX)
    return FALSE;
  RemoveAt(i);
  return TRUE;
}


PObject * PAbstractList::RemoveAt(PINDEX index)
{
  PAssert(SetCurrent(index), PInvalidArrayIndex);

  PListElement * elmt = info->lastElement;

  if (elmt->prev != NULL)
    elmt->prev->next = elmt->next;
  else {
    info->head = elmt->next;
    if (info->head != NULL)
      info->head->prev = NULL;
  }

  if (elmt->next != NULL)
    elmt->next->prev = elmt->prev;
  else {
    info->tail = elmt->prev;
    if (info->tail != NULL)
      info->tail->next = NULL;
  }

  if (elmt->next != NULL)
    info->lastElement = elmt->next;
  else {
    info->lastElement = elmt->prev;
    info->lastIndex--;
  }
  reference->size--;

  PObject * obj = elmt->data;
  if (obj != NULL && reference->deleteObjects) {
    delete obj;
    obj = NULL;
  }
  delete elmt;
  return obj;
}


PObject * PAbstractList::GetAt(PINDEX index) const
{
  return SetCurrent(index) ? info->lastElement->data : NULL;
}


BOOL PAbstractList::SetAt(PINDEX index, PObject * val)
{
  if (!SetCurrent(index))
    return FALSE;
  info->lastElement->data = val;
  return TRUE;
}


PINDEX PAbstractList::GetObjectsIndex(const PObject * obj) const
{
  PINDEX index = 0;
  PListElement * element = info->head;
  while (element != NULL) {
    if (element->data == obj) {
      info->lastElement = element;
      info->lastIndex = index;
      return index;
    }
    element = element->next;
    index++;
  }

  return P_MAX_INDEX;
}


PINDEX PAbstractList::GetValuesIndex(const PObject & obj) const
{
  PINDEX index = 0;
  PListElement * element = info->head;
  while (element != NULL) {
    if (*element->data == obj) {
      info->lastElement = element;
      info->lastIndex = index;
      return index;
    }
    element = element->next;
    index++;
  }

  return P_MAX_INDEX;
}


BOOL PAbstractList::Enumerate(PEnumerator func, PObject * inf) const
{
  for (PListElement * element = info->head;
                                    element != NULL; element = element->next) {
    if (!func(*element->data, inf))
      return FALSE;
  }
  return TRUE;
}


BOOL PAbstractList::SetCurrent(PINDEX index) const
{
  if (index >= GetSize())
    return FALSE;

  if (info->lastElement == NULL || info->lastIndex >= GetSize() || 
      index < info->lastIndex/2 || index > (info->lastIndex+GetSize())/2) {
    if (index < GetSize()/2) {
      info->lastIndex = 0;
      info->lastElement = info->head;
    }
    else {
      info->lastIndex = GetSize()-1;
      info->lastElement = info->tail;
    }
  }

  while (info->lastIndex < index) {
    info->lastElement = info->lastElement->next;
    info->lastIndex++;
  }

  while (info->lastIndex > index) {
    info->lastElement = info->lastElement->prev;
    info->lastIndex--;
  }

  return TRUE;
}


PListElement::PListElement(PObject * theData)
{
  next = prev = NULL;
  data = theData;
}


///////////////////////////////////////////////////////////////////////////////

void PAbstractSortedList::DestroyContents()
{
  RemoveAll();
  delete info;
}


void PAbstractSortedList::CopyContents(const PAbstractSortedList & list)
{
  info = list.info;
}


void PAbstractSortedList::CloneContents(const PAbstractSortedList * list)
{
  info = new SortedListInfo;
  PAssertNULL(info);
  PSortedListElement * element = list->info->root;
  while (element->left != NULL)
    element = element->left;
  while (element != NULL) {
    Append(element->data->Clone());
    element = element->Successor();
  }
}


BOOL PAbstractSortedList::SetSize(PINDEX)
{
  return TRUE;
}


PObject::Comparison PAbstractSortedList::Compare(const PObject & obj) const
{
  PSortedListElement * elmt1 = info->root;
  while (elmt1->left != NULL)
    elmt1 = elmt1->left;

  PSortedListElement * elmt2 = ((const PAbstractSortedList &)obj).info->root;
  while (elmt2->left != NULL)
    elmt2 = elmt2->left;

  while (elmt1 != NULL && elmt2 != NULL) {
    if (elmt1 == NULL)
      return LessThan;
    if (elmt2 == NULL)
      return GreaterThan;
    if (*elmt1->data < *elmt2->data)
      return LessThan;
    if (*elmt1->data > *elmt2->data)
      return GreaterThan;
    elmt1 = elmt1->Successor();
    elmt2 = elmt2->Successor();
  }
  return EqualTo;
}


PINDEX PAbstractSortedList::Append(PObject * obj)
{
  PSortedListElement * element = new PSortedListElement(PAssertNULL(obj));
  PSortedListElement * child = info->root, * parent = NULL;
  while (child != NULL) {
    child->subTreeSize++;
    parent = child;
    child = *element->data < *child->data ? child->left : child->right;
  }
  element->parent = parent;
  if (parent == NULL)
    info->root = element;
  else if (*element->data < *parent->data)
    parent->left = element;
  else
    parent->right = element;

  element->MakeRed();
  while (element != info->root && !element->parent->IsBlack()) {
    if (element->parent == element->parent->parent->left) {
      child = element->parent->parent->right;
      if (child != NULL && !child->IsBlack()) {
        element->parent->MakeBlack();
        child->MakeBlack();
        element = element->parent->parent;
      }
      else {
        if (element == element->parent->right) {
          element = element->parent;
          LeftRotate(element);
        }
        element->parent->MakeBlack();
        element->parent->parent->MakeRed();
        RightRotate(element->parent->parent);
      }
    }
    else {
      child = element->parent->parent->left;
      if (child != NULL && !child->IsBlack()) {
        element->parent->MakeBlack();
        child->MakeBlack();
        element = element->parent->parent;
      }
      else {
        if (element == element->parent->left) {
          element = element->parent;
          RightRotate(element);
        }
        element->parent->MakeBlack();
        element->parent->parent->MakeRed();
        LeftRotate(element->parent->parent);
      }
    }
  }

  info->root->MakeBlack();

  reference->size++;
  info->lastIndex = P_MAX_INDEX;
  info->lastElement = NULL;
  return GetSize();
}


BOOL PAbstractSortedList::Remove(const PObject * obj)
{
  PSortedListElement * element = info->root;
  while (element != NULL && element->data != obj)
    element = *obj < *element->data ? element->left : element->right;
  if (element == NULL)
    return FALSE;

  RemoveElement(element);
  return TRUE;
}


PObject * PAbstractSortedList::RemoveAt(PINDEX index)
{
  PSortedListElement * node = info->root->OrderSelect(index+1);
  PObject * data = node->data;
  RemoveElement(node);
  return reference->deleteObjects ? NULL : data;
}


void PAbstractSortedList::RemoveAll()
{
  if (info->root != NULL) {
    info->root->DeleteSubTrees(reference->deleteObjects);
    delete info->root;
    info->root = NULL;
    reference->size = 0;
  }
}


PINDEX PAbstractSortedList::Insert(const PObject &, PObject * obj)
{
  return Append(obj);
}


PINDEX PAbstractSortedList::InsertAt(PINDEX, PObject * obj)
{
  return Append(obj);
}


BOOL PAbstractSortedList::SetAt(PINDEX, PObject *)
{
  return FALSE;
}


PObject * PAbstractSortedList::GetAt(PINDEX index) const
{
  if (index >= GetSize())
    return NULL;

  if (index == info->lastIndex-1) {
    info->lastIndex--;
    info->lastElement = info->lastElement->Predecessor();
  }
  else if (index == info->lastIndex+1) {
    info->lastIndex++;
    info->lastElement = info->lastElement->Successor();
  }
  else
    info->lastElement = info->root->OrderSelect(index+1);

  return info->lastElement->data;
}


BOOL PAbstractSortedList::Enumerate(PEnumerator func, PObject * inf) const
{
  PSortedListElement * element = info->root;
  if (element != NULL) {
    while (element->left != NULL)
      element = element->left;
    while (element != NULL) {
      if (!func(*element->data, inf))
        return FALSE;
      element = element->Successor();
    }
  }
  return TRUE;
}


PINDEX PAbstractSortedList::GetObjectsIndex(const PObject * obj) const
{
  return info->root == NULL ? P_MAX_INDEX : info->root->ValueSelect(*obj);
}


PINDEX PAbstractSortedList::GetValuesIndex(const PObject & obj) const
{
  return info->root == NULL ? P_MAX_INDEX : info->root->ValueSelect(obj);
}


void PAbstractSortedList::RemoveElement(PSortedListElement * node)
{
  PAssertNULL(node);

  if (node->data != NULL && reference->deleteObjects)
    delete node->data;

  PSortedListElement * y = 
          node->left == NULL || node->right == NULL ? node : node->Successor();
  PSortedListElement * x = y->left != NULL ? y->left : y->right;

  if (x != NULL)
    x->parent = y->parent;

  if (y->parent == NULL)
    info->root = x;
  else if (y == y->parent->left)
    y->parent->left = x;
  else
    y->parent->right = x;

  if (y != node) {
    node->data = y->data;
    node->subTreeSize = y->subTreeSize;
  }

  PSortedListElement * t = y->parent;
  while (t != NULL) {
    t->subTreeSize--;
    t = t->parent;
  }

  if (x != NULL && y->IsBlack()) {
    while (x != info->root && x->IsBlack()) {
      if (x == x->parent->left) {
        PSortedListElement * w = x->parent->right;
        if (!w->IsBlack()) {
          w->MakeBlack();
          x->parent->MakeRed();
          LeftRotate(x->parent);
          w = x->parent->right;
        }
        if (w->IsLeftBlack() && w->IsRightBlack()) {
          w->MakeRed();
          x = x->parent;
        }
        else {
          if (w->IsRightBlack()) {
            w->left->MakeBlack();
            w->MakeRed();
            RightRotate(w);
            w = x->parent->right;
          }
          w->colour = x->parent->colour;
          x->parent->MakeBlack();
          if (w->right != NULL)
            w->right->MakeBlack();
          LeftRotate(x->parent);
          x = info->root;
        }
      }
      else {
        PSortedListElement * w = x->parent->left;
        if (!w->IsBlack()) {
          w->MakeBlack();
          x->parent->MakeRed();
          RightRotate(x->parent);
          w = x->parent->left;
        }
        if (w->IsRightBlack() && w->IsLeftBlack()) {
          w->MakeRed();
          x = x->parent;
        }
        else {
          if (w->IsLeftBlack()) {
            w->right->MakeBlack();
            w->MakeRed();
            LeftRotate(w);
            w = x->parent->left;
          }
          w->colour = x->parent->colour;
          x->parent->MakeBlack();
          if (w->left != NULL)
            w->left->MakeBlack();
          RightRotate(x->parent);
          x = info->root;
        }
      }
    }
    x->MakeBlack();
  }

  delete y;

  reference->size--;
  info->lastIndex = P_MAX_INDEX;
  info->lastElement = NULL;
}


void PAbstractSortedList::LeftRotate(PSortedListElement * node)
{
  PSortedListElement * pivot = PAssertNULL(node)->right;
  node->right = pivot->left;
  if (pivot->left != NULL)
    pivot->left->parent = node;
  pivot->parent = node->parent;
  if (node->parent == NULL)
    info->root = pivot;
  else if (node == node->parent->left)
    node->parent->left = pivot;
  else
    node->parent->right = pivot;
  pivot->left = node;
  node->parent = pivot;
  pivot->subTreeSize = node->subTreeSize;
  node->subTreeSize = node->LeftTreeSize() + node->RightTreeSize() + 1;
}


void PAbstractSortedList::RightRotate(PSortedListElement * node)
{
  PSortedListElement * pivot = PAssertNULL(node)->left;
  node->left = pivot->right;
  if (pivot->right != NULL)
    pivot->right->parent = node;
  pivot->parent = node->parent;
  if (node->parent == NULL)
    info->root = pivot;
  else if (node == node->parent->right)
    node->parent->right = pivot;
  else
    node->parent->left = pivot;
  pivot->right = node;
  node->parent = pivot;
  pivot->subTreeSize = node->subTreeSize;
  node->subTreeSize = node->LeftTreeSize() + node->RightTreeSize() + 1;
}


PSortedListElement::PSortedListElement(PObject * theData)
{
  parent = left = right = NULL;
  colour = Black;
  subTreeSize = 1;
  data = theData;
}


PSortedListElement * PSortedListElement::Successor() const
{
  PSortedListElement * next;
  if (right != NULL) {
    next = right;
    while (next->left != NULL)
      next = next->left;
  }
  else {
    next = parent;
    const PSortedListElement * node = this;
    while (next != NULL && node == next->right) {
      node = next;
      next = node->parent;
    }
  }
  return next;
}


PSortedListElement * PSortedListElement::Predecessor() const
{
  PSortedListElement * pred;
  if (left != NULL) {
    pred = left;
    while (pred->right != NULL)
      pred = pred->right;
  }
  else {
    pred = parent;
    const PSortedListElement * node = this;
    while (pred != NULL && node == pred->right) {
      node = pred;
      pred = node->parent;
    }
  }
  return pred;
}


PSortedListElement * PSortedListElement::OrderSelect(PINDEX index)
{
  PINDEX r = LeftTreeSize()+1;
  if (index == r)
    return this;

  if (index < r) {
    if (left != NULL)
      return left->OrderSelect(index);
  }
  else {
    if (right != NULL)
      return right->OrderSelect(index - r);
  }

  return NULL;
}


PINDEX PSortedListElement::ValueSelect(const PObject & obj)
{
  switch (data->Compare(obj)) {
    case PObject::LessThan :
      if (right != NULL) {
        PINDEX index = right->ValueSelect(obj);
        return index != P_MAX_INDEX ? LeftTreeSize() + index : P_MAX_INDEX;
      }
      break;

    case PObject::GreaterThan :
      if (left != NULL)
        return left->ValueSelect(obj);
      break;

    default :
      return LeftTreeSize();
  }

  return P_MAX_INDEX;
}


void PSortedListElement::DeleteSubTrees(BOOL deleteObject)
{
  if (left != NULL) {
    left->DeleteSubTrees(deleteObject);
    delete left;
    left = NULL;
  }
  if (right != NULL) {
    right->DeleteSubTrees(deleteObject);
    delete right;
    right = NULL;
  }
  if (deleteObject) {
    delete data;
    data = NULL;
  }
}


///////////////////////////////////////////////////////////////////////////////

PObject * PScalarKey::Clone() const
{
  return PNEW PScalarKey(theKey);
}


PObject::Comparison PScalarKey::Compare(const PObject & obj) const
{
  const PScalarKey & other = (const PScalarKey &)obj;
  
  if (theKey < other.theKey)
    return LessThan;

  if (theKey > other.theKey)
    return GreaterThan;

  return EqualTo;
}


PINDEX PScalarKey::HashFunction() const
{
  return PABSINDEX(theKey)%23;
}


ostream & PScalarKey::PrintOn(ostream & strm) const
{
  return strm << theKey;
}


///////////////////////////////////////////////////////////////////////////////

void PInternalHashTable::DestroyContents()
{
  for (PINDEX i = 0; i < GetSize(); i++) {
    PHashTableElement * list = GetAt(i);
    if (list != NULL) {
      PHashTableElement * elmt = list;
      do {
        PHashTableElement * nextElmt = elmt->next;
        if (elmt->data != NULL && reference->deleteObjects)
          delete elmt->data;
        if (deleteKeys)
          delete elmt->key;
        delete elmt;
        elmt = nextElmt;
      } while (elmt != list);
    }
  }
  PAbstractArray::DestroyContents();
}


void PInternalHashTable::AppendElement(PObject * key, PObject * data)
{
  lastElement = NULL;

  PINDEX bucket = PAssertNULL(key)->HashFunction();
  PHashTableElement * list = GetAt(bucket);
  PHashTableElement * element = new PHashTableElement;
  PAssertNULL(element);
  element->key = key;
  element->data = data;
  if (list == NULL) {
    element->next = element->prev = element;
    SetAt(bucket, element);
  }
  else if (list == list->prev) {
    list->next = list->prev = element;
    element->next = element->prev = list;
  }
  else {
    element->next = list;
    element->prev = list->prev;
    list->prev->next = element;
    list->prev = element;
  }
  lastElement = element;
  lastIndex = P_MAX_INDEX;
}


PObject * PInternalHashTable::RemoveElement(const PObject & key)
{
  PObject * obj = NULL;
  if (GetElementAt(key) != NULL) {
    if (lastElement == lastElement->prev)
      SetAt(key.HashFunction(), NULL);
    else {
      lastElement->prev->next = lastElement->next;
      lastElement->next->prev = lastElement->prev;
      SetAt(key.HashFunction(), lastElement->next);
    }
    obj = lastElement->data;
    if (deleteKeys)
      delete lastElement->key;
    delete lastElement;
    lastElement = NULL;
  }
  return obj;
}


BOOL PInternalHashTable::SetLastElementAt(PINDEX index)
{
  if (index == 0 || lastElement == NULL || lastIndex == P_MAX_INDEX) {
    lastIndex = 0;
    lastBucket = 0;
    while ((lastElement = operator[](lastBucket)) == NULL) {
      if (lastBucket >= GetSize())
        return FALSE;
      lastBucket++;
    }
  }

  if (lastIndex == index)
    return TRUE;

  if (lastIndex < index) {
    while (lastIndex != index) {
      if (lastElement->next != operator[](lastBucket))
        lastElement = lastElement->next;
      else {
        do {
          if (++lastBucket >= GetSize())
            return FALSE;
        } while ((lastElement = operator[](lastBucket)) == NULL);
      }
      lastIndex++;
    }
  }
  else {
    while (lastIndex != index) {
      if (lastElement != operator[](lastBucket))
        lastElement = lastElement->prev;
      else {
        do {
          if (lastBucket-- == 0)
            return FALSE;
        } while ((lastElement = operator[](lastBucket)) == NULL);
        lastElement = lastElement->prev;
      }
      lastIndex--;
    }
  }

  return TRUE;
}


PHashTableElement * PInternalHashTable::GetElementAt(const PObject & key)
{
  if (lastElement != NULL && *lastElement->key == key)
    return lastElement;

  PHashTableElement * list = GetAt(key.HashFunction());
  if (list != NULL) {
    PHashTableElement * element = list;
    do {
      if (*element->key == key) {
        lastElement = element;
        lastIndex = P_MAX_INDEX;
        return lastElement;
      }
      element = element->next;
    } while (element != list);
  }
  return NULL;
}


PINDEX PInternalHashTable::GetElementsIndex(
                           const PObject * obj, BOOL byValue, BOOL keys) const
{
  PINDEX index = 0;
  for (PINDEX i = 0; i < GetSize(); i++) {
    PHashTableElement * list = operator[](i);
    if (list != NULL) {
      PHashTableElement * element = list;
      do {
        PObject * keydata = keys ? element->key : element->data;
        if (byValue ? (*keydata == *obj) : (keydata == obj))
          return index;
        index++;
      } while (element != list);
    }
  }
  return P_MAX_INDEX;
}


BOOL PInternalHashTable::EnumerateElements(
                            PEnumerator func, PObject * info, BOOL keys) const
{
  for (PINDEX i = 0; i < GetSize(); i++) {
    PHashTableElement * list = operator[](i);
    if (list != NULL) {
      PHashTableElement * element = list;
      do {
        if (!func(keys ? *element->key : *element->data, info))
          return FALSE;
      } while (element != list);
    }
  }
  return TRUE;
}



///////////////////////////////////////////////////////////////////////////////

PHashTable::PHashTable()
  : hashTable(PNEW PInternalHashTable)
{
  PAssertNULL(hashTable);
  hashTable->lastElement = NULL;
}


void PHashTable::DestroyContents()
{
  hashTable->reference->deleteObjects = reference->deleteObjects;
  delete hashTable;
}


void PHashTable::CopyContents(const PHashTable & hash)
{
  hashTable = hash.hashTable;
}

  
void PHashTable::CloneContents(const PHashTable * hash)
{
  hashTable = (PInternalHashTable *)hash->hashTable->Clone();
}


PObject::Comparison PHashTable::Compare(const PObject & obj) const
{
  return reference != ((const PHashTable &)obj).reference
                                                      ? GreaterThan : EqualTo;
}


BOOL PHashTable::SetSize(PINDEX)
{
  return TRUE;
}


PObject & PHashTable::AbstractGetDataAt(PINDEX index) const
{
  PAssert(hashTable->SetLastElementAt(index), PInvalidArrayIndex);
  return *hashTable->lastElement->data;
}


const PObject & PHashTable::AbstractGetKeyAt(PINDEX index) const
{
  PAssert(hashTable->SetLastElementAt(index), PInvalidArrayIndex);
  return *hashTable->lastElement->key;
}


///////////////////////////////////////////////////////////////////////////////

void PAbstractSet::DestroyContents()
{
  hashTable->deleteKeys = reference->deleteObjects;
  PHashTable::DestroyContents();
}


void PAbstractSet::CopyContents(const PAbstractSet & )
{
}

  
void PAbstractSet::CloneContents(const PAbstractSet * )
{
}


PINDEX PAbstractSet::Append(PObject * obj)
{
  if (!Contains(*obj)) {
    hashTable->AppendElement(obj, NULL);
    reference->size++;
  }
  return 0;
}


PINDEX PAbstractSet::Insert(const PObject &, PObject * obj)
{
  return Append(obj);
}


PINDEX PAbstractSet::InsertAt(PINDEX, PObject * obj)
{
  return Append(obj);
}


BOOL PAbstractSet::Remove(const PObject * obj)
{
  hashTable->deleteKeys = hashTable->reference->deleteObjects = reference->deleteObjects;
  if (!hashTable->RemoveElement(*obj))
    return FALSE;
  reference->size--;
  return TRUE;
}


PObject * PAbstractSet::RemoveAt(PINDEX)
{
  PAssertAlways(PUnimplementedFunction);
  return NULL;
}


PINDEX PAbstractSet::GetObjectsIndex(const PObject * obj) const
{
  return hashTable->GetElementsIndex(obj, FALSE, TRUE);
}


PINDEX PAbstractSet::GetValuesIndex(const PObject & obj) const
{
  return hashTable->GetElementsIndex(&obj, TRUE, TRUE);
}


BOOL PAbstractSet::SetAt(PINDEX, PObject * obj)
{
  return Append(obj);
;
}


PObject * PAbstractSet::GetAt(PINDEX) const
{
  return NULL;
}


BOOL PAbstractSet::Enumerate(PEnumerator func, PObject * info) const
{
  return hashTable->EnumerateElements(func, info, TRUE);
}



///////////////////////////////////////////////////////////////////////////////

PINDEX PAbstractDictionary::Append(PObject *)
{
  PAssertAlways(PUnimplementedFunction);
  return 0;
}


PINDEX PAbstractDictionary::Insert(const PObject &, PObject *)
{
  PAssertAlways(PUnimplementedFunction);
  return 0;
}


PINDEX PAbstractDictionary::InsertAt(PINDEX index, PObject * obj)
{
  SetAt(AbstractGetKeyAt(index), obj);
  return index;
}
 
 
BOOL PAbstractDictionary::Remove(const PObject *)
{
  PAssertAlways(PUnimplementedFunction);
  return FALSE;
}


PObject * PAbstractDictionary::RemoveAt(PINDEX index)
{
  PObject & obj = AbstractGetDataAt(index);
  SetAt(AbstractGetKeyAt(index), NULL);
  return &obj;
}


PINDEX PAbstractDictionary::GetObjectsIndex(const PObject * obj) const
{
  return hashTable->GetElementsIndex(obj, FALSE, FALSE);
}


PINDEX PAbstractDictionary::GetValuesIndex(const PObject & obj) const
{
  return hashTable->GetElementsIndex(&obj, TRUE, FALSE);
}


BOOL PAbstractDictionary::SetAt(PINDEX index, PObject * val)
{
  return SetAt(PScalarKey(index), val);
}


PObject * PAbstractDictionary::GetAt(PINDEX index) const
{
  return GetAt(PScalarKey(index));
}
 
 
BOOL PAbstractDictionary::SetDataAt(PINDEX index, PObject * val)
{
  return SetAt(AbstractGetKeyAt(index), val);
}


BOOL PAbstractDictionary::Enumerate(PEnumerator func, PObject * info) const
{
  return hashTable->EnumerateElements(func, info, FALSE);
}


BOOL PAbstractDictionary::EnumerateKeys(PEnumerator func, PObject * info) const
{
  return hashTable->EnumerateElements(func, info, TRUE);
}


BOOL PAbstractDictionary::SetAt(const PObject & key, PObject * obj)
{
  if (obj == NULL) {
    obj = hashTable->RemoveElement(key);
    if (obj != NULL && reference->deleteObjects)
      delete obj;
    reference->size--;
  }
  else {
    PHashTableElement * element = hashTable->GetElementAt(key);
    if (element == NULL) {
      hashTable->AppendElement(key.Clone(), obj);
      reference->size++;
    }
    else {
      if (reference->deleteObjects)
        delete hashTable->lastElement->data;
      hashTable->lastElement->data = obj;
    }
  }
  return TRUE;
}


PObject * PAbstractDictionary::GetAt(const PObject & key) const
{
  PHashTableElement * element = hashTable->GetElementAt(key);
  return element != NULL ? element->data : NULL;
}


PObject & PAbstractDictionary::GetRefAt(const PObject & key) const
{
  PHashTableElement * element = hashTable->GetElementAt(key);
  return *PAssertNULL(element)->data;
}


// End Of File ///////////////////////////////////////////////////////////////
