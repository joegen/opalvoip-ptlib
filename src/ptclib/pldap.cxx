/*
 * pldap.cxx
 *
 * Lightweight Directory Access Protocol interface class.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-2003 Equivalence Pty. Ltd.
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
 * Contributor(s): ______________________________________.
 *
 * $Log: pldap.cxx,v $
 * Revision 1.6  2003/04/16 08:00:19  robertj
 * Windoes psuedo autoconf support
 *
 * Revision 1.5  2003/04/07 11:59:52  robertj
 * Fixed search function returning an error if can't find anything for filter.
 *
 * Revision 1.4  2003/04/01 07:05:16  robertj
 * Added ability to specify host:port in opening an LDAP server
 *
 * Revision 1.3  2003/03/31 03:32:53  robertj
 * Major addition of functionality.
 *
 */

#ifdef __GNUC__
#pragma implementation "pldap.h"
#endif

#include <ptlib.h>

#include <ptlib/sockets.h>
#include <ptclib/pldap.h>

#if P_LDAP

#include <ldap.h>


#if defined(_WIN32)
#pragma comment(lib, P_LDAP_LIBRARY)
#endif


///////////////////////////////////////////////////////////////////////////////

PLDAPSession::PLDAPSession(const PString & baseDN)
  : ldapContext(NULL),
    errorNumber(LDAP_SUCCESS),
    protocolVersion(LDAP_VERSION3),
    defaultBaseDN(baseDN),
    searchLimit(UINT_MAX),
    timeout(PMaxTimeInterval),
    multipleValueSeparator('\n')
{
}


PLDAPSession::~PLDAPSession()
{
  Close();
}


BOOL PLDAPSession::Open(const PString & server, WORD port)
{
  Close();

  PString host = server;
  PINDEX colon = server.Find(':');
  if (colon != P_MAX_INDEX) {
    host = server.Left(colon);
    port = PIPSocket::GetPortByService(server.Mid(colon+1), "tcp");
  }

  ldapContext = ldap_init(server, port);
  if (!IsOpen())
    return FALSE;

  SetOption(LDAP_OPT_PROTOCOL_VERSION, protocolVersion);
  return TRUE;
}


BOOL PLDAPSession::Close()
{
  if (!IsOpen())
    return FALSE;

  ldap_unbind(ldapContext);
  ldapContext = NULL;
  return TRUE;
}


BOOL PLDAPSession::SetOption(int optcode, int value)
{
  if (!IsOpen())
    return FALSE;

  return ldap_set_option(ldapContext, optcode, &value);
}


BOOL PLDAPSession::SetOption(int optcode, void * value)
{
  if (!IsOpen())
    return FALSE;

  return ldap_set_option(ldapContext, optcode, value);
}


BOOL PLDAPSession::Bind(const PString & who,
                        const PString & passwd,
                        AuthenticationMethod authMethod)
{
  if (!IsOpen())
    return FALSE;

  const char * whoPtr;
  if (who.IsEmpty())
    whoPtr = NULL;
  else
    whoPtr = who;
  errorNumber = ldap_bind_s(ldapContext, whoPtr, passwd, authMethod);
  return errorNumber == LDAP_SUCCESS;
}


PLDAPSession::ModAttrib::ModAttrib(const PString & n, Operation o)
  : name(n),
    op(o)
{
}


void PLDAPSession::ModAttrib::SetLDAPMod(struct ldapmod & mod, Operation defaultOp)
{
  mod.mod_type = (char *)(const char *)name;

  Operation realOp = op == NumOperations ? defaultOp : op;
  static const int OpCode[NumOperations] = {
    LDAP_MOD_ADD, LDAP_MOD_REPLACE, LDAP_MOD_DELETE
  };
  mod.mod_op = OpCode[realOp];

  if (IsBinary())
    mod.mod_op |= LDAP_MOD_BVALUES;

  SetLDAPModVars(mod);
}


PLDAPSession::StringModAttrib::StringModAttrib(const PString & name,
                                               Operation op)
  : ModAttrib(name, op)
{
}


PLDAPSession::StringModAttrib::StringModAttrib(const PString & name,
                                               const PString & value,
                                               Operation op)
  : ModAttrib(name, op)
{
  AddValue(value);
}


PLDAPSession::StringModAttrib::StringModAttrib(const PString & name,
                                               const PStringList & vals,
                                               Operation op)
  : ModAttrib(name, op),
    values(vals)
{
}


void PLDAPSession::StringModAttrib::SetValue(const PString & value)
{
  values.RemoveAll();
  values.AppendString(value);
}


void PLDAPSession::StringModAttrib::AddValue(const PString & value)
{
  values.AppendString(value);
}


BOOL PLDAPSession::StringModAttrib::IsBinary() const
{
  return FALSE;
}


void PLDAPSession::StringModAttrib::SetLDAPModVars(struct ldapmod & mod)
{
  pointers.SetSize(values.GetSize()+1);
  PINDEX i;
  for (i = 0; i < values.GetSize(); i++)
    pointers[i] = values[i].GetPointer();
  pointers[i] = NULL;
  mod.mod_values = pointers.GetPointer();
}


PLDAPSession::BinaryModAttrib::BinaryModAttrib(const PString & name,
                                               Operation op)
  : ModAttrib(name, op)
{
}


PLDAPSession::BinaryModAttrib::BinaryModAttrib(const PString & name,
                                               const PBYTEArray & value,
                                               Operation op)
  : ModAttrib(name, op)
{
  AddValue(value);
}


PLDAPSession::BinaryModAttrib::BinaryModAttrib(const PString & name,
                                               const PList<PBYTEArray> & vals,
                                               Operation op)
  : ModAttrib(name, op),
    values(vals)
{
}


void PLDAPSession::BinaryModAttrib::SetValue(const PBYTEArray & value)
{
  values.RemoveAll();
  values.Append(new PBYTEArray(value));
}


void PLDAPSession::BinaryModAttrib::AddValue(const PBYTEArray & value)
{
  values.Append(new PBYTEArray(value));
}


BOOL PLDAPSession::BinaryModAttrib::IsBinary() const
{
  return TRUE;
}


void PLDAPSession::BinaryModAttrib::SetLDAPModVars(struct ldapmod & mod)
{
  pointers.SetSize(values.GetSize()+1);
  bervals.SetSize(values.GetSize()*sizeof(berval));
  berval * ber = (berval *)bervals.GetPointer();
  PINDEX i;
  for (i = 0; i < values.GetSize(); i++) {
    ber[i].bv_val = (char *)values[i].GetPointer();
    ber[i].bv_len = values[i].GetSize();
    pointers[i] = &ber[i];
  }
  pointers[i] = NULL;
  mod.mod_bvalues = pointers.GetPointer();
}


static LDAPMod ** CreateLDAPModArray(const PList<PLDAPSession::ModAttrib> & attributes,
                                     PLDAPSession::ModAttrib::Operation defaultOp,
                                     PBYTEArray & storage)
{
  PINDEX count = attributes.GetSize();
  storage.SetSize(count*sizeof(LDAPMod) + (count+1)*sizeof(LDAPMod *));

  LDAPMod ** attrs = (LDAPMod **)storage.GetPointer();
  LDAPMod *  attr  = (LDAPMod * )&attrs[count+1];
  for (PINDEX i = 0; i < count; i++) {
    attrs[i] = &attr[i];
    attributes[i].SetLDAPMod(attr[i], defaultOp);
  }

  return attrs;
}


static PList<PLDAPSession::ModAttrib> AttribsFromDict(const PStringToString & attributes)
{
  PList<PLDAPSession::ModAttrib> attrs;

  for (PINDEX i = 0; i < attributes.GetSize(); i++)
    attrs.Append(new PLDAPSession::StringModAttrib(attributes.GetKeyAt(i), attributes.GetDataAt(i)));

  return attrs;
}


static PList<PLDAPSession::ModAttrib> AttribsFromArray(const PStringArray & attributes)
{
  PList<PLDAPSession::ModAttrib> attrs;

  for (PINDEX i = 0; i < attributes.GetSize(); i++) {
    PString attr = attributes[i];
    PINDEX equal = attr.Find('=');
    if (equal != P_MAX_INDEX)
      attrs.Append(new PLDAPSession::StringModAttrib(attr.Left(equal), attr.Mid(equal+1)));
  }

  return attrs;
}


static PList<PLDAPSession::ModAttrib> AttribsFromStruct(const PLDAPStructBase & attributes)
{
  PList<PLDAPSession::ModAttrib> attrs;

  for (PINDEX i = 0; i < attributes.GetNumAttributes(); i++) {
    PLDAPAttributeBase & attr = attributes.GetAttribute(i);
    if (attr.IsBinary())
      attrs.Append(new PLDAPSession::BinaryModAttrib(attr.GetName(), attr.ToBinary()));
    else
      attrs.Append(new PLDAPSession::StringModAttrib(attr.GetName(), attr.ToString()));
  }

  return attrs;
}


BOOL PLDAPSession::Add(const PString & dn, const PList<ModAttrib> & attributes)
{
  if (!IsOpen())
    return FALSE;

  PBYTEArray storage;
  int msgid;
  errorNumber = ldap_add_ext(ldapContext,
                             dn,
                             CreateLDAPModArray(attributes, ModAttrib::Add, storage),
                             NULL,
                             NULL,
                             &msgid);
  if (errorNumber != LDAP_SUCCESS)
    return FALSE;

  P_timeval tval = timeout;
  LDAPMessage * result = NULL;
  ldap_result(ldapContext, msgid, LDAP_MSG_ALL, tval, &result);
  errorNumber = ldap_result2error(ldapContext, result, TRUE);

  return errorNumber == LDAP_SUCCESS;
}


BOOL PLDAPSession::Add(const PString & dn, const PStringToString & attributes)
{
  return Add(dn, AttribsFromDict(attributes));
}


BOOL PLDAPSession::Add(const PString & dn, const PStringArray & attributes)
{
  return Add(dn, AttribsFromArray(attributes));
}


BOOL PLDAPSession::Add(const PString & dn, const PLDAPStructBase & attributes)
{
  return Add(dn, AttribsFromStruct(attributes));
}


BOOL PLDAPSession::Modify(const PString & dn, const PList<ModAttrib> & attributes)
{
  if (!IsOpen())
    return FALSE;

  PBYTEArray storage;
  int msgid;
  errorNumber = ldap_modify_ext(ldapContext,
                                dn,
                                CreateLDAPModArray(attributes, ModAttrib::Replace, storage),
                                NULL,
                                NULL,
                                &msgid);
  if (errorNumber != LDAP_SUCCESS)
    return FALSE;

  P_timeval tval = timeout;
  LDAPMessage * result = NULL;
  ldap_result(ldapContext, msgid, LDAP_MSG_ALL, tval, &result);
  errorNumber = ldap_result2error(ldapContext, result, TRUE);

  return errorNumber == LDAP_SUCCESS;
}


BOOL PLDAPSession::Modify(const PString & dn, const PStringToString & attributes)
{
  return Add(dn, AttribsFromDict(attributes));
}


BOOL PLDAPSession::Modify(const PString & dn, const PStringArray & attributes)
{
  return Add(dn, AttribsFromArray(attributes));
}


BOOL PLDAPSession::Modify(const PString & dn, const PLDAPStructBase & attributes)
{
  return Add(dn, AttribsFromStruct(attributes));
}


BOOL PLDAPSession::Delete(const PString & dn)
{
  if (!IsOpen())
    return FALSE;

  int msgid;
  errorNumber = ldap_delete_ext(ldapContext, dn, NULL, NULL, &msgid);
  if (errorNumber != LDAP_SUCCESS)
    return FALSE;

  P_timeval tval = timeout;
  LDAPMessage * result = NULL;
  ldap_result(ldapContext, msgid, LDAP_MSG_ALL, tval, &result);
  errorNumber = ldap_result2error(ldapContext, result, TRUE);

  return errorNumber == LDAP_SUCCESS;
}


PLDAPSession::SearchContext::SearchContext()
{
  result = NULL;
  message = NULL;
  found = FALSE;
  completed = FALSE;
}


PLDAPSession::SearchContext::~SearchContext()
{
  if (message != NULL)
    ldap_msgfree(message);

  if (result != NULL && result != message)
    ldap_msgfree(result);
}


BOOL PLDAPSession::Search(SearchContext & context,
                          const PString & filter,
                          const PStringArray & attributes,
                          const PString & baseDN,
                          SearchScope scope)
{
  if (!IsOpen())
    return FALSE;

  PCharArray storage;
  char ** attribs = attributes.ToCharArray(&storage);

  PString base = baseDN;
  if (base.IsEmpty())
    base = defaultBaseDN;

  static const int ScopeCode[NumSearchScope] = {
    LDAP_SCOPE_BASE, LDAP_SCOPE_ONELEVEL, LDAP_SCOPE_SUBTREE
  };

  P_timeval tval = timeout;

  errorNumber = ldap_search_ext(ldapContext,
                                base,
                                ScopeCode[scope],
                                filter,
                                attribs,
                                FALSE,
                                NULL,
                                NULL,
                                tval,
                                searchLimit,
                                &context.msgid);
  if (errorNumber != LDAP_SUCCESS)
    return FALSE;

  if (ldap_result(ldapContext, context.msgid, LDAP_MSG_ONE, tval, &context.result) > 0)
    return GetNextSearchResult(context);

  errorNumber = ldap_result2error(ldapContext, context.result, TRUE);
  if (errorNumber == 0)
    errorNumber = LDAP_OTHER;
  return FALSE;
}


BOOL PLDAPSession::GetSearchResult(SearchContext & context, PStringToString & data)
{
  data.RemoveAll();

  if (!IsOpen())
    return FALSE;

  if (context.result == NULL || context.message == NULL || context.completed)
    return FALSE;

  // Extract the resulting data

  data.SetAt("dn", GetSearchResultDN(context));

  BerElement * ber = NULL;
  char * attrib = ldap_first_attribute(ldapContext, context.message, &ber);
  while (attrib != NULL) {

    struct berval ** bvals = ldap_get_values_len(ldapContext, context.message, attrib);
    if (bvals != NULL) {
      PString value = data(attrib);

      for (PINDEX i = 0; bvals[i] != NULL; i++ ) {
        if (!value)
          value += multipleValueSeparator;
        value += PString(bvals[i]->bv_val, bvals[i]->bv_len);
      }
      ber_bvecfree(bvals);

      data.SetAt(attrib, value);
    }

    ldap_memfree(attrib);
    attrib = ldap_next_attribute(ldapContext, context.message, ber);
  }

  return TRUE;
}


BOOL PLDAPSession::GetSearchResult(SearchContext & context,
                                   const PString & attribute,
                                   PString & data)
{
  data = PString::Empty();

  if (!IsOpen())
    return FALSE;

  if (context.result == NULL || context.message == NULL || context.completed)
    return FALSE;

  if (attribute == "dn") {
    data = GetSearchResultDN(context);
    return TRUE;
  }

  char ** values = ldap_get_values(ldapContext, context.message, attribute);
  if (values == NULL)
    return FALSE;

  PINDEX count = ldap_count_values(values);
  for (PINDEX i = 0; i < count; i++) {
    if (!data)
      data += multipleValueSeparator;
    data += values[i];
  }

  ldap_value_free(values);
  return TRUE;
}


BOOL PLDAPSession::GetSearchResult(SearchContext & context,
                                   const PString & attribute,
                                   PStringArray & data)
{
  data.RemoveAll();

  if (!IsOpen())
    return FALSE;

  if (context.result == NULL || context.message == NULL || context.completed)
    return FALSE;

  if (attribute == "dn") {
    data.SetSize(1);
    data[0] = GetSearchResultDN(context);
    return TRUE;
  }

  char ** values = ldap_get_values(ldapContext, context.message, attribute);
  if (values == NULL)
    return FALSE;

  PINDEX count = ldap_count_values(values);
  data.SetSize(count);
  for (PINDEX i = 0; i < count; i++)
    data[i] = values[i];

  ldap_value_free(values);
  return TRUE;
}


BOOL PLDAPSession::GetSearchResult(SearchContext & context,
                                   const PString & attribute,
                                   PArray<PBYTEArray> & data)
{
  data.RemoveAll();

  if (!IsOpen())
    return FALSE;

  if (attribute == "dn") {
    char * dn = ldap_get_dn(ldapContext, context.message);
    data.Append(new PBYTEArray((const BYTE *)dn, ::strlen(dn)));
    ldap_memfree(dn);
    return TRUE;
  }

  struct berval ** values = ldap_get_values_len(ldapContext, context.message, attribute);
  if (values == NULL)
    return FALSE;

  PINDEX count = ldap_count_values_len(values);
  data.SetSize(count);
  for (PINDEX i = 0; i < count; i++)
    data[i] = PBYTEArray((const BYTE *)values[i]->bv_val, values[i]->bv_len);

  ldap_value_free_len(values);
  return TRUE;
}


BOOL PLDAPSession::GetSearchResult(SearchContext & context,
                                   PLDAPStructBase & data)
{
  if (!IsOpen())
    return FALSE;

  BOOL atLeastOne = FALSE;

  for (PINDEX i = 0; i < data.GetNumAttributes(); i++) {
    PLDAPAttributeBase & attr = data.GetAttribute(i);
    if (attr.IsBinary()) {
      PArray<PBYTEArray> bin;
      if (GetSearchResult(context, attr.GetName(), bin)) {
        attr.FromBinary(bin);
        atLeastOne = TRUE;
      }
    }
    else {
      PString str;
      if (GetSearchResult(context, attr.GetName(), str)) {
        attr.FromString(str);
        atLeastOne = TRUE;
      }
    }
  }

  return atLeastOne;
}


PString PLDAPSession::GetSearchResultDN(SearchContext & context)
{
  PString str;

  if (context.message != NULL) {
    char * dn = ldap_get_dn(ldapContext, context.message);
    if (dn != NULL) {
      str = dn;
      ldap_memfree(dn);
    }
  }

  return str;
}


BOOL PLDAPSession::GetNextSearchResult(SearchContext & context)
{
  if (!IsOpen())
    return FALSE;

  if (context.result == NULL || context.completed)
    return FALSE;

  P_timeval tval = timeout;
  do {
    if (context.message == NULL)
      context.message = ldap_first_message(ldapContext, context.result);
    else
      context.message = ldap_next_message(ldapContext, context.message);

    if (context.message != NULL) {
      switch (ldap_msgtype(context.message)) {
        case LDAP_RES_SEARCH_ENTRY :
          context.found = TRUE;
          errorNumber = LDAP_SUCCESS;
          return TRUE;

        case LDAP_RES_SEARCH_RESULT :
          errorNumber = ldap_result2error(ldapContext, context.message, FALSE);
          if (errorNumber == 0 && !context.found)
            errorNumber = LDAP_NO_RESULTS_RETURNED;
          context.completed = TRUE;
          return FALSE;
      }
      // Ignore other result message types for now ...
    }

    ldap_msgfree(context.result);
  } while (ldap_result(ldapContext, context.msgid, LDAP_MSG_ONE, tval, &context.result) > 0);

  errorNumber = ldap_result2error(ldapContext, context.result, FALSE);
  if (errorNumber == 0)
    errorNumber = LDAP_OTHER;
  return FALSE;
}


PList<PStringToString> PLDAPSession::Search(const PString & filter,
                                            const PStringArray & attributes,
                                            const PString & base,
                                            SearchScope scope)
{
  PList<PStringToString> data;

  SearchContext context;
  if (!Search(context, filter, attributes, base, scope))
    return data;

  do {
    PStringToString * entry = new PStringToString;
    if (GetSearchResult(context, *entry))
      data.Append(entry);
    else {
      delete entry;
      break;
    }
  } while (GetNextSearchResult(context));

  return data;
}


PString PLDAPSession::GetErrorText() const
{
  return ldap_err2string(errorNumber);
}


///////////////////////////////////////////////////////////////////////////////

PLDAPAttributeBase::PLDAPAttributeBase(const char * n, void * ptr, PINDEX sz)
  : name(n),
    pointer(ptr),
    size(sz)
{
  PLDAPStructBase::GetInitialiser().AddAttribute(this);
}


PString PLDAPAttributeBase::ToString() const
{
  PStringStream stream;
  PrintOn(stream);
  return stream;
}


void PLDAPAttributeBase::FromString(const PString & str)
{
  PStringStream stream(str);
  ReadFrom(stream);
}


PBYTEArray PLDAPAttributeBase::ToBinary() const
{
  return PBYTEArray((const BYTE *)pointer, size, FALSE);
}


void PLDAPAttributeBase::FromBinary(const PArray<PBYTEArray> & data)
{
  if (data.GetSize() > 0 && data[0].GetSize() == size)
    memcpy(pointer, data[0], size);
}


///////////////////////////////////////////////////////////////////////////////

PMutex            PLDAPStructBase::initialiserMutex;
PLDAPStructBase * PLDAPStructBase::initialiserInstance;

PLDAPStructBase::PLDAPStructBase()
{
  attributes.DisallowDeleteObjects();

  initialiserMutex.Wait();
  initialiserStack = initialiserInstance;
  initialiserInstance = this;
}


PLDAPStructBase & PLDAPStructBase::operator=(const PLDAPStructBase & other)
{
  for (PINDEX i = 0; i < attributes.GetSize(); i++)
    attributes.GetDataAt(i).Copy(other.attributes.GetDataAt(i));

  return *this;
}


PLDAPStructBase & PLDAPStructBase::operator=(const PStringArray & array)
{
  for (PINDEX i = 0; i < array.GetSize(); i++) {
    PString str = array[i];
    PINDEX equal = str.Find('=');
    if (equal != P_MAX_INDEX) {
      PLDAPAttributeBase * attr = GetAttribute(str.Left(equal));
      if (attr != NULL)
        attr->FromString(str.Mid(equal+1));
    }
  }
  return *this;
}


PLDAPStructBase & PLDAPStructBase::operator=(const PStringToString & dict)
{
  for (PINDEX i = 0; i < dict.GetSize(); i++) {
    PLDAPAttributeBase * attr = GetAttribute(dict.GetKeyAt(i));
    if (attr != NULL)
      attr->FromString(dict.GetDataAt(i));
  }
  return *this;
}


void PLDAPStructBase::PrintOn(ostream & strm) const
{
  strm << attributes << '\n';
}


void PLDAPStructBase::AddAttribute(PLDAPAttributeBase * attr)
{
  attributes.SetAt(attr->GetName(), attr);
}


void PLDAPStructBase::EndConstructor()
{
  initialiserInstance = initialiserStack;
  initialiserMutex.Signal();
}


#endif // P_LDAP


// End of file ////////////////////////////////////////////////////////////////
