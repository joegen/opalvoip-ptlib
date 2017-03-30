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
 */

#ifdef __GNUC__
#pragma implementation "pldap.h"
#endif

#include <ptlib.h>

#include <ptlib/sockets.h>
#include <ptclib/pldap.h>

#define new PNEW


#if P_LDAP

#define LDAP_DEPRECATED 1
#include <ldap.h>


#if defined(_MSC_VER)
  #pragma comment(lib, P_LDAP_LIBRARY)
#endif

///////////////////////////////////////////////////////////////////////////////

PLDAPSession::PLDAPSession(const PString & baseDN)
  : m_ldapContext(NULL),
    m_errorNumber(LDAP_SUCCESS),
    m_protocolVersion(LDAP_VERSION3),
    m_defaultBaseDN(baseDN),
    m_searchLimit(0),
    m_timeout(0, 30),
    m_multipleValueSeparator('\n')
{
}


PLDAPSession::~PLDAPSession()
{
  Close();
}


PBoolean PLDAPSession::Open(const PString & server, WORD port)
{
  Close();

  PString host = server;
  PINDEX colon = server.Find(':');
  if (colon != P_MAX_INDEX) {
    host = server.Left(colon);
    port = PIPSocket::GetPortByService(server.Mid(colon+1), "tcp");
  }

  m_ldapContext = ldap_init(server, port);
  if (!IsOpen())
    return false;

  SetOption(LDAP_OPT_PROTOCOL_VERSION, m_protocolVersion);
  return true;
}


PBoolean PLDAPSession::Close()
{
  if (!IsOpen())
    return false;

  ldap_unbind(m_ldapContext);
  m_ldapContext = NULL;
  return true;
}


PBoolean PLDAPSession::SetOption(int optcode, int value)
{
  if (!IsOpen())
    return false;

  return ldap_set_option(m_ldapContext, optcode, &value);
}


PBoolean PLDAPSession::SetOption(int optcode, void * value)
{
  if (!IsOpen())
    return false;

  return ldap_set_option(m_ldapContext, optcode, value);
}

PBoolean PLDAPSession::StartTLS()
{
#ifdef LDAP_EXOP_START_TLS
  m_errorNumber = ldap_start_tls_s(m_ldapContext, NULL, NULL);
  return m_errorNumber == LDAP_SUCCESS;
#else
  return LDAP_OPERATIONS_ERROR;
#endif
}

PBoolean PLDAPSession::Bind(const PString & who,
                        const PString & passwd,
                        AuthenticationMethod authMethod)
{
  if (!IsOpen())
    return false;

  const char * whoPtr;
  if (who.IsEmpty())
    whoPtr = NULL;
  else
    whoPtr = who;

#ifdef SOLARIS
  static const int AuthMethodCode[NumAuthenticationMethod2] = {
    LDAP_AUTH_SIMPLE, LDAP_AUTH_SASL, LDAP_AUTH_KRBV41_30, LDAP_AUTH_KRBV42_30
#else
  static const int AuthMethodCode[NumAuthenticationMethod] = {
    LDAP_AUTH_SIMPLE, LDAP_AUTH_SASL, LDAP_AUTH_KRBV4
#endif
  };
  m_errorNumber = ldap_bind_s(m_ldapContext, whoPtr, passwd, AuthMethodCode[authMethod]);
  return m_errorNumber == LDAP_SUCCESS;
}


PLDAPSession::ModAttrib::ModAttrib(const PString & n, Operation o)
  : m_name(n),
    m_op(o)
{
}


void PLDAPSession::ModAttrib::SetLDAPMod(struct ldapmod & mod, Operation defaultOp)
{
  mod.mod_type = (char *)(const char *)m_name;

  Operation realOp = m_op == NumOperations ? defaultOp : m_op;
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
    m_values(vals)
{
}


void PLDAPSession::StringModAttrib::SetValue(const PString & value)
{
  m_values.RemoveAll();
  m_values.AppendString(value);
}


void PLDAPSession::StringModAttrib::AddValue(const PString & value)
{
  m_values.AppendString(value);
}


PBoolean PLDAPSession::StringModAttrib::IsBinary() const
{
  return false;
}


void PLDAPSession::StringModAttrib::SetLDAPModVars(struct ldapmod & mod)
{
  m_pointers.SetSize(m_values.GetSize()+1);
  PINDEX i;
  for (i = 0; i < m_values.GetSize(); i++)
    m_pointers[i] = m_values[i];
  m_pointers[i] = NULL;
  mod.mod_values = (char **)m_pointers.GetPointer();
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
                                               const PArray<PBYTEArray> & vals,
                                               Operation op)
  : ModAttrib(name, op),
    m_values(vals)
{
}


void PLDAPSession::BinaryModAttrib::SetValue(const PBYTEArray & value)
{
  m_values.RemoveAll();
  m_values.Append(new PBYTEArray(value));
}


void PLDAPSession::BinaryModAttrib::AddValue(const PBYTEArray & value)
{
  m_values.Append(new PBYTEArray(value));
}


PBoolean PLDAPSession::BinaryModAttrib::IsBinary() const
{
  return true;
}


void PLDAPSession::BinaryModAttrib::SetLDAPModVars(struct ldapmod & mod)
{
  m_pointers.SetSize(m_values.GetSize()+1);
  m_bervals.SetSize(m_values.GetSize()*sizeof(berval));
  berval * ber = (berval *)m_bervals.GetPointer();
  PINDEX i;
  for (i = 0; i < m_values.GetSize(); i++) {
    ber[i].bv_val = (char *)m_values[i].GetPointer();
    ber[i].bv_len = m_values[i].GetSize();
    m_pointers[i] = &ber[i];
  }
  m_pointers[i] = NULL;
  mod.mod_bvalues = m_pointers.GetPointer();
}


static LDAPMod ** CreateLDAPModArray(const PArray<PLDAPSession::ModAttrib> & attributes,
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


static PArray<PLDAPSession::ModAttrib> AttribsFromDict(const PStringToString & attributes)
{
  PArray<PLDAPSession::ModAttrib> attrs(attributes.GetSize());

  PINDEX index = 0;
  for (PStringToString::const_iterator it = attributes.begin(); it != attributes.end(); ++it)
    attrs.SetAt(index++, new PLDAPSession::StringModAttrib(it->first, it->second.Lines()));

  return attrs;
}


static PArray<PLDAPSession::ModAttrib> AttribsFromArray(const PStringArray & attributes)
{
  PArray<PLDAPSession::ModAttrib> attrs;

  for (PINDEX i = 0; i < attributes.GetSize(); i++) {
    PString attr = attributes[i];
    PINDEX equal = attr.Find('=');
    if (equal != P_MAX_INDEX)
      attrs.Append(new PLDAPSession::StringModAttrib(attr.Left(equal),
                                                     attr.Mid(equal+1).Lines()));
  }

  return attrs;
}


static PArray<PLDAPSession::ModAttrib> AttribsFromStruct(const PLDAPStructBase & attributes)
{
  PArray<PLDAPSession::ModAttrib> attrs;

  for (PLDAPStructBase::AttribDict::const_iterator it = attributes.GetAttributes().begin(); it != attributes.GetAttributes().end(); ++it) {
    const PLDAPAttributeBase & attr = it->second;
    if (attr.IsBinary())
      attrs.Append(new PLDAPSession::BinaryModAttrib(attr.GetName(), attr.ToBinary()));
    else {
      PString str = attr.ToString();
      if (!str)
        attrs.Append(new PLDAPSession::StringModAttrib(attr.GetName(), str));
    }
  }

  return attrs;
}


PBoolean PLDAPSession::Add(const PString & dn, const PArray<ModAttrib> & attributes)
{
  if (!IsOpen())
    return false;

  PBYTEArray storage;
  int msgid;
  m_errorNumber = ldap_add_ext(m_ldapContext,
                             dn,
                             CreateLDAPModArray(attributes, ModAttrib::Add, storage),
                             NULL,
                             NULL,
                             &msgid);
  if (m_errorNumber != LDAP_SUCCESS)
    return false;

  P_timeval tval = m_timeout;
  LDAPMessage * result = NULL;
  ldap_result(m_ldapContext, msgid, LDAP_MSG_ALL, tval, &result);
  if (result)
    m_errorNumber = ldap_result2error(m_ldapContext, result, true);

  return m_errorNumber == LDAP_SUCCESS;
}


PBoolean PLDAPSession::Add(const PString & dn, const PStringToString & attributes)
{
  return Add(dn, AttribsFromDict(attributes));
}


PBoolean PLDAPSession::Add(const PString & dn, const PStringArray & attributes)
{
  return Add(dn, AttribsFromArray(attributes));
}


PBoolean PLDAPSession::Add(const PString & dn, const PLDAPStructBase & attributes)
{
  return Add(dn, AttribsFromStruct(attributes));
}


PBoolean PLDAPSession::Modify(const PString & dn, const PArray<ModAttrib> & attributes)
{
  if (!IsOpen())
    return false;

  PBYTEArray storage;
  int msgid;
  m_errorNumber = ldap_modify_ext(m_ldapContext,
                                dn,
                                CreateLDAPModArray(attributes, ModAttrib::Replace, storage),
                                NULL,
                                NULL,
                                &msgid);
  if (m_errorNumber != LDAP_SUCCESS)
    return false;

  P_timeval tval = m_timeout;
  LDAPMessage * result = NULL;
  ldap_result(m_ldapContext, msgid, LDAP_MSG_ALL, tval, &result);
  if (result)
    m_errorNumber = ldap_result2error(m_ldapContext, result, true);

  return m_errorNumber == LDAP_SUCCESS;
}


PBoolean PLDAPSession::Modify(const PString & dn, const PStringToString & attributes)
{
  return Modify(dn, AttribsFromDict(attributes));
}


PBoolean PLDAPSession::Modify(const PString & dn, const PStringArray & attributes)
{
  return Modify(dn, AttribsFromArray(attributes));
}


PBoolean PLDAPSession::Modify(const PString & dn, const PLDAPStructBase & attributes)
{
  return Modify(dn, AttribsFromStruct(attributes));
}


PBoolean PLDAPSession::Delete(const PString & dn)
{
  if (!IsOpen())
    return false;

  int msgid;
  m_errorNumber = ldap_delete_ext(m_ldapContext, dn, NULL, NULL, &msgid);
  if (m_errorNumber != LDAP_SUCCESS)
    return false;

  P_timeval tval = m_timeout;
  LDAPMessage * result = NULL;
  ldap_result(m_ldapContext, msgid, LDAP_MSG_ALL, tval, &result);
  if (result)
    m_errorNumber = ldap_result2error(m_ldapContext, result, true);

  return m_errorNumber == LDAP_SUCCESS;
}


PLDAPSession::SearchContext::SearchContext()
{
  m_result = NULL;
  m_message = NULL;
  m_found = false;
  m_completed = false;
}


PLDAPSession::SearchContext::~SearchContext()
{
  if (m_message != NULL)
    ldap_msgfree(m_message);

  if (m_result != NULL && m_result != m_message)
    ldap_msgfree(m_result);
}


PBoolean PLDAPSession::Search(SearchContext & context,
                          const PString & filter,
                          const PStringArray & attributes,
                          const PString & baseDN,
                          SearchScope scope)
{
  if (!IsOpen())
    return false;

  PCharArray storage;
  char ** attribs = attributes.ToCharArray(&storage);

  PString base = baseDN;
  if (base.IsEmpty())
    base = m_defaultBaseDN;

  static const int ScopeCode[NumSearchScope] = {
    LDAP_SCOPE_BASE, LDAP_SCOPE_ONELEVEL, LDAP_SCOPE_SUBTREE
  };

  P_timeval tval = m_timeout;

  m_errorNumber = ldap_search_ext(m_ldapContext,
                                base,
                                ScopeCode[scope],
                                filter,
                                attribs,
                                false,
                                NULL,
                                NULL,
                                tval,
                                m_searchLimit,
                                &context.m_msgid);

  if (m_errorNumber != LDAP_SUCCESS)
    return false;

  if (ldap_result(m_ldapContext, context.m_msgid, LDAP_MSG_ONE, tval, &context.m_result) > 0)
    return GetNextSearchResult(context);

  if (context.m_result)
    m_errorNumber = ldap_result2error(m_ldapContext, context.m_result, true);
  if (m_errorNumber == 0)
    m_errorNumber = LDAP_OTHER;
  return false;
}


PBoolean PLDAPSession::GetSearchResult(SearchContext & context, PStringToString & data)
{
  data.RemoveAll();

  if (!IsOpen())
    return false;

  if (context.m_result == NULL || context.m_message == NULL || context.m_completed)
    return false;

  // Extract the resulting data

  data.SetAt("dn", GetSearchResultDN(context));

  BerElement * ber = NULL;
  char * attrib = ldap_first_attribute(m_ldapContext, context.m_message, &ber);
  while (attrib != NULL) {

    struct berval ** bvals = ldap_get_values_len(m_ldapContext, context.m_message, attrib);
    if (bvals != NULL) {
      PString value = data(attrib);

      for (PINDEX i = 0; bvals[i] != NULL; i++ ) {
        if (!value)
          value += m_multipleValueSeparator;
        value += PString(bvals[i]->bv_val, bvals[i]->bv_len);
      }
      ber_bvecfree(bvals);

      data.SetAt(attrib, value);
    }

    ldap_memfree(attrib);
    attrib = ldap_next_attribute(m_ldapContext, context.m_message, ber);
  }

  if (ber != NULL)
    ber_free (ber, 0);

  return true;
}


PBoolean PLDAPSession::GetSearchResult(SearchContext & context,
                                   const PString & attribute,
                                   PString & data)
{
  data.MakeEmpty();

  if (!IsOpen())
    return false;

  if (context.m_result == NULL || context.m_message == NULL || context.m_completed)
    return false;

  if (attribute == "dn") {
    data = GetSearchResultDN(context);
    return true;
  }

  char ** values = ldap_get_values(m_ldapContext, context.m_message, attribute);
  if (values == NULL)
    return false;

  PINDEX count = ldap_count_values(values);
  for (PINDEX i = 0; i < count; i++) {
    if (!data)
      data += m_multipleValueSeparator;
    data += values[i];
  }

  ldap_value_free(values);
  return true;
}


PBoolean PLDAPSession::GetSearchResult(SearchContext & context,
                                   const PString & attribute,
                                   PStringArray & data)
{
  data.RemoveAll();

  if (!IsOpen())
    return false;

  if (context.m_result == NULL || context.m_message == NULL || context.m_completed)
    return false;

  if (attribute == "dn") {
    data.SetSize(1);
    data[0] = GetSearchResultDN(context);
    return true;
  }

  char ** values = ldap_get_values(m_ldapContext, context.m_message, attribute);
  if (values == NULL)
    return false;

  PINDEX count = ldap_count_values(values);
  data.SetSize(count);
  for (PINDEX i = 0; i < count; i++)
    data[i] = values[i];

  ldap_value_free(values);
  return true;
}


PBoolean PLDAPSession::GetSearchResult(SearchContext & context,
                                   const PString & attribute,
                                   PArray<PBYTEArray> & data)
{
  data.RemoveAll();

  if (!IsOpen())
    return false;

  if (attribute == "dn") {
    char * dn = ldap_get_dn(m_ldapContext, context.m_message);
    data.Append(new PBYTEArray((const BYTE *)dn, ::strlen(dn)));
    ldap_memfree(dn);
    return true;
  }

  struct berval ** values = ldap_get_values_len(m_ldapContext, context.m_message, attribute);
  if (values == NULL)
    return false;

  PINDEX count = ldap_count_values_len(values);
  data.SetSize(count);
  for (PINDEX i = 0; i < count; i++) {
    PBYTEArray* dataPtr = new PBYTEArray(values[i]->bv_len);
    data.SetAt(i, dataPtr);
    
    memcpy(data[i].GetPointer(), (const BYTE *)values[i]->bv_val, values[i]->bv_len); 
  }

  ldap_value_free_len(values);
  return true;
}


PBoolean PLDAPSession::GetSearchResult(SearchContext & context,
                                   PLDAPStructBase & data)
{
  if (!IsOpen())
    return false;

  PBoolean atLeastOne = false;

  for (PLDAPStructBase::AttribDict::iterator it = data.GetAttributes().begin(); it != data.GetAttributes().end(); ++it) {
    PLDAPAttributeBase & attr = it->second;
    if (attr.IsBinary()) {
      PArray<PBYTEArray> bin;
      if (GetSearchResult(context, attr.GetName(), bin)) {
        attr.FromBinary(bin);
        atLeastOne = true;
      }
    }
    else {
      PString str;
      if (GetSearchResult(context, attr.GetName(), str)) {
        attr.FromString(str);
        atLeastOne = true;
      }
    }
  }

  return atLeastOne;
}


PString PLDAPSession::GetSearchResultDN(SearchContext & context)
{
  PString str;

  if (context.m_message != NULL) {
    char * dn = ldap_get_dn(m_ldapContext, context.m_message);
    if (dn != NULL) {
      str = dn;
      ldap_memfree(dn);
    }
  }

  return str;
}


PBoolean PLDAPSession::GetNextSearchResult(SearchContext & context)
{
  if (!IsOpen())
    return false;

  if (context.m_result == NULL || context.m_completed)
    return false;

  P_timeval tval = m_timeout;
  do {
    if (context.m_message == NULL)
      context.m_message = ldap_first_message(m_ldapContext, context.m_result);
    else
      context.m_message = ldap_next_message(m_ldapContext, context.m_message);

    if (context.m_message != NULL) {
      switch (ldap_msgtype(context.m_message)) {
        case LDAP_RES_SEARCH_ENTRY :
          context.m_found = true;
          m_errorNumber = LDAP_SUCCESS;
          return true;

        case LDAP_RES_SEARCH_RESULT :
          m_errorNumber = ldap_result2error(m_ldapContext, context.m_message, false);
          if (m_errorNumber == 0 && !context.m_found)
            m_errorNumber = LDAP_NO_RESULTS_RETURNED;
          context.m_completed = true;
          return false;
        case LDAP_RES_SEARCH_REFERENCE :
          m_errorNumber = LDAP_SUCCESS;
          return true;
        // Ignore other result message types for now ...
        default:
          PTRACE(3, "Unhandled LDAP message type " << ldap_msgtype(context.m_message));
      }
    }

    ldap_msgfree(context.m_result);
  } while (ldap_result(m_ldapContext, context.m_msgid, LDAP_MSG_ONE, tval, &context.m_result) > 0);

  if (context.m_result)
    m_errorNumber = ldap_result2error(m_ldapContext, context.m_result, false);
  if (m_errorNumber == 0)
    m_errorNumber = LDAP_OTHER;
  return false;
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
  return ldap_err2string(m_errorNumber);
}


///////////////////////////////////////////////////////////////////////////////

PLDAPAttributeBase::PLDAPAttributeBase(const char * n, void * ptr, PINDEX sz)
  : m_name(n),
    m_pointer(ptr),
    m_size(sz)
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
  return PBYTEArray((const BYTE *)m_pointer, m_size, false);
}


void PLDAPAttributeBase::FromBinary(const PArray<PBYTEArray> & data)
{
  if (data.GetSize() > 0 && data[0].GetSize() == m_size)
    memcpy(m_pointer, data[0], m_size);
}


///////////////////////////////////////////////////////////////////////////////

PMutex            PLDAPStructBase::m_initialiserMutex;
PLDAPStructBase * PLDAPStructBase::m_initialiserInstance;

PLDAPStructBase::PLDAPStructBase()
{
  m_attributes.DisallowDeleteObjects();

  m_initialiserMutex.Wait();
  m_initialiserStack = m_initialiserInstance;
  m_initialiserInstance = this;
}


PLDAPStructBase & PLDAPStructBase::operator=(const PLDAPStructBase & other)
{
  for (AttribDict::iterator it = m_attributes.begin(); it != m_attributes.end(); ++it) {
    PLDAPAttributeBase * otherAttrib = other.m_attributes.GetAt(it->first);
    if (otherAttrib != NULL)
      it->second.Copy(*otherAttrib);
  }

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
  for (PStringToString::const_iterator it = dict.begin(); it != dict.end(); ++it) {
    PLDAPAttributeBase * attr = GetAttribute(it->first);
    if (attr != NULL)
      attr->FromString(it->second);
  }
  return *this;
}


void PLDAPStructBase::PrintOn(ostream & strm) const
{
  strm << m_attributes << '\n';
}


void PLDAPStructBase::AddAttribute(PLDAPAttributeBase * attr)
{
  m_attributes.SetAt(attr->GetName(), attr);
}


void PLDAPStructBase::EndConstructor()
{
  m_initialiserInstance = m_initialiserStack;
  m_initialiserMutex.Signal();
}

///////////////////////////////////////////////////////////////////

static const char PLDAPSchemaPluginBaseClass[] = "PLDAPSchema";

PLDAPSchema::PLDAPSchema()
{
}

PLDAPSchema::Attribute::Attribute(const PString & name, AttributeType type)
: m_name(name),m_type(type)
{
}

void PLDAPSchema::LoadSchema()
{
	AttributeList(m_attributelist); 
}

PLDAPSchema * PLDAPSchema::CreateSchema(const PString & schemaName, PPluginManager * pluginMgr)
{
  return PPluginManager::CreatePluginAs<PLDAPSchema>(pluginMgr, schemaName, PLDAPSchemaPluginBaseClass);
}

PStringArray PLDAPSchema::GetSchemaNames(PPluginManager * pluginMgr)
{
  return PPluginManager::GetPluginsProviding(pluginMgr, PLDAPSchemaPluginBaseClass, false);
}

PStringArray PLDAPSchema::GetSchemaFriendlyNames(PPluginManager * pluginMgr)
{
  return PPluginManager::GetPluginsProviding(pluginMgr, PLDAPSchemaPluginBaseClass, true);
}

void PLDAPSchema::OnReceivedAttribute(const PString & attribute, const PString & value)
{
   SetAttribute(attribute,value);
}

PBoolean PLDAPSchema::SetAttribute(const PString & attribute, const PString & value)
{

	for (std::list<Attribute>::const_iterator r = m_attributelist.begin(); r != m_attributelist.end(); ++r) {
		if ((r->m_name == attribute) &&  (r->m_type != AttributeBinary)) {
	       m_attributes.insert(make_pair(attribute,value));
		   PTRACE(4, "schema\tMatch " << attribute);
		   return true;
		}
	}

	return false;
}

PBoolean PLDAPSchema::SetAttribute(const PString & attribute, const PBYTEArray & value)
{
	for (std::list<Attribute>::const_iterator r = m_attributelist.begin(); r != m_attributelist.end(); ++r) {
		if ((r->m_name == attribute) && (r->m_type == AttributeBinary)) {
	       m_binattributes.insert(make_pair(attribute,value));
		   PTRACE(4, "schema\tMatch Binary " << attribute);
		   return true;
		}
	}

	return false;
}

PBoolean PLDAPSchema::GetAttribute(const PString & attribute, PString & value)
{
	for (ldapAttributes::const_iterator r = m_attributes.begin(); r != m_attributes.end(); ++r) {
		if (r->first == attribute) {
			value = r->second;
			return true;
		}
	}
	return false; 
}

PBoolean PLDAPSchema::GetAttribute(const PString & attribute, PBYTEArray & value)
{
	for (ldapBinAttributes::const_iterator r = m_binattributes.begin(); r != m_binattributes.end(); ++r) {
		if (r->first == attribute) {
			value = r->second;
			return true;
		}
	}
	return false; 
}

PStringList PLDAPSchema::GetAttributeList()
{
	PStringList att;
  	for (std::list<Attribute>::iterator r = m_attributelist.begin(); r != m_attributelist.end(); ++r) {
        att.AppendString(r->m_name);
	}   
	return att;
}

PBoolean PLDAPSchema::Exists(const PString & attribute)
{
	for (std::list<Attribute>::const_iterator itList = m_attributelist.begin(); itList != m_attributelist.end(); ++itList) {
	  if (itList->m_name == attribute) {
		  if (itList->m_type == AttributeString) {
	          for (ldapAttributes::const_iterator itAttr = m_attributes.begin(); itAttr != m_attributes.end(); ++itAttr) {
                  if (itAttr->first == attribute)
			         return true;
		      }
		  } else if (itList->m_type == AttributeBinary) {
	          for (ldapBinAttributes::const_iterator itBinattr = m_binattributes.begin(); itBinattr != m_binattributes.end(); ++itBinattr) {
                  if (itBinattr->first == attribute)
			         return true;
			  }
		  }
	  }
	}
	return false;
}

PLDAPSchema::AttributeType PLDAPSchema::GetAttributeType(const PString & attribute)
{
	for (std::list<Attribute>::const_iterator r = m_attributelist.begin(); r != m_attributelist.end(); ++r) {
		if (r->m_name == attribute) 
		   return (AttributeType)r->m_type;
	}
	return AttibuteUnknown;
}

void PLDAPSchema::OnSendSchema(PArray<PLDAPSession::ModAttrib> & attrib, PLDAPSession::ModAttrib::Operation op)
{
	for (ldapAttributes::const_iterator r = m_attributes.begin(); r != m_attributes.end(); ++r) 
        attrib.Append(new PLDAPSession::StringModAttrib(r->first,r->second,op));

	for (ldapBinAttributes::const_iterator s = m_binattributes.begin(); s != m_binattributes.end(); ++s) {
        attrib.Append(new PLDAPSession::BinaryModAttrib(s->first,s->second,op));
	}
}
#endif // P_LDAP
