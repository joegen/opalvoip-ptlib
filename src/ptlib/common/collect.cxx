/*
 * collect.cxx
 *
 * Container Classes
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Portable Windows Library.
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#include <ptlib.h>


PDEFINE_POOL_ALLOCATOR(PListElement)
PDEFINE_POOL_ALLOCATOR(PListInfo)
PDEFINE_POOL_ALLOCATOR(PSortedListElement)
PDEFINE_POOL_ALLOCATOR(PHashTableElement)


#define new PNEW
#undef  __CLASS__
#define __CLASS__ GetClass()


///////////////////////////////////////////////////////////////////////////////

void PCollection::PrintOn(ostream &strm) const
{
  char separator = strm.fill();
  int width = (int)strm.width();
  for (PINDEX  i = 0; i < GetSize(); i++) {
    if (i > 0 && separator != ' ')
      strm << separator;
    PObject * obj = GetAt(i);
    if (obj != NULL) {
      if (separator != ' ')
        strm.width(width);
      strm << *obj;
    }
  }
  if (separator == '\n')
    strm << '\n';
}


void PCollection::RemoveAll()
{
  while (GetSize() > 0)
    RemoveAt(0);
}


///////////////////////////////////////////////////////////////////////////////

void PArrayObjects::CopyContents(const PArrayObjects & array)
{
  theArray = array.theArray;
}


void PArrayObjects::DestroyContents()
{
  if (reference->deleteObjects && theArray != NULL) {
    for (PINDEX i = 0; i < theArray->GetSize(); i++) {
      if ((*theArray)[i] != NULL)
        delete (*theArray)[i];
    }
  }
  delete theArray;
  theArray = NULL;
}


void PArrayObjects::RemoveAll()
{
  SetSize(0);
}


void PArrayObjects::CloneContents(const PArrayObjects * array)
{
  PBaseArray<PObject *> & oldArray = *array->theArray;
  theArray = new PBaseArray<PObject *>(oldArray.GetSize());
  for (PINDEX i = 0; i < GetSize(); i++) {
    PObject * ptr = oldArray[i];
    if (ptr != NULL)
      SetAt(i, ptr->Clone());
  }
}


PObject::Comparison PArrayObjects::Compare(const PObject & obj) const
{
  PAssert(PIsDescendant(&obj, PArrayObjects), PInvalidCast);
  const PArrayObjects & other = (const PArrayObjects &)obj;
  PINDEX i;
  for (i = 0; i < GetSize(); i++) {
    if (i >= other.GetSize() || *(*theArray)[i] < *(*other.theArray)[i])
      return LessThan;
    if (*(*theArray)[i] > *(*other.theArray)[i])
      return GreaterThan;
  }
  return i < other.GetSize() ? GreaterThan : EqualTo;
}


PINDEX PArrayObjects::GetSize() const
{
  return theArray->GetSize();
}


PBoolean PArrayObjects::SetSize(PINDEX newSize)
{
  PINDEX sz = theArray->GetSize();
  if (reference->deleteObjects && sz > 0) {
    for (PINDEX i = sz; i > newSize; i--) {
      PObject * obj = theArray->GetAt(i-1);
      if (obj != NULL)
        delete obj;
    }
  }
  return theArray->SetSize(newSize);
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


PBoolean PArrayObjects::Remove(const PObject * obj)
{
  PINDEX i = GetObjectsIndex(obj);
  if (i == P_MAX_INDEX)
    return PFalse;
  RemoveAt(i);
  return PTrue;
}


PObject * PArrayObjects::GetAt(PINDEX index) const
{
  return (*theArray)[index];
}


PBoolean PArrayObjects::SetAt(PINDEX index, PObject * obj)
{
  if (!theArray->SetMinSize(index+1))
    return PFalse;
  PObject * oldObj = theArray->GetAt(index);
  if (oldObj != NULL && reference->deleteObjects)
    delete oldObj;
  (*theArray)[index] = obj;
  return PTrue;
}


PINDEX PArrayObjects::InsertAt(PINDEX index, PObject * obj)
{
  PINDEX i = GetSize();
  SetSize(i+1);
  for (; i > index; i--)
    (*theArray)[i] = (*theArray)[i-1];
  (*theArray)[index] = obj;
  return index;
}


PObject * PArrayObjects::RemoveAt(PINDEX index)
{
  PObject * obj = (*theArray)[index];

  PINDEX size = GetSize()-1;
  PINDEX i;
  for (i = index; i < size; i++)
    (*theArray)[i] = (*theArray)[i+1];
  (*theArray)[i] = NULL;

  SetSize(size);

  if (obj != NULL && reference->deleteObjects) {
    delete obj;
    obj = NULL;
  }

  return obj;
}


PINDEX PArrayObjects::GetObjectsIndex(const PObject * obj) const
{
  for (PINDEX i = 0; i < GetSize(); i++) {
    if ((*theArray)[i] == obj)
      return i;
  }
  return P_MAX_INDEX;
}


PINDEX PArrayObjects::GetValuesIndex(const PObject & obj) const
{
  for (PINDEX i = 0; i < GetSize(); i++) {
    PObject * elmt = (*theArray)[i];
    if (elmt != NULL && *elmt == obj)
      return i;
  }
  return P_MAX_INDEX;
}


///////////////////////////////////////////////////////////////////////////////

void PAbstractList::DestroyContents()
{
  RemoveAll();
  delete info;
  info = NULL;
}


void PAbstractList::CopyContents(const PAbstractList & list)
{
  info = list.info;
}


void PAbstractList::CloneContents(const PAbstractList * list)
{
  Element * element = list->info->head;

  info = new PListInfo;
  PAssert(info != NULL, POutOfMemory);

  while (element != NULL) {
    Element * newElement = new Element(element->data->Clone());

    if (info->head == NULL)
      info->head = info->tail = newElement;
    else {
      newElement->prev = info->tail;
      info->tail->next = newElement;
      info->tail = newElement;
    }

    element = element->next;
  }
}


PObject::Comparison PAbstractList::Compare(const PObject & obj) const
{
  PAssert(PIsDescendant(&obj, PAbstractList), PInvalidCast);
  Element * elmt1 = info->head;
  Element * elmt2 = ((const PAbstractList &)obj).info->head;
  while (elmt1 != NULL || elmt2 != NULL) {
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


PBoolean PAbstractList::SetSize(PINDEX)
{
  return PTrue;
}


PINDEX PAbstractList::Append(PObject * obj)
{
  if (PAssertNULL(obj) == NULL)
    return P_MAX_INDEX;

  Element * element = new Element(obj);
  if (info->tail != NULL)
    info->tail->next = element;
  element->prev = info->tail;
  element->next = NULL;
  if (info->head == NULL)
    info->head = element;
  info->tail = element;

  return reference->size++;
}


void PAbstractList::Prepend(PObject * obj)
{
  if (PAssertNULL(obj) == NULL)
    return;

  Element * element = new Element(obj);
  if (info->head != NULL)
    info->head->prev = element;
  element->prev = NULL;
  element->next = info->head;
  if (info->tail == NULL)
    info->tail = element;
  info->head = element;
  ++reference->size;
}


PINDEX PAbstractList::Insert(const PObject & before, PObject * obj)
{
  PINDEX index;
  InsertElement(FindElement(before, &index), obj);
  return index;
}


PINDEX PAbstractList::InsertAt(PINDEX index, PObject * obj)
{
  if (PAssertNULL(obj) == NULL)
    return P_MAX_INDEX;
  
  if (index >= GetSize())
    return Append(obj);

  Element * element = FindElement(index);
  if (!PAssert(element != NULL, PInvalidArrayIndex))
    return P_MAX_INDEX;

  Element * newElement = new Element(obj);
  if (element->prev != NULL)
    element->prev->next = newElement;
  else
    info->head = newElement;
  newElement->prev = element->prev;
  newElement->next = element;
  element->prev = newElement;

  reference->size++;
  return index;
}


PBoolean PAbstractList::Remove(const PObject * obj)
{
  if (PAssertNULL(info) == NULL)
    return false;

  Element * elmt = info->head;
  while (elmt != NULL) {
    if (elmt->data == obj) {
      RemoveElement(elmt);
      return true;
    }
    elmt = elmt->next;
  }
  
  return false;
}


PObject * PAbstractList::RemoveAt(PINDEX index)
{
  if (PAssertNULL(info) == NULL)
    return NULL;

  Element * element = FindElement(index);
  return PAssert(element != NULL, PInvalidArrayIndex) ? RemoveElement(element) : NULL;
}


void PAbstractList::InsertElement(PListElement * element, PObject * obj)
{
  if (element == NULL)
    Append(obj);

  Element * newElement = new Element(obj);
  if (element->prev != NULL)
    element->prev->next = newElement;
  else
    info->head = newElement;
  newElement->prev = element->prev;
  newElement->next = element;
  element->prev = newElement;
  ++reference->size;
}


PObject * PAbstractList::RemoveElement(PListElement * elmt)
{
  if (PAssertNULL(info) == NULL)
    return NULL;
  
  if (elmt == NULL)
    return NULL;

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

  if((reference == NULL) || (reference->size == 0)){
    PAssertAlways("reference is null or reference->size == 0");
    return NULL;
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
  Element * element = FindElement(index);
  return element != NULL ? element->data : (PObject *)NULL;
}


PObject & PAbstractList::GetReferenceAt(PINDEX index) const
{
  Element * element = FindElement(index);
  if (element != NULL)
    return *element->data;

  PAssertAlways(PInvalidArrayIndex);
  return *(PObject *)NULL;
}

PBoolean PAbstractList::SetAt(PINDEX index, PObject * val)
{
  Element * element = FindElement(index);
  if (element == NULL)
    return false;
  element->data = val;
  return true;
}

PBoolean PAbstractList::ReplaceAt(PINDEX index, PObject * val)
{
  Element * element = FindElement(index);
  if (element == NULL)
    return false;
  
  if (element->data != NULL && reference->deleteObjects)
    delete element->data;

  element->data = val;
  return true;
}

PINDEX PAbstractList::GetObjectsIndex(const PObject * obj) const
{
  PINDEX index = 0;
  Element * element = info->head;

  while (element != NULL) {
    if (element->data == obj) 
      return index;
    element = element->next;
    index++;
  }

  return P_MAX_INDEX;
}


PINDEX PAbstractList::GetValuesIndex(const PObject & obj) const
{
  PINDEX index;
  return FindElement(obj, &index) != NULL ? index : P_MAX_INDEX;
}


PListElement * PAbstractList::FindElement(PINDEX index) const
{
  if (index >= GetSize())
    return NULL;

  Element * lastElement;
  PINDEX lastIndex;
  if (index < GetSize()/2) {
    lastIndex = 0;
    lastElement = info->head;
  }
  else {
    lastIndex = GetSize()-1;
    lastElement = info->tail;
  }

  while (lastIndex < index) {
    lastElement = lastElement->next;
    ++lastIndex;
  }

  while (lastIndex > index) {
    lastElement = lastElement->prev;
    --lastIndex;
  }

  return lastElement;
}


PListElement * PAbstractList::FindElement(const PObject & obj, PINDEX * indexPtr) const
{
  if (PAssertNULL(info) == NULL)
    return NULL;

  PINDEX index = 0;
  Element * element = info->head;
  while (element != NULL) {
    if (*element->data == obj)
      break;
    element = element->next;
    index++;
  }

  if (indexPtr != NULL)
    *indexPtr = index;
  return element;
}


PListElement::PListElement(PObject * theData)
{
  next = prev = NULL;
  data = theData;
}


///////////////////////////////////////////////////////////////////////////////

PAbstractSortedList::PAbstractSortedList()
  : m_root(&nil)
{
  nil.m_parent = nil.m_left = nil.m_right = &nil;
  nil.m_subTreeSize = 0;
  nil.m_colour = PSortedListElement::Black;
  nil.m_data = NULL;
}


void PAbstractSortedList::DestroyContents()
{
  RemoveAll();
}


void PAbstractSortedList::CopyContents(const PAbstractSortedList & list)
{
  m_root = list.m_root;
}


void PAbstractSortedList::CloneContents(const PAbstractSortedList * list)
{
  reference->size = 0;

  // Have to do this in this manner rather than just doing a for() loop
  // as "this" and "list" may be the same object and we just changed info in
  // "this" so we need to use the info in "list" saved previously.
  PSortedListElement * element = OrderSelect(list->m_root, 1);
  while (element != &nil) {
    Append(element->m_data->Clone());
    element = Successor(element);
  }
}


PBoolean PAbstractSortedList::SetSize(PINDEX)
{
  return PTrue;
}


PObject::Comparison PAbstractSortedList::Compare(const PObject & obj) const
{
  PAssert(PIsDescendant(&obj, PAbstractSortedList), PInvalidCast);
  PSortedListElement * elmt1 = m_root;
  while (elmt1->m_left != &nil)
    elmt1 = elmt1->m_left;

  PSortedListElement * elmt2 = dynamic_cast<const PAbstractSortedList &>(obj).m_root;
  while (elmt2->m_left != &nil)
    elmt2 = elmt2->m_left;

  while (elmt1 != &nil && elmt2 != &nil) {
    if (elmt1 == &nil)
      return LessThan;
    if (elmt2 == &nil)
      return GreaterThan;
    if (*elmt1->m_data < *elmt2->m_data)
      return LessThan;
    if (*elmt1->m_data > *elmt2->m_data)
      return GreaterThan;
    elmt1 = Successor(elmt1);
    elmt2 = Successor(elmt2);
  }
  return EqualTo;
}


PINDEX PAbstractSortedList::Append(PObject * obj)
{
  if (PAssertNULL(obj) == NULL)
    return P_MAX_INDEX;

  PSortedListElement * z = new PSortedListElement;
  z->m_parent = z->m_left = z->m_right = &nil;
  z->m_colour = PSortedListElement::Black;
  z->m_subTreeSize = 1;
  z->m_data = obj;

  PSortedListElement * x = m_root;
  PSortedListElement * y = &nil;
  while (x != &nil) {
    x->m_subTreeSize++;
    y = x;
    x = *z->m_data < *x->m_data ? x->m_left : x->m_right;
  }
  z->m_parent = y;
  if (y == &nil)
    m_root = z;
  else if (*z->m_data < *y->m_data)
    y->m_left = z;
  else
    y->m_right = z;

  PSortedListElement * lastElement = x = z;
  PINDEX lastIndex;

  x->m_colour = PSortedListElement::Red;
  while (x != m_root && x->m_parent->m_colour == PSortedListElement::Red) {
    if (x->m_parent == x->m_parent->m_parent->m_left) {
      y = x->m_parent->m_parent->m_right;
      if (y->m_colour == PSortedListElement::Red) {
        x->m_parent->m_colour = PSortedListElement::Black;
        y->m_colour = PSortedListElement::Black;
        x->m_parent->m_parent->m_colour = PSortedListElement::Red;
        x = x->m_parent->m_parent;
      }
      else {
        if (x == x->m_parent->m_right) {
          x = x->m_parent;
          LeftRotate(x);
        }
        x->m_parent->m_colour = PSortedListElement::Black;
        x->m_parent->m_parent->m_colour = PSortedListElement::Red;
        RightRotate(x->m_parent->m_parent);
      }
    }
    else {
      y = x->m_parent->m_parent->m_left;
      if (y->m_colour == PSortedListElement::Red) {
        x->m_parent->m_colour = PSortedListElement::Black;
        y->m_colour = PSortedListElement::Black;
        x->m_parent->m_parent->m_colour = PSortedListElement::Red;
        x = x->m_parent->m_parent;
      }
      else {
        if (x == x->m_parent->m_left) {
          x = x->m_parent;
          RightRotate(x);
        }
        x->m_parent->m_colour = PSortedListElement::Black;
        x->m_parent->m_parent->m_colour = PSortedListElement::Red;
        LeftRotate(x->m_parent->m_parent);
      }
    }
  }

  m_root->m_colour = PSortedListElement::Black;

  x = lastElement;
  lastIndex = x->m_left->m_subTreeSize;
  while (x != m_root) {
    if (x != x->m_parent->m_left)
      lastIndex += x->m_parent->m_left->m_subTreeSize+1;
    x = x->m_parent;
  }

  reference->size++;
  return lastIndex;
}


PBoolean PAbstractSortedList::Remove(const PObject * obj)
{
  PSortedListElement * element = FindElement(obj, NULL);
  if (element == NULL)
    return false;

  RemoveElement(element);
  return true;
}


PObject * PAbstractSortedList::RemoveAt(PINDEX index)
{
  PSortedListElement * node = OrderSelect(m_root, index+1);
  if (node == &nil)
    return NULL;

  PObject * data = node->m_data;
  RemoveElement(node);
  return reference->deleteObjects ? (PObject *)NULL : data;
}


void PAbstractSortedList::RemoveAll()
{
  if (m_root != &nil) {
    DeleteSubTrees(m_root, reference->deleteObjects);
    delete m_root;
    m_root = &nil;
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


PBoolean PAbstractSortedList::SetAt(PINDEX, PObject *)
{
  return PFalse;
}


PObject * PAbstractSortedList::GetAt(PINDEX index) const
{
  if (index >= GetSize())
    return NULL;

  return OrderSelect(m_root, index+1)->m_data;
}


PINDEX PAbstractSortedList::GetObjectsIndex(const PObject * obj) const
{
  PINDEX index;
  return FindElement(obj, &index) != NULL ? index : P_MAX_INDEX;
}


PSortedListElement * PAbstractSortedList::FindElement(const PObject * obj, PINDEX * index) const
{
  PSortedListElement * element = NULL;
  PINDEX pos = ValueSelect(m_root, *obj, element);
  if (pos == P_MAX_INDEX)
    return NULL;

  if (element->m_data != obj) {
    PINDEX savePos = pos;
    PSortedListElement * saveElmt = element;
    while (element->m_data != obj && (element = Predecessor(element)) != &nil && *obj == *element->m_data)
      pos--;
    if (element->m_data != obj) {
      pos = savePos;
      element = saveElmt;
      while (element->m_data != obj && (element = Successor(element)) != &nil && *obj == *element->m_data)
        pos++;
      if (element->m_data != obj)
        return NULL;
    }
  }

  if (index != NULL)
    *index = pos;

  return element;
}


PINDEX PAbstractSortedList::GetValuesIndex(const PObject & obj) const
{
  PINDEX index;
  return FindElement(obj, &index) != NULL ? index : P_MAX_INDEX;
}


PSortedListElement * PAbstractSortedList::FindElement(const PObject & obj, PINDEX * index) const
{
  PSortedListElement * element;
  PINDEX pos = ValueSelect(m_root, obj, element);
  if (pos == P_MAX_INDEX)
    return NULL;

  PSortedListElement * prev;
  while ((prev = Predecessor(element)) != &nil && prev->m_data->Compare(obj) == EqualTo) {
    element = prev;
    pos--;
  }

  if (index != NULL)
    *index = pos;

  return element;
}


void PAbstractSortedList::RemoveElement(PSortedListElement * node)
{
  // Don't try an remove one of the special leaf nodes!
  if (PAssertNULL(node) == &nil)
    return;

  if (node->m_data != NULL && reference->deleteObjects)
    delete node->m_data;

  PSortedListElement * y = node->m_left == &nil || node->m_right == &nil ? node : Successor(node);

  PSortedListElement * t = y;
  while (t != &nil) {
    t->m_subTreeSize--;
    t = t->m_parent;
  }

  PSortedListElement * x = y->m_left != &nil ? y->m_left : y->m_right;
  x->m_parent = y->m_parent;

  if (y->m_parent == &nil)
    m_root = x;
  else if (y == y->m_parent->m_left)
    y->m_parent->m_left = x;
  else
    y->m_parent->m_right = x;

  if (y != node)
    node->m_data = y->m_data;

  if (y->m_colour == PSortedListElement::Black) {
    while (x != m_root && x->m_colour == PSortedListElement::Black) {
      if (x == x->m_parent->m_left) {
        PSortedListElement * w = x->m_parent->m_right;
        if (w->m_colour == PSortedListElement::Red) {
          w->m_colour = PSortedListElement::Black;
          x->m_parent->m_colour = PSortedListElement::Red;
          LeftRotate(x->m_parent);
          w = x->m_parent->m_right;
        }
        if (w->m_left->m_colour == PSortedListElement::Black && w->m_right->m_colour == PSortedListElement::Black) {
          w->m_colour = PSortedListElement::Red;
          x = x->m_parent;
        }
        else {
          if (w->m_right->m_colour == PSortedListElement::Black) {
            w->m_left->m_colour = PSortedListElement::Black;
            w->m_colour = PSortedListElement::Red;
            RightRotate(w);
            w = x->m_parent->m_right;
          }
          w->m_colour = x->m_parent->m_colour;
          x->m_parent->m_colour = PSortedListElement::Black;
          w->m_right->m_colour = PSortedListElement::Black;
          LeftRotate(x->m_parent);
          x = m_root;
        }
      }
      else {
        PSortedListElement * w = x->m_parent->m_left;
        if (w->m_colour == PSortedListElement::Red) {
          w->m_colour = PSortedListElement::Black;
          x->m_parent->m_colour = PSortedListElement::Red;
          RightRotate(x->m_parent);
          w = x->m_parent->m_left;
        }
        if (w->m_right->m_colour == PSortedListElement::Black && w->m_left->m_colour == PSortedListElement::Black) {
          w->m_colour = PSortedListElement::Red;
          x = x->m_parent;
        }
        else {
          if (w->m_left->m_colour == PSortedListElement::Black) {
            w->m_right->m_colour = PSortedListElement::Black;
            w->m_colour = PSortedListElement::Red;
            LeftRotate(w);
            w = x->m_parent->m_left;
          }
          w->m_colour = x->m_parent->m_colour;
          x->m_parent->m_colour = PSortedListElement::Black;
          w->m_left->m_colour = PSortedListElement::Black;
          RightRotate(x->m_parent);
          x = m_root;
        }
      }
    }
    x->m_colour = PSortedListElement::Black;
  }

  delete y;

  reference->size--;
}


void PAbstractSortedList::LeftRotate(PSortedListElement * node)
{
  PSortedListElement * pivot = node->m_right;
  node->m_right = pivot->m_left;
  if (pivot->m_left != &nil)
    pivot->m_left->m_parent = node;
  pivot->m_parent = node->m_parent;
  if (node->m_parent == &nil)
    m_root = pivot;
  else if (node == node->m_parent->m_left)
    node->m_parent->m_left = pivot;
  else
    node->m_parent->m_right = pivot;
  pivot->m_left = node;
  node->m_parent = pivot;
  pivot->m_subTreeSize = node->m_subTreeSize;
  node->m_subTreeSize = node->m_left->m_subTreeSize + node->m_right->m_subTreeSize + 1;
}


void PAbstractSortedList::RightRotate(PSortedListElement * node)
{
  PSortedListElement * pivot = node->m_left;
  node->m_left = pivot->m_right;
  if (pivot->m_right != &nil)
    pivot->m_right->m_parent = node;
  pivot->m_parent = node->m_parent;
  if (node->m_parent == &nil)
    m_root = pivot;
  else if (node == node->m_parent->m_right)
    node->m_parent->m_right = pivot;
  else
    node->m_parent->m_left = pivot;
  pivot->m_right = node;
  node->m_parent = pivot;
  pivot->m_subTreeSize = node->m_subTreeSize;
  node->m_subTreeSize = node->m_left->m_subTreeSize + node->m_right->m_subTreeSize + 1;
}


PSortedListElement * PAbstractSortedList::Successor(PSortedListElement * node) const
{
  PSortedListElement * next;
  if (node->m_right != &nil) {
    next = node->m_right;
    while (next->m_left != &nil)
      next = next->m_left;
  }
  else {
    next = node->m_parent;
    while (next != &nil && node == next->m_right) {
      node = next;
      next = node->m_parent;
    }
  }
  return next;
}


PSortedListElement * PAbstractSortedList::Predecessor(PSortedListElement * node) const
{
  PSortedListElement * pred;
  if (node->m_left != &nil) {
    pred = node->m_left;
    while (pred->m_right != &nil)
      pred = pred->m_right;
  }
  else {
    pred = node->m_parent;
    while (pred != &nil && node == pred->m_left) {
      node = pred;
      pred = node->m_parent;
    }
  }
  return pred;
}


PSortedListElement * PAbstractSortedList::OrderSelect(PSortedListElement * node, PINDEX index) const
{
  PINDEX r = node->m_left->m_subTreeSize+1;
  if (index == r)
    return node;

  if (index < r) {
    if (node->m_left != &nil)
      return OrderSelect(node->m_left, index);
  }
  else {
    if (node->m_right != &nil)
      return OrderSelect(node->m_right, index - r);
  }

  PAssertAlways2("PAbstractSortedList::Element", "Order select failed!");
  return const_cast<PSortedListElement *>(&nil);
}


PINDEX PAbstractSortedList::ValueSelect(PSortedListElement * node, const PObject & obj, PSortedListElement * & selectedElement) const
{
  if (node != &nil) {
    switch (node->m_data->Compare(obj)) {
      case PObject::LessThan :
      {
        PINDEX index = ValueSelect(node->m_right, obj, selectedElement);
        if (index != P_MAX_INDEX)
          return node->m_left->m_subTreeSize + index + 1;
        break;
      }

      case PObject::GreaterThan :
        return ValueSelect(node->m_left, obj, selectedElement);

      default :
        selectedElement = node;
        return node->m_left->m_subTreeSize;
    }
  }

  return P_MAX_INDEX;
}


void PAbstractSortedList::DeleteSubTrees(PSortedListElement * node, PBoolean deleteObject)
{
  if (node->m_left != &nil) {
    DeleteSubTrees(node->m_left, deleteObject);
    delete node->m_left;
    node->m_left = &nil;
  }
  if (node->m_right != &nil) {
    DeleteSubTrees(node->m_right, deleteObject);
    delete node->m_right;
    node->m_right = &nil;
  }
  if (deleteObject) {
    delete node->m_data;
    node->m_data = NULL;
  }
}


///////////////////////////////////////////////////////////////////////////////

PObject * POrdinalKey::Clone() const
{
  return new POrdinalKey(theKey);
}


PObject::Comparison POrdinalKey::Compare(const PObject & obj) const
{
  PAssert(PIsDescendant(&obj, POrdinalKey), PInvalidCast);
  const POrdinalKey & other = (const POrdinalKey &)obj;
  
  if (theKey < other.theKey)
    return LessThan;

  if (theKey > other.theKey)
    return GreaterThan;

  return EqualTo;
}


PINDEX POrdinalKey::HashFunction() const
{
  return PABSINDEX(theKey)%23;
}


void POrdinalKey::PrintOn(ostream & strm) const
{
  strm << theKey;
}


///////////////////////////////////////////////////////////////////////////////

void PHashTableInfo::DestroyContents()
{
  for (PINDEX i = 0; i < GetSize(); i++) {
    Element * list = GetAt(i);
    if (list != NULL) {
      Element * elmt = list;
      do {
        Element * nextElmt = elmt->next;
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


PINDEX PHashTableInfo::AppendElement(PObject * key, PObject * data)
{
  PINDEX bucket = PAssertNULL(key)->HashFunction();
  Element * list = GetAt(bucket);
  Element * element = new Element;
  PAssert(element != NULL, POutOfMemory);
  element->key = key;
  element->data = data;
  element->bucket = bucket;
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
  return bucket;
}


PObject * PHashTableInfo::RemoveElement(const PObject & key)
{
  PObject * obj = NULL;
  Element * element = GetElementAt(key);
  if (element != NULL) {
    if (element == element->prev)
      SetAt(element->bucket, NULL);
    else {
      element->prev->next = element->next;
      element->next->prev = element->prev;
      SetAt(element->bucket, element->next);
    }
    obj = element->data;
    if (deleteKeys)
      delete element->key;
    delete element;
  }
  return obj;
}


PHashTableElement * PHashTableInfo::GetElementAt(PINDEX index)
{
  PHashTableElement * element;
  PINDEX bucket = 0;
  while ((element = GetAt(bucket)) == NULL) {
    if (bucket >= GetSize())
      return NULL;
    bucket++;
  }

  for (PINDEX i = 0; i < index; ++i) {
    if (element->next != operator[](bucket))
      element = element->next;
    else {
      do {
        if (++bucket >= GetSize())
          return NULL;
      } while ((element = operator[](bucket)) == NULL);
    }
  }

  return element;
}


PHashTableElement * PHashTableInfo::GetElementAt(const PObject & key)
{
  Element * list = GetAt(key.HashFunction());
  if (list != NULL) {
    Element * element = list;
    do {
      if (*element->key == key) 
        return element;
      element = element->next;
    } while (element != list);
  }
  return NULL;
}


PINDEX PHashTableInfo::GetElementsIndex(const PObject * obj, PBoolean byValue, PBoolean keys) const
{
  PINDEX index = 0;
  for (PINDEX i = 0; i < GetSize(); i++) {
    Element * list = operator[](i);
    if (list != NULL) {
      Element * element = list;
      do {
        PObject * keydata = keys ? element->key : element->data;
        if (byValue ? (*keydata == *obj) : (keydata == obj))
          return index;
        element = element->next;
        index++;
      } while (element != list);
    }
  }
  return P_MAX_INDEX;
}


PHashTableElement * PHashTableInfo::NextElement(PHashTableElement * element) const
{
  PINDEX bucket = element->bucket;
  if (element->next != operator[](bucket))
    element = element->next;
  else {
    do {
      if (++bucket >= GetSize())
        return NULL;
    } while ((element = operator[](bucket)) == NULL);
  }
  return element;
}


PHashTableElement * PHashTableInfo::PrevElement(PHashTableElement * element) const
{
  PINDEX bucket = element->bucket;
  if (element != operator[](bucket))
    element = element->prev;
  else {
    do {
      if (bucket-- == 0)
        return NULL;
    } while ((element = operator[](bucket)) == NULL);
    element = element->prev;
  }
  return element;
}


///////////////////////////////////////////////////////////////////////////////

PHashTable::PHashTable()
  : hashTable(new PHashTable::Table)
{
  PAssert(hashTable != NULL, POutOfMemory);
}


void PHashTable::DestroyContents()
{
  if (hashTable != NULL) {
    hashTable->reference->deleteObjects = reference->deleteObjects;
    delete hashTable;
    hashTable = NULL;
  }
}


void PHashTable::CopyContents(const PHashTable & hash)
{
  hashTable = hash.hashTable;
}

  
void PHashTable::CloneContents(const PHashTable * hash)
{
  PINDEX sz = PAssertNULL(hash)->GetSize();
  PHashTable::Table * original = PAssertNULL(hash->hashTable);

  hashTable = new PHashTable::Table(original->GetSize());
  PAssert(hashTable != NULL, POutOfMemory);
  hashTable->deleteKeys = original->deleteKeys;

  for (PINDEX i = 0; i < sz; i++) {
    Element * lastElement = original->GetElementAt(i);
    PObject * data = lastElement->data;
    if (data != NULL)
      data = data->Clone();
    hashTable->AppendElement(lastElement->key->Clone(), data);
  }
}


PObject::Comparison PHashTable::Compare(const PObject & obj) const
{
  PAssert(PIsDescendant(&obj, PHashTable), PInvalidCast);
  return reference != ((const PHashTable &)obj).reference
                                                      ? GreaterThan : EqualTo;
}


PBoolean PHashTable::SetSize(PINDEX)
{
  return PTrue;
}


PObject & PHashTable::AbstractGetDataAt(PINDEX index) const
{
  Element * lastElement = hashTable->GetElementAt(index);
  PAssert(lastElement != NULL, PInvalidArrayIndex);
  return *lastElement->data;
}


const PObject & PHashTable::AbstractGetKeyAt(PINDEX index) const
{
  Element * lastElement = hashTable->GetElementAt(index);
  PAssert(lastElement != NULL, PInvalidArrayIndex);
  return *lastElement->key;
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
  if (AbstractContains(*obj)) {
    if (reference->deleteObjects)
      delete obj;
    return P_MAX_INDEX;
  }

  reference->size++;
  return hashTable->AppendElement(obj, NULL);
}


PINDEX PAbstractSet::Insert(const PObject &, PObject * obj)
{
  return Append(obj);
}


PINDEX PAbstractSet::InsertAt(PINDEX, PObject * obj)
{
  return Append(obj);
}


PBoolean PAbstractSet::Remove(const PObject * obj)
{
  if (PAssertNULL(obj) == NULL)
    return PFalse;

  if (hashTable->GetElementAt(*obj) == NULL)
    return PFalse;

  hashTable->deleteKeys = hashTable->reference->deleteObjects = reference->deleteObjects;
  hashTable->RemoveElement(*obj);
  reference->size--;
  return PTrue;
}


PObject * PAbstractSet::RemoveAt(PINDEX index)
{
  Element * lastElement = hashTable->GetElementAt(index);
  if (lastElement == NULL)
    return NULL;

  PObject * obj = lastElement->key;
  hashTable->deleteKeys = hashTable->reference->deleteObjects = reference->deleteObjects;
  hashTable->RemoveElement(*obj);
  reference->size--;
  return obj;
}


PINDEX PAbstractSet::GetObjectsIndex(const PObject * obj) const
{
  return hashTable->GetElementsIndex(obj, PFalse, PTrue);
}


PINDEX PAbstractSet::GetValuesIndex(const PObject & obj) const
{
  return hashTable->GetElementsIndex(&obj, PTrue, PTrue);
}


PObject * PAbstractSet::GetAt(PINDEX index) const
{
  return (PObject *)&AbstractGetKeyAt(index);
}


PBoolean PAbstractSet::SetAt(PINDEX, PObject * obj)
{
  return Append(obj);
}


bool PAbstractSet::Union(const PAbstractSet & set)
{
  bool something = false;
  for (PINDEX i = 0; i < set.GetSize(); ++i) {
    const PObject & obj = set.AbstractGetKeyAt(i);
    if (!AbstractContains(obj)) {
      something = true;
      Append(obj.Clone());
    }
  }
  return something;
}


bool PAbstractSet::Intersection(const PAbstractSet & set1,
                                const PAbstractSet & set2,
                                PAbstractSet * intersection)
{
  bool something = false;
  for (PINDEX i = 0; i < set1.GetSize(); ++i) {
    const PObject & obj = set1.AbstractGetKeyAt(i);
    if (set2.AbstractContains(obj)) {
      something = true;
      if (intersection == NULL)
        break;
      intersection->Append(obj.Clone());
    }
  }
  return something;
}


///////////////////////////////////////////////////////////////////////////////

PINDEX PAbstractDictionary::Append(PObject *)
{
  PAssertAlways(PUnimplementedFunction);
  return 0;
}


PINDEX PAbstractDictionary::Insert(const PObject & before, PObject * obj)
{
  AbstractSetAt(before, obj);
  return 0;
}


PINDEX PAbstractDictionary::InsertAt(PINDEX index, PObject * obj)
{
  AbstractSetAt(AbstractGetKeyAt(index), obj);
  return index;
}
 
 
PBoolean PAbstractDictionary::Remove(const PObject * obj)
{
  PINDEX idx = GetObjectsIndex(obj);
  if (idx == P_MAX_INDEX)
    return PFalse;

  RemoveAt(idx);
  return PTrue;
}


PObject * PAbstractDictionary::RemoveAt(PINDEX index)
{
  PObject & obj = AbstractGetDataAt(index);
  AbstractSetAt(AbstractGetKeyAt(index), NULL);
  return &obj;
}


PINDEX PAbstractDictionary::GetObjectsIndex(const PObject * obj) const
{
  return hashTable->GetElementsIndex(obj, PFalse, PFalse);
}


PINDEX PAbstractDictionary::GetValuesIndex(const PObject & obj) const
{
  return hashTable->GetElementsIndex(&obj, PTrue, PFalse);
}


PBoolean PAbstractDictionary::SetAt(PINDEX index, PObject * val)
{
  return AbstractSetAt(AbstractGetKeyAt(index), val);
}


PObject * PAbstractDictionary::GetAt(PINDEX index) const
{
  Element * element = hashTable->GetElementAt(index);
  return PAssert(element != NULL, PInvalidArrayIndex) ? element->data : NULL;
}
 
 
PBoolean PAbstractDictionary::SetDataAt(PINDEX index, PObject * val)
{
  return AbstractSetAt(AbstractGetKeyAt(index), val);
}


PObject * PAbstractDictionary::AbstractSetAt(const PObject & key, PObject * obj)
{
  if (obj == NULL) {
    obj = hashTable->RemoveElement(key);
    if (obj != NULL) {
      reference->size--;
      if (reference->deleteObjects) {
        delete obj;
        obj = NULL;
      }
    }
  }
  else {
    Element * element = hashTable->GetElementAt(key);
    if (element == NULL) {
      hashTable->AppendElement(key.Clone(), obj);
      reference->size++;
    }
    else {
      if (reference->deleteObjects) 
    	delete element->data;
      element->data = obj;
    }
  }

  return obj;
}


PObject * PAbstractDictionary::AbstractGetAt(const PObject & key) const
{
  Element * element = hashTable->GetElementAt(key);
  return element != NULL ? element->data : (PObject *)NULL;
}


PObject & PAbstractDictionary::GetRefAt(const PObject & key) const
{
  Element * element = hashTable->GetElementAt(key);
  return *PAssertNULL(element)->data;
}


void PAbstractDictionary::AbstractGetKeys(PArrayObjects & keys) const
{
  keys.SetSize(GetSize());
  PINDEX index = 0;
  for (Element * element = hashTable->GetElementAt((PINDEX)0); element != NULL; element = hashTable->NextElement(element))
    keys.SetAt(index++, element->key->Clone());
}


void PAbstractDictionary::PrintOn(ostream &strm) const
{
  char separator = strm.fill();
  if (separator == ' ')
    separator = '\n';

  for (PINDEX i = 0; i < GetSize(); i++) {
    if (i > 0)
      strm << separator;
    strm << AbstractGetKeyAt(i) << '=' << AbstractGetDataAt(i);
  }

  if (separator == '\n')
    strm << separator;
}


// End Of File ///////////////////////////////////////////////////////////////
