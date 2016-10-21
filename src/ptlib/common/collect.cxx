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
PDEFINE_POOL_ALLOCATOR(PSortedListInfo)
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
    return false;
  RemoveAt(i);
  return true;
}


PObject * PArrayObjects::GetAt(PINDEX index) const
{
  return (*theArray)[index];
}


PBoolean PArrayObjects::SetAt(PINDEX index, PObject * obj)
{
  if (!theArray->SetMinSize(index+1))
    return false;
  PObject * oldObj = theArray->GetAt(index);
  if (oldObj != obj) {
    if (oldObj != NULL && reference->deleteObjects)
      delete oldObj;
    (*theArray)[index] = obj;
  }
  return true;
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
  delete m_info;
  m_info = NULL;
}


void PAbstractList::CopyContents(const PAbstractList & list)
{
  m_info = list.m_info;
}


void PAbstractList::CloneContents(const PAbstractList * list)
{
  Element * element = list->m_info->head;

  m_info = new PListInfo;
  PAssert(m_info != NULL, POutOfMemory);

  while (element != NULL) {
    Element * newElement = new Element(element->data->Clone());

    if (m_info->head == NULL)
      m_info->head = m_info->tail = newElement;
    else {
      newElement->prev = m_info->tail;
      m_info->tail->next = newElement;
      m_info->tail = newElement;
    }

    element = element->next;
  }
}


PObject::Comparison PAbstractList::Compare(const PObject & obj) const
{
  PAssert(PIsDescendant(&obj, PAbstractList), PInvalidCast);
  Element * elmt1 = m_info->head;
  Element * elmt2 = ((const PAbstractList &)obj).m_info->head;
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
  return true;
}


PINDEX PAbstractList::Append(PObject * obj)
{
  if (PAssertNULL(obj) == NULL)
    return P_MAX_INDEX;

  Element * element = new Element(obj);
  if (m_info->tail != NULL)
    m_info->tail->next = element;
  element->prev = m_info->tail;
  element->next = NULL;
  if (m_info->head == NULL)
    m_info->head = element;
  m_info->tail = element;

  return reference->size++;
}


void PAbstractList::Prepend(PObject * obj)
{
  if (PAssertNULL(obj) == NULL)
    return;

  Element * element = new Element(obj);
  if (m_info->head != NULL)
    m_info->head->prev = element;
  element->prev = NULL;
  element->next = m_info->head;
  if (m_info->tail == NULL)
    m_info->tail = element;
  m_info->head = element;
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
    m_info->head = newElement;
  newElement->prev = element->prev;
  newElement->next = element;
  element->prev = newElement;

  reference->size++;
  return index;
}


PBoolean PAbstractList::Remove(const PObject * obj)
{
  if (PAssertNULL(m_info) == NULL)
    return false;

  Element * elmt = m_info->head;
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
  if (PAssertNULL(m_info) == NULL)
    return NULL;

  Element * element = FindElement(index);
  return PAssert(element != NULL, PInvalidArrayIndex) ? RemoveElement(element) : NULL;
}


void PAbstractList::InsertElement(PListElement * element, PObject * obj)
{
  if (element == NULL) {
    Append(obj);
    return;
  }

  Element * newElement = new Element(obj);
  if (element->prev != NULL)
    element->prev->next = newElement;
  else
    m_info->head = newElement;
  newElement->prev = element->prev;
  newElement->next = element;
  element->prev = newElement;
  ++reference->size;
}


PObject * PAbstractList::RemoveElement(PListElement * elmt)
{
  if (PAssertNULL(m_info) == NULL)
    return NULL;
  
  if (elmt == NULL)
    return NULL;

  if (elmt->prev != NULL)
    elmt->prev->next = elmt->next;
  else {
    m_info->head = elmt->next;
    if (m_info->head != NULL)
      m_info->head->prev = NULL;
  }

  if (elmt->next != NULL)
    elmt->next->prev = elmt->prev;
  else {
    m_info->tail = elmt->prev;
    if (m_info->tail != NULL)
      m_info->tail->next = NULL;
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
  Element * element = m_info->head;

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
    lastElement = m_info->head;
  }
  else {
    lastIndex = GetSize()-1;
    lastElement = m_info->tail;
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
  if (PAssertNULL(m_info) == NULL)
    return NULL;

  PINDEX index = 0;
  Element * element = m_info->head;
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

PSortedListElement::PSortedListElement(PSortedListElement * nil, PObject * obj)
{
  m_parent = m_left = m_right = nil != NULL ? nil : this;
  m_subTreeSize = nil != NULL ? 1 : 0;
  m_colour = Black;
  m_data = obj;
}


PAbstractSortedList::PAbstractSortedList()
  : m_info(new PSortedListInfo)
{
  PAssert(m_info != NULL, POutOfMemory);
}


void PAbstractSortedList::DestroyContents()
{
  RemoveAll();
  delete m_info;
}


void PAbstractSortedList::CopyContents(const PAbstractSortedList & list)
{
  m_info = list.m_info;
}


void PAbstractSortedList::CloneContents(const PAbstractSortedList * list)
{
  // Remember info for when list == this
  PSortedListInfo * otherInfo = list->m_info;

  m_info = new PSortedListInfo;
  PAssert(m_info != NULL, POutOfMemory);
  reference->size = 0;

  // Have to do this in this manner rather than just doing a for() loop
  // as "this" and "list" may be the same object and we just changed info in
  // "this" so we need to use the info in "list" saved previously.
  PSortedListElement * element = otherInfo->OrderSelect(1);
  while (element != &otherInfo->nil) {
    Append(element->m_data->Clone());
    element = otherInfo->Successor(element);
  }
}


PBoolean PAbstractSortedList::SetSize(PINDEX)
{
  return true;
}


PObject::Comparison PAbstractSortedList::Compare(const PObject & obj) const
{
  PAssert(PIsDescendant(&obj, PAbstractSortedList), PInvalidCast);
  PSortedListElement * elmt1 = m_info->m_root;
  while (elmt1->m_left != &m_info->nil)
    elmt1 = elmt1->m_left;

  PSortedListInfo * otherInfo = dynamic_cast<const PAbstractSortedList &>(obj).m_info;
  PSortedListElement * elmt2 = otherInfo->m_root;
  while (elmt2->m_left != &m_info->nil)
    elmt2 = elmt2->m_left;

  while (elmt1 != &m_info->nil && elmt2 != &m_info->nil) {
    if (elmt1 == &m_info->nil)
      return LessThan;
    if (elmt2 == &m_info->nil)
      return GreaterThan;
    if (*elmt1->m_data < *elmt2->m_data)
      return LessThan;
    if (*elmt1->m_data > *elmt2->m_data)
      return GreaterThan;
    elmt1 = m_info->Successor(elmt1);
    elmt2 = otherInfo->Successor(elmt2);
  }
  return EqualTo;
}


PINDEX PAbstractSortedList::Append(PObject * obj)
{
  if (PAssertNULL(obj) == NULL)
    return P_MAX_INDEX;

  PSortedListElement * z = new PSortedListElement(&m_info->nil, obj);
  PSortedListElement * x = m_info->m_root;
  PSortedListElement * y = &m_info->nil;
  while (x != &m_info->nil) {
    x->m_subTreeSize++;
    y = x;
    x = *z->m_data < *x->m_data ? x->m_left : x->m_right;
  }
  z->m_parent = y;
  if (y == &m_info->nil)
    m_info->m_root = z;
  else if (*z->m_data < *y->m_data)
    y->m_left = z;
  else
    y->m_right = z;

  PSortedListElement * lastElement = x = z;
  PINDEX lastIndex;

  x->m_colour = PSortedListElement::Red;
  while (x != m_info->m_root && x->m_parent->m_colour == PSortedListElement::Red) {
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

  m_info->m_root->m_colour = PSortedListElement::Black;

  x = lastElement;
  lastIndex = x->m_left->m_subTreeSize;
  while (x != m_info->m_root) {
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
  PSortedListElement * node = m_info->OrderSelect(index+1);
  if (node == &m_info->nil)
    return NULL;

  PObject * data = node->m_data;
  RemoveElement(node);
  return reference->deleteObjects ? (PObject *)NULL : data;
}


void PAbstractSortedList::RemoveAll()
{
  if (m_info->m_root != &m_info->nil) {
    DeleteSubTrees(m_info->m_root, reference->deleteObjects);
    delete m_info->m_root;
    m_info->m_root = &m_info->nil;
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
  return false;
}


PObject * PAbstractSortedList::GetAt(PINDEX index) const
{
  if (index >= GetSize())
    return NULL;

  return m_info->OrderSelect(index+1)->m_data;
}


PINDEX PAbstractSortedList::GetObjectsIndex(const PObject * obj) const
{
  PINDEX index;
  return FindElement(obj, &index) != NULL ? index : P_MAX_INDEX;
}


PSortedListElement * PAbstractSortedList::FindElement(const PObject * obj, PINDEX * index) const
{
  PSortedListElement * element = NULL;
  PINDEX pos = m_info->ValueSelect(*obj, element);
  if (pos == P_MAX_INDEX)
    return NULL;

  if (element->m_data != obj) {
    PINDEX savePos = pos;
    PSortedListElement * saveElmt = element;
    while (element->m_data != obj && (element = m_info->Predecessor(element)) != &m_info->nil && *obj == *element->m_data)
      pos--;
    if (element->m_data != obj) {
      pos = savePos;
      element = saveElmt;
      while (element->m_data != obj && (element = m_info->Successor(element)) != &m_info->nil && *obj == *element->m_data)
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
  PINDEX pos = m_info->ValueSelect(obj, element);
  if (pos == P_MAX_INDEX)
    return NULL;

  PSortedListElement * prev;
  while ((prev = m_info->Predecessor(element)) != &m_info->nil && prev->m_data->Compare(obj) == EqualTo) {
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
  if (PAssertNULL(node) == &m_info->nil)
    return;

  if (node->m_data != NULL && reference->deleteObjects)
    delete node->m_data;

  PSortedListElement * y = node->m_left == &m_info->nil || node->m_right == &m_info->nil ? node : m_info->Successor(node);

  PSortedListElement * t = y;
  while (t != &m_info->nil) {
    t->m_subTreeSize--;
    t = t->m_parent;
  }

  PSortedListElement * x = y->m_left != &m_info->nil ? y->m_left : y->m_right;
  x->m_parent = y->m_parent;

  if (y->m_parent == &m_info->nil)
    m_info->m_root = x;
  else if (y == y->m_parent->m_left)
    y->m_parent->m_left = x;
  else
    y->m_parent->m_right = x;

  if (y != node)
    node->m_data = y->m_data;

  if (y->m_colour == PSortedListElement::Black) {
    while (x != m_info->m_root && x->m_colour == PSortedListElement::Black) {
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
          x = m_info->m_root;
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
          x = m_info->m_root;
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
  if (pivot->m_left != &m_info->nil)
    pivot->m_left->m_parent = node;
  pivot->m_parent = node->m_parent;
  if (node->m_parent == &m_info->nil)
    m_info->m_root = pivot;
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
  if (pivot->m_right != &m_info->nil)
    pivot->m_right->m_parent = node;
  pivot->m_parent = node->m_parent;
  if (node->m_parent == &m_info->nil)
    m_info->m_root = pivot;
  else if (node == node->m_parent->m_right)
    node->m_parent->m_right = pivot;
  else
    node->m_parent->m_left = pivot;
  pivot->m_right = node;
  node->m_parent = pivot;
  pivot->m_subTreeSize = node->m_subTreeSize;
  node->m_subTreeSize = node->m_left->m_subTreeSize + node->m_right->m_subTreeSize + 1;
}


PSortedListElement * PSortedListInfo::Successor(PSortedListElement * node) const
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


PSortedListElement * PSortedListInfo::Predecessor(PSortedListElement * node) const
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


PSortedListElement * PSortedListInfo::OrderSelect(PSortedListElement * node, PINDEX index) const
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


PINDEX PSortedListInfo::ValueSelect(PSortedListElement * node, const PObject & obj, PSortedListElement * & selectedElement) const
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
  if (node->m_left != &m_info->nil) {
    DeleteSubTrees(node->m_left, deleteObject);
    delete node->m_left;
    node->m_left = &m_info->nil;
  }
  if (node->m_right != &m_info->nil) {
    DeleteSubTrees(node->m_right, deleteObject);
    delete node->m_right;
    node->m_right = &m_info->nil;
  }
  if (deleteObject) {
    delete node->m_data;
    node->m_data = NULL;
  }
}


///////////////////////////////////////////////////////////////////////////////

void PHashTableInfo::DestroyContents()
{
  for (PINDEX i = 0; i < GetSize(); i++) {
    PHashTableElement * elmt = GetAt(i).m_head;
    while (elmt != NULL) {
      PHashTableElement * nextElmt = elmt->m_next;
      if (elmt->m_data != NULL && reference->deleteObjects)
        delete elmt->m_data;
      if (deleteKeys)
        delete elmt->m_key;
      delete elmt;
      elmt = nextElmt;
    }
  }
  PAbstractArray::DestroyContents();
}


void PHashTableInfo::AppendElement(PObject * key, PObject * data PTRACE_PARAM(, PHashTable * owner))
{
  PINDEX bucket = PAssertNULL(key)->HashFunction();
  if (bucket >= GetSize())
    SetSize(bucket+1);

  PHashTableList & list = operator[](bucket);
  PHashTableElement * element = new PHashTableElement;
  PAssert(element != NULL, POutOfMemory);
  element->m_key = key;
  element->m_data = data;
  element->m_bucket = bucket;
  element->m_next = NULL;

  if (list.m_head == NULL) {
    element->m_prev = NULL;
    list.m_head = list.m_tail = element;
  }
  else {
    element->m_prev = list.m_tail;
    list.m_tail->m_next = element;
    list.m_tail =  element;
  }

#if PTRACING
  ++list.m_size;
  PINDEX totalSize = owner->GetSize();
  PTRACE_IF(1, list.m_size > 20 && list.m_size > totalSize/2, owner, "PTLib",
            "Poor hash function used, more than 50% of " << totalSize <<
            " items in same bucket for class " << owner->GetClass());
#endif
}


PObject * PHashTableInfo::RemoveElement(const PObject & key)
{
  PObject * obj = NULL;
  PHashTableElement * element = GetElementAt(key);
  if (element != NULL) {
    PHashTableList & list = operator[](element->m_bucket);
    if (element == list.m_head) {
      if (element == list.m_tail)
        list.m_head = list.m_tail = NULL;
      else {
        list.m_head = list.m_head->m_next;
        list.m_head->m_prev = NULL;
      }
    }
    else {
      if (element == list.m_tail) {
        list.m_tail = list.m_tail->m_prev;
        list.m_tail->m_next = NULL;
      }
      else {
        element->m_prev->m_next = element->m_next;
        element->m_next->m_prev = element->m_prev;
      }
    }
#if PTRACING
    --list.m_size;
#endif

    obj = element->m_data;
    if (deleteKeys)
      delete element->m_key;
    delete element;
  }
  return obj;
}


PHashTableElement * PHashTableInfo::GetElementAt(PINDEX index)
{
  PHashTableElement * element;
  PINDEX bucket = 0;
  while ((element = GetAt(bucket).m_head) == NULL) {
    if (bucket >= GetSize())
      return NULL;
    bucket++;
  }

  for (PINDEX i = 0; i < index; ++i) {
    if (element->m_next != NULL)
      element = element->m_next;
    else {
      do {
        if (++bucket >= GetSize())
          return NULL;
      } while ((element = GetAt(bucket).m_head) == NULL);
    }
  }

  return element;
}


PHashTableElement * PHashTableInfo::GetElementAt(const PObject & key)
{
  PHashTableElement * element = GetAt(key.HashFunction()).m_head;
  while (element != NULL) {
    if (*element->m_key == key) 
      return element;
    element = element->m_next;
  }
  return NULL;
}


PINDEX PHashTableInfo::GetElementsIndex(const PObject * obj, PBoolean byValue, PBoolean keys) const
{
  PINDEX index = 0;
  for (PINDEX i = 0; i < GetSize(); i++) {
    PHashTableElement * element = GetAt(i).m_head;
    while (element != NULL) {
      PObject * keydata = keys ? element->m_key : element->m_data;
      if (byValue ? (*keydata == *obj) : (keydata == obj))
        return index;
      element = element->m_next;
      index++;
    }
  }
  return P_MAX_INDEX;
}


PHashTableElement * PHashTableInfo::NextElement(PHashTableElement * element) const
{
  if (element == NULL)
    return NULL;

  PINDEX bucket = element->m_bucket;
  if (element->m_next != NULL)
    element = element->m_next;
  else {
    do {
      if (++bucket >= GetSize())
        return NULL;
    } while ((element = GetAt(bucket).m_head) == NULL);
  }
  return element;
}


PHashTableElement * PHashTableInfo::PrevElement(PHashTableElement * element) const
{
  if (element == NULL)
    return NULL;

  PINDEX bucket = element->m_bucket;
  if (element->m_prev != NULL)
    element = element->m_prev;
  else {
    do {
      if (bucket-- == 0)
        return NULL;
    } while ((element = GetAt(bucket).m_tail) == NULL);
  }
  return element;
}


///////////////////////////////////////////////////////////////////////////////

PHashTable::PHashTable()
  : hashTable(new PHashTableInfo)
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
  PHashTableInfo * original = PAssertNULL(hash->hashTable);

  hashTable = new PHashTableInfo(original->GetSize());
  PAssert(hashTable != NULL, POutOfMemory);
  hashTable->deleteKeys = original->deleteKeys;

  for (PINDEX i = 0; i < sz; i++) {
    PHashTableElement * lastElement = original->GetElementAt(i);
    PObject * data = lastElement->m_data;
    if (data != NULL)
      data = data->Clone();
    hashTable->AppendElement(lastElement->m_key->Clone(), data PTRACE_PARAM(, this));
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
  return true;
}


PObject & PHashTable::AbstractGetDataAt(PINDEX index) const
{
  PHashTableElement * lastElement = hashTable->GetElementAt(index);
  PAssert(lastElement != NULL, PInvalidArrayIndex);
  return *lastElement->m_data;
}


const PObject & PHashTable::AbstractGetKeyAt(PINDEX index) const
{
  PHashTableElement * lastElement = hashTable->GetElementAt(index);
  PAssert(lastElement != NULL, PInvalidArrayIndex);
  return *lastElement->m_key;
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

  ++reference->size;
  hashTable->AppendElement(obj, NULL PTRACE_PARAM(, this));
  return reference->size;
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
    return false;

  if (hashTable->GetElementAt(*obj) == NULL)
    return false;

  hashTable->deleteKeys = hashTable->reference->deleteObjects = reference->deleteObjects;
  hashTable->RemoveElement(*obj);
  reference->size--;
  return true;
}


PObject * PAbstractSet::RemoveAt(PINDEX index)
{
  PHashTableElement * lastElement = hashTable->GetElementAt(index);
  if (lastElement == NULL)
    return NULL;

  PObject * obj = lastElement->m_key;
  hashTable->deleteKeys = hashTable->reference->deleteObjects = reference->deleteObjects;
  hashTable->RemoveElement(*obj);
  reference->size--;
  return obj;
}


PINDEX PAbstractSet::GetObjectsIndex(const PObject * obj) const
{
  return hashTable->GetElementsIndex(obj, false, true);
}


PINDEX PAbstractSet::GetValuesIndex(const PObject & obj) const
{
  return hashTable->GetElementsIndex(&obj, true, true);
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
    return false;

  RemoveAt(idx);
  return true;
}


PObject * PAbstractDictionary::RemoveAt(PINDEX index)
{
  PObject & obj = AbstractGetDataAt(index);
  AbstractSetAt(AbstractGetKeyAt(index), NULL);
  return &obj;
}


PINDEX PAbstractDictionary::GetObjectsIndex(const PObject * obj) const
{
  return hashTable->GetElementsIndex(obj, false, false);
}


PINDEX PAbstractDictionary::GetValuesIndex(const PObject & obj) const
{
  return hashTable->GetElementsIndex(&obj, true, false);
}


PBoolean PAbstractDictionary::SetAt(PINDEX index, PObject * val)
{
  return AbstractSetAt(AbstractGetKeyAt(index), val);
}


PObject * PAbstractDictionary::GetAt(PINDEX index) const
{
  PHashTableElement * element = hashTable->GetElementAt(index);
  return PAssert(element != NULL, PInvalidArrayIndex) ? element->m_data : NULL;
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
    PHashTableElement * element = hashTable->GetElementAt(key);
    if (element == NULL) {
      ++reference->size;
      hashTable->AppendElement(key.Clone(), obj PTRACE_PARAM(, this));
    }
    else if (element->m_data != obj) {
      if (reference->deleteObjects) 
    	delete element->m_data;
      element->m_data = obj;
    }
  }

  return obj;
}


PObject * PAbstractDictionary::AbstractGetAt(const PObject & key) const
{
  PHashTableElement * element = hashTable->GetElementAt(key);
  return element != NULL ? element->m_data : (PObject *)NULL;
}


PObject & PAbstractDictionary::GetRefAt(const PObject & key) const
{
  PHashTableElement * element = hashTable->GetElementAt(key);
  return *PAssertNULL(element)->m_data;
}


void PAbstractDictionary::AbstractGetKeys(PArrayObjects & keys) const
{
  keys.SetSize(GetSize());
  PINDEX index = 0;
  for (PHashTableElement * element = hashTable->GetElementAt((PINDEX)0); element != NULL; element = hashTable->NextElement(element))
    keys.SetAt(index++, element->m_key->Clone());
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
