/*
 * $Id: collect.cxx,v 1.38 1998/05/17 02:29:46 robertj Exp $
 *
 * Portable Windows Library
 *
 * PContainer Class Implementation
 *
 * Copyright 1993 Equivalence
 *
 * $Log: collect.cxx,v $
 * Revision 1.38  1998/05/17 02:29:46  robertj
 * Fixed GetObjectsIndex()/GetValuesIndex() finding elements that have a hash clash.
 *
 * Revision 1.37  1998/03/26 23:31:50  robertj
 * Fixed bug in RemoveAll() deleting objects twice.
 *
 * Revision 1.36  1998/03/26 11:19:50  robertj
 * Fix bug with unsigned PINDEX in array SetSize.
 *
 * Revision 1.35  1998/03/25 12:58:41  robertj
 * Fixed memory leak if resize PArray
 *
 * Revision 1.34  1998/03/24 02:58:52  robertj
 * Fixed uninitialised variable in dictionary MakeUnique() function.
 *
 * Revision 1.33  1998/01/26 01:41:19  robertj
 * GNU compatibility.
 *
 * Revision 1.32  1998/01/26 00:36:10  robertj
 * Fixed MakeUnique() function for dictionaries and sets.
 *
 * Revision 1.31  1998/01/06 12:00:15  robertj
 * Fixed "typesafe" templates/macros for dictionaries, especially on GNU.
 *
 * Revision 1.30  1997/12/11 10:30:02  robertj
 * Added type correct Contains() function to dictionaries.
 *
 * Revision 1.29  1997/06/08 04:48:30  robertj
 * Fixed problems in sorted list with multiple identical entries.
 *
 * Revision 1.28  1997/04/27 05:50:14  robertj
 * DLL support.
 *
 * Revision 1.27  1997/02/14 13:59:09  robertj
 * Rewrite of sorted list to use sentinel record rather than NULL pointer.
 *
 * Revision 1.26  1996/08/17 09:55:23  robertj
 * Optimised RemoveAll() for object arrays.
 *
 * Revision 1.25  1996/08/08 10:08:43  robertj
 * Directory structure changes for common files.
 *
 * Revision 1.24  1996/07/15 10:32:52  robertj
 * Fixed bug in sorted list (crash on remove).
 *
 * Revision 1.23  1996/05/26 03:46:24  robertj
 * Compatibility to GNU 2.7.x
 *
 * Revision 1.22  1996/03/26 00:52:38  robertj
 * Fixed bug in dictionary decrementing size when removing element even if already removed.
 *
 * Revision 1.21  1996/02/19 13:32:31  robertj
 * Fixed yet another bug in PSortedList, not setting cache index value correctly.
 *
 * Revision 1.20  1996/02/08 12:24:13  robertj
 * Added default print for dictionaries in form key=data\n.
 * Added missing GetAt() function on PSet to be consistent with all others.
 *
 * Revision 1.19  1996/02/03 11:07:59  robertj
 * A bit more bullet proofing of sorted list class.
 *
 * Revision 1.18  1996/01/30 23:30:40  robertj
 * Added optimisation to sorted list GetAt() to use cached element.
 *
 * Revision 1.17  1996/01/28 14:11:45  robertj
 * Fixed bug in sorted list for when getting entry one before last one cached.
 *
 * Revision 1.16  1996/01/28 02:52:45  robertj
 * Added assert into all Compare functions to assure comparison between compatible objects.
 *
 * Revision 1.15  1996/01/23 13:18:29  robertj
 * Fixed bug in sorted list GetObjectsIndex not checking if is same object
 * Fixed bug in sorted list append not returning correct value.
 *
 * Revision 1.14  1995/01/27 11:12:38  robertj
 * Fixed nasty bug in sorted lists.
 *
 * Revision 1.13  1995/01/09  12:31:49  robertj
 * Removed unnecesary return value from I/O functions.
 *
 * Revision 1.12  1994/12/13  11:50:52  robertj
 * Added MakeUnique() function to all container classes.
 *
 * Revision 1.11  1994/12/12  10:16:25  robertj
 * Restructuring and documentation of container classes.
 * Renaming of some macros for declaring container classes.
 * Added some extra functionality to PString.
 * Added start to 2 byte characters in PString.
 * Fixed incorrect overrides in PCaselessString.
 *
 * Revision 1.10  1994/12/05  11:24:58  robertj
 * Fixed bugs in InsertAt and RemoveAt in PObjectArray.
 *
 * Revision 1.9  1994/10/30  11:34:49  robertj
 * Fixed ObjectArray to have pointer to array object pointers.
 *
 * Revision 1.8  1994/10/23  03:41:31  robertj
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

#include <ptlib.h>



///////////////////////////////////////////////////////////////////////////////

void PCollection::PrintOn(ostream &strm) const
{
  for (PINDEX  i = 0; i < GetSize(); i++) {
    PObject * obj = GetAt(i);
    if (obj != NULL)
      strm << *obj;
  }
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
    for (PINDEX i = 0; i < theArray->GetSize(); i++) {
      if ((*theArray)[i] != NULL)
        delete (*theArray)[i];
    }
  }
  delete theArray;
}


void PArrayObjects::RemoveAll()
{
  SetSize(0);
}


void PArrayObjects::CloneContents(const PArrayObjects * array)
{
  ObjPtrArray & oldArray = *array->theArray;
  theArray = PNEW ObjPtrArray(oldArray.GetSize());
  for (PINDEX i = 0; i < GetSize(); i++) {
    PObject * ptr = oldArray[i];
    if (ptr != NULL)
      SetAt(i, ptr->Clone());
  }
}


PObject::Comparison PArrayObjects::Compare(const PObject & obj) const
{
  PAssert(obj.IsDescendant(PArrayObjects::Class()), PInvalidCast);
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


BOOL PArrayObjects::SetSize(PINDEX newSize)
{
  PINDEX sz = theArray->GetSize();
  if (reference->deleteObjects && sz > 0) {
    for (PINDEX i = sz-1; i > newSize; i--) {
      PObject * obj = theArray->GetAt(i);
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
  return (*theArray)[index];
}


BOOL PArrayObjects::SetAt(PINDEX index, PObject * obj)
{
  if (!theArray->SetMinSize(index+1))
    return FALSE;
  PObject * oldObj = theArray->GetAt(index);
  if (oldObj != NULL && reference->deleteObjects)
    delete oldObj;
  (*theArray)[index] = obj;
  return TRUE;
}


PINDEX PArrayObjects::InsertAt(PINDEX index, PObject * obj)
{
  for (PINDEX i = GetSize(); i > index; i--)
    (*theArray)[i] = (*theArray)[i-1];
  (*theArray)[index] = obj;
  return index;
}


PObject * PArrayObjects::RemoveAt(PINDEX index)
{
  PObject * obj = (*theArray)[index];
  PINDEX size = GetSize()-1;
  for (PINDEX i = index; i < size; i++)
    (*theArray)[i] = (*theArray)[i+1];
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
    if (*(*theArray)[i] == obj)
      return i;
  }
  return P_MAX_INDEX;
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
  Element * element = list->info->head;

  info = new Info;
  PAssertNULL(info);

  while (element != NULL) {
    Append(element->data->Clone());
    element = element->next;
  }
}


PObject::Comparison PAbstractList::Compare(const PObject & obj) const
{
  PAssert(obj.IsDescendant(PAbstractList::Class()), PInvalidCast);
  Element * elmt1 = info->head;
  Element * elmt2 = ((const PAbstractList &)obj).info->head;
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
  Element * element = new Element(PAssertNULL(obj));
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

  Element * newElement = new Element(obj);
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

  Element * elmt = info->lastElement;

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
  return SetCurrent(index) ? info->lastElement->data : (PObject *)NULL;
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
  Element * element = info->head;
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
  Element * element = info->head;
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


PAbstractList::Element::Element(PObject * theData)
{
  next = prev = NULL;
  data = theData;
}


///////////////////////////////////////////////////////////////////////////////

PAbstractSortedList::Element nil = NULL;

PAbstractSortedList::PAbstractSortedList()
{
  info = new Info;
  PAssert(info != NULL, POutOfMemory);
}


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
  Element * element = list->info->root;
  while (element->left != &nil)
    element = element->left;

  info = new Info;
  PAssertNULL(info);

  while (element != &nil) {
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
  PAssert(obj.IsDescendant(PAbstractSortedList::Class()), PInvalidCast);
  Element * elmt1 = info->root;
  while (elmt1->left != &nil)
    elmt1 = elmt1->left;

  Element * elmt2 = ((const PAbstractSortedList &)obj).info->root;
  while (elmt2->left != &nil)
    elmt2 = elmt2->left;

  while (elmt1 != &nil && elmt2 != &nil) {
    if (elmt1 == &nil)
      return LessThan;
    if (elmt2 == &nil)
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
  Element * z = new Element(PAssertNULL(obj));
  Element * x = info->root;
  Element * y = &nil;
  while (x != &nil) {
    x->subTreeSize++;
    y = x;
    x = *z->data < *x->data ? x->left : x->right;
  }
  z->parent = y;
  if (y == &nil)
    info->root = z;
  else if (*z->data < *y->data)
    y->left = z;
  else
    y->right = z;

  info->lastElement = x = z;

  x->colour = Element::Red;
  while (x != info->root && x->parent->colour == Element::Red) {
    if (x->parent == x->parent->parent->left) {
      y = x->parent->parent->right;
      if (y->colour == Element::Red) {
        x->parent->colour = Element::Black;
        y->colour = Element::Black;
        x->parent->parent->colour = Element::Red;
        x = x->parent->parent;
      }
      else {
        if (x == x->parent->right) {
          x = x->parent;
          LeftRotate(x);
        }
        x->parent->colour = Element::Black;
        x->parent->parent->colour = Element::Red;
        RightRotate(x->parent->parent);
      }
    }
    else {
      y = x->parent->parent->left;
      if (y->colour == Element::Red) {
        x->parent->colour = Element::Black;
        y->colour = Element::Black;
        x->parent->parent->colour = Element::Red;
        x = x->parent->parent;
      }
      else {
        if (x == x->parent->left) {
          x = x->parent;
          RightRotate(x);
        }
        x->parent->colour = Element::Black;
        x->parent->parent->colour = Element::Red;
        LeftRotate(x->parent->parent);
      }
    }
  }

  info->root->colour = Element::Black;

  x = info->lastElement;
  info->lastIndex = x->left->subTreeSize;
  while (x != info->root) {
    if (x != x->parent->left)
      info->lastIndex += x->parent->left->subTreeSize+1;
    x = x->parent;
  }

  reference->size++;
  return info->lastIndex;
}


BOOL PAbstractSortedList::Remove(const PObject * obj)
{
  Element * element = info->root;
  while (element != &nil && element->data != obj)
    element = *obj < *element->data ? element->left : element->right;
  if (element == &nil)
    return FALSE;

  RemoveElement(element);
  return TRUE;
}


PObject * PAbstractSortedList::RemoveAt(PINDEX index)
{
  Element * node = info->root->OrderSelect(index+1);
  PObject * data = node->data;
  RemoveElement(node);
  return reference->deleteObjects ? (PObject *)NULL : data;
}


void PAbstractSortedList::RemoveAll()
{
  if (info->root != &nil) {
    info->root->DeleteSubTrees(reference->deleteObjects);
    delete info->root;
    info->root = &nil;
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

  if (index != info->lastIndex) {
    if (index == info->lastIndex-1) {
      info->lastIndex--;
      info->lastElement = info->lastElement->Predecessor();
    }
    else if (index == info->lastIndex+1 && info->lastElement != NULL) {
      info->lastIndex++;
      info->lastElement = info->lastElement->Successor();
    }
    else {
      info->lastIndex = index;
      info->lastElement = info->root->OrderSelect(index+1);
    }
  }

  return PAssertNULL(info->lastElement)->data;
}


PINDEX PAbstractSortedList::GetObjectsIndex(const PObject * obj) const
{
  Element * elmt;
  PINDEX pos = info->root->ValueSelect(*obj, elmt);
  if (pos == P_MAX_INDEX)
    return P_MAX_INDEX;

  if (elmt->data != obj) {
    PINDEX savePos = pos;
    Element * saveElmt = elmt;
    while (elmt->data != obj &&
            (elmt = elmt->Predecessor()) != &nil &&
            *obj == *elmt->data)
      pos--;
    if (elmt->data != obj) {
      pos = savePos;
      elmt = saveElmt;
      while (elmt->data != obj &&
              (elmt = elmt->Successor()) != &nil &&
              *obj == *elmt->data)
        pos++;
      if (elmt->data != obj)
        return P_MAX_INDEX;
    }
  }

  info->lastIndex = pos;
  info->lastElement = elmt;

  return pos;
}


PINDEX PAbstractSortedList::GetValuesIndex(const PObject & obj) const
{
  PINDEX pos = info->root->ValueSelect(obj, info->lastElement);
  if (pos == P_MAX_INDEX)
    return P_MAX_INDEX;

  info->lastIndex = pos;

  Element * prev;
  while ((prev = info->lastElement->Predecessor()) != &nil && obj == *prev->data) {
    info->lastElement = prev;
    info->lastIndex--;
  }

  return info->lastIndex;
}


void PAbstractSortedList::RemoveElement(Element * node)
{
  PAssertNULL(node);

  if (node->data != NULL && reference->deleteObjects)
    delete node->data;

  Element * y = node->left == &nil || node->right == &nil ? node : node->Successor();

  Element * t = y;
  while (t != &nil) {
    t->subTreeSize--;
    t = t->parent;
  }

  Element * x = y->left != &nil ? y->left : y->right;
  x->parent = y->parent;

  if (y->parent == &nil)
    info->root = x;
  else if (y == y->parent->left)
    y->parent->left = x;
  else
    y->parent->right = x;

  if (y != node)
    node->data = y->data;

  if (y->colour == Element::Black) {
    while (x != info->root && x->colour == Element::Black) {
      if (x == x->parent->left) {
        Element * w = x->parent->right;
        if (w->colour == Element::Red) {
          w->colour = Element::Black;
          x->parent->colour = Element::Red;
          LeftRotate(x->parent);
          w = x->parent->right;
        }
        if (w->left->colour == Element::Black && w->right->colour == Element::Black) {
          w->colour = Element::Red;
          x = x->parent;
        }
        else {
          if (w->right->colour == Element::Black) {
            w->left->colour = Element::Black;
            w->colour = Element::Red;
            RightRotate(w);
            w = x->parent->right;
          }
          w->colour = x->parent->colour;
          x->parent->colour = Element::Black;
          w->right->colour = Element::Black;
          LeftRotate(x->parent);
          x = info->root;
        }
      }
      else {
        Element * w = x->parent->left;
        if (w->colour == Element::Red) {
          w->colour = Element::Black;
          x->parent->colour = Element::Red;
          RightRotate(x->parent);
          w = x->parent->left;
        }
        if (w->right->colour == Element::Black && w->left->colour == Element::Black) {
          w->colour = Element::Red;
          x = x->parent;
        }
        else {
          if (w->left->colour == Element::Black) {
            w->right->colour = Element::Black;
            w->colour = Element::Red;
            LeftRotate(w);
            w = x->parent->left;
          }
          w->colour = x->parent->colour;
          x->parent->colour = Element::Black;
          w->left->colour = Element::Black;
          RightRotate(x->parent);
          x = info->root;
        }
      }
    }
    x->colour = Element::Black;
  }

  delete y;

  reference->size--;
  info->lastIndex = P_MAX_INDEX;
  info->lastElement = NULL;
}


void PAbstractSortedList::LeftRotate(Element * node)
{
  Element * pivot = PAssertNULL(node)->right;
  node->right = pivot->left;
  if (pivot->left != &nil)
    pivot->left->parent = node;
  pivot->parent = node->parent;
  if (node->parent == &nil)
    info->root = pivot;
  else if (node == node->parent->left)
    node->parent->left = pivot;
  else
    node->parent->right = pivot;
  pivot->left = node;
  node->parent = pivot;
  pivot->subTreeSize = node->subTreeSize;
  node->subTreeSize = node->left->subTreeSize + node->right->subTreeSize + 1;
}


void PAbstractSortedList::RightRotate(Element * node)
{
  Element * pivot = PAssertNULL(node)->left;
  node->left = pivot->right;
  if (pivot->right != &nil)
    pivot->right->parent = node;
  pivot->parent = node->parent;
  if (node->parent == &nil)
    info->root = pivot;
  else if (node == node->parent->right)
    node->parent->right = pivot;
  else
    node->parent->left = pivot;
  pivot->right = node;
  node->parent = pivot;
  pivot->subTreeSize = node->subTreeSize;
  node->subTreeSize = node->left->subTreeSize + node->right->subTreeSize + 1;
}


PAbstractSortedList::Element::Element(PObject * theData)
{
  parent = left = right = &nil;
  colour = Black;
  subTreeSize = theData != NULL ? 1 : 0;
  data = theData;
}


PAbstractSortedList::Element * PAbstractSortedList::Element::Successor() const
{
  Element * next;
  if (right != &nil) {
    next = right;
    while (next->left != &nil)
      next = next->left;
  }
  else {
    next = parent;
    const Element * node = this;
    while (next != &nil && node == next->right) {
      node = next;
      next = node->parent;
    }
  }
  return next;
}


PAbstractSortedList::Element*PAbstractSortedList::Element::Predecessor() const
{
  Element * pred;
  if (left != &nil) {
    pred = left;
    while (pred->right != &nil)
      pred = pred->right;
  }
  else {
    pred = parent;
    const Element * node = this;
    while (pred != &nil && node == pred->left) {
      node = pred;
      pred = node->parent;
    }
  }
  return pred;
}


PAbstractSortedList::Element * PAbstractSortedList::Element::OrderSelect(PINDEX index)
{
  PINDEX r = left->subTreeSize+1;
  if (index == r)
    return this;

  if (index < r) {
    if (left != &nil)
      return left->OrderSelect(index);
  }
  else {
    if (right != &nil)
      return right->OrderSelect(index - r);
  }

  PAssertAlways("Order select failed!");
  return &nil;
}


PINDEX PAbstractSortedList::Element::ValueSelect(const PObject & obj,
                                                 Element * & lastElement)
{
  if (this != &nil) {
    switch (data->Compare(obj)) {
      case PObject::LessThan :
      {
        PINDEX index = right->ValueSelect(obj, lastElement);
        if (index != P_MAX_INDEX)
          return left->subTreeSize + index + 1;
        break;
      }

      case PObject::GreaterThan :
        return left->ValueSelect(obj, lastElement);

      default :
        lastElement = this;
        return left->subTreeSize;
    }
  }

  return P_MAX_INDEX;
}


void PAbstractSortedList::Element::DeleteSubTrees(BOOL deleteObject)
{
  if (left != &nil) {
    left->DeleteSubTrees(deleteObject);
    delete left;
    left = &nil;
  }
  if (right != &nil) {
    right->DeleteSubTrees(deleteObject);
    delete right;
    right = &nil;
  }
  if (deleteObject) {
    delete data;
    data = NULL;
  }
}


PAbstractSortedList::Info::Info()
{
  root = &nil;
  lastElement = NULL;
  lastIndex = P_MAX_INDEX;
}


///////////////////////////////////////////////////////////////////////////////

PObject * POrdinalKey::Clone() const
{
  return PNEW POrdinalKey(theKey);
}


PObject::Comparison POrdinalKey::Compare(const PObject & obj) const
{
  PAssert(obj.IsDescendant(POrdinalKey::Class()), PInvalidCast);
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

void PHashTable::Table::DestroyContents()
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


PINDEX PHashTable::Table::AppendElement(PObject * key, PObject * data)
{
  lastElement = NULL;

  PINDEX bucket = PAssertNULL(key)->HashFunction();
  Element * list = GetAt(bucket);
  Element * element = new Element;
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
  return bucket;
}


PObject * PHashTable::Table::RemoveElement(const PObject & key)
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


BOOL PHashTable::Table::SetLastElementAt(PINDEX index)
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


PHashTable::Element * PHashTable::Table::GetElementAt(const PObject & key)
{
  if (lastElement != NULL && *lastElement->key == key)
    return lastElement;

  Element * list = GetAt(key.HashFunction());
  if (list != NULL) {
    Element * element = list;
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


PINDEX PHashTable::Table::GetElementsIndex(
                           const PObject * obj, BOOL byValue, BOOL keys) const
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


///////////////////////////////////////////////////////////////////////////////

PHashTable::PHashTable()
  : hashTable(PNEW PHashTable::Table)
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
  PAssertNULL(hash);
  PINDEX sz = hash->GetSize();
  PAssertNULL(hash->hashTable);
  PHashTable::Table * original = hash->hashTable;

  hashTable = PNEW PHashTable::Table(original->GetSize());
  PAssertNULL(hashTable);
  hashTable->lastElement = NULL;

  for (PINDEX i = 0; i < sz; i++) {
    original->SetLastElementAt(i);
    PObject * data = original->lastElement->data;
    if (data != NULL)
      data = data->Clone();
    hashTable->AppendElement(original->lastElement->key->Clone(), data);
  }
}


PObject::Comparison PHashTable::Compare(const PObject & obj) const
{
  PAssert(obj.IsDescendant(PHashTable::Class()), PInvalidCast);
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


BOOL PAbstractSet::Remove(const PObject * obj)
{
  hashTable->deleteKeys =
                hashTable->reference->deleteObjects = reference->deleteObjects;
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


PObject * PAbstractSet::GetAt(PINDEX index) const
{
  return (PObject *)&AbstractGetKeyAt(index);
}


BOOL PAbstractSet::SetAt(PINDEX, PObject * obj)
{
  return Append(obj);
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
 
 
BOOL PAbstractDictionary::Remove(const PObject *)
{
  PAssertAlways(PUnimplementedFunction);
  return FALSE;
}


PObject * PAbstractDictionary::RemoveAt(PINDEX index)
{
  PObject & obj = AbstractGetDataAt(index);
  AbstractSetAt(AbstractGetKeyAt(index), NULL);
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
  return SetAt(POrdinalKey(index), val);
}


PObject * PAbstractDictionary::GetAt(PINDEX index) const
{
  return GetAt(POrdinalKey(index));
}
 
 
BOOL PAbstractDictionary::SetDataAt(PINDEX index, PObject * val)
{
  return AbstractSetAt(AbstractGetKeyAt(index), val);
}


BOOL PAbstractDictionary::AbstractSetAt(const PObject & key, PObject * obj)
{
  if (obj == NULL) {
    obj = hashTable->RemoveElement(key);
    if (obj != NULL) {
      if (reference->deleteObjects)
        delete obj;
      reference->size--;
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
        delete hashTable->lastElement->data;
      hashTable->lastElement->data = obj;
    }
  }
  return TRUE;
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


void PAbstractDictionary::PrintOn(ostream &strm) const
{
  for (PINDEX  i = 0; i < GetSize(); i++)
    strm << AbstractGetKeyAt(i) << '=' << AbstractGetDataAt(i) << endl;
}


// End Of File ///////////////////////////////////////////////////////////////
