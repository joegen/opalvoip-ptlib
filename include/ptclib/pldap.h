/*
 * pldap.h
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

#ifndef PTLIB_PLDAP_H
#define PTLIB_PLDAP_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#if defined(P_LDAP)

#include <ptlib/sockets.h>
#include <ptlib/pluginmgr.h>
#include <map>
#include <list>

struct ldap;
struct ldapmsg;
struct ldapmod;
struct berval;

class PLDAPStructBase;


/**This class will create an LDAP client to access a remote LDAP server.
  */
class PLDAPSession : public PObject
{
  PCLASSINFO(PLDAPSession, PObject);
  public:
    /**Create a LDAP client.
      */
    PLDAPSession(
      const PString & defaultBaseDN = PString::Empty()
    );

    /**Close the sesison on destruction
      */
    ~PLDAPSession();

    /**Open the LDAP session to the specified server.
       The server name string is of the form hostip[:port] where hostip is a
       host name or IP address and the :port is an optional port number. If
       not present then the port variable is used. If that is also zero then
       the default port of 369 is used.
      */
    PBoolean Open(
      const PString & server,
      WORD port = 0
    );

    /**Close the LDAP session
      */
    PBoolean Close();

    /**Determine of session is open.
      */
    PBoolean IsOpen() const { return m_ldapContext != NULL; }

    /**Set LDAP option parameter (OpenLDAp specific values)
      */
    PBoolean SetOption(
      int optcode,
      int value
    );

    /**Set LDAP option parameter (OpenLDAP specific values)
      */
    PBoolean SetOption(
      int optcode,
      void * value
    );

    enum AuthenticationMethod {
      AuthSimple,
      AuthSASL,
      AuthKerberos,
#ifdef SOLARIS
      NumAuthenticationMethod1,
      NumAuthenticationMethod2
#else
      NumAuthenticationMethod
#endif
    };

    /**Start encrypted connection
      */
    PBoolean StartTLS();

    /**Bind to the remote LDAP server.
      */
    PBoolean Bind(
      const PString & who = PString::Empty(),
      const PString & passwd = PString::Empty(),
      AuthenticationMethod authMethod = AuthSimple
    );

    class ModAttrib : public PObject {
        PCLASSINFO(ModAttrib, PObject);
      public:
        enum Operation {
          Add,
          Replace,
          Delete,
          NumOperations
        };

      protected:
        ModAttrib(
          const PString & name,
          Operation op = NumOperations
        );

      public:
        const PString & GetName() const { return m_name; }

        Operation GetOperation() const { return m_op; }

        void SetLDAPMod(
          struct ldapmod & mod,
          Operation defaultOp
        );

      protected:
        virtual PBoolean IsBinary() const = 0;
        virtual void SetLDAPModVars(struct ldapmod & mod) = 0;

        PString   m_name;
        Operation m_op;
    };

    class StringModAttrib : public ModAttrib {
        PCLASSINFO(StringModAttrib, ModAttrib);
      public:
        StringModAttrib(
          const PString & name,
          Operation op = NumOperations
        );
        StringModAttrib(
          const PString & name,
          const PString & value,
          Operation op = NumOperations
        );
        StringModAttrib(
          const PString & name,
          const PStringList & values,
          Operation op = NumOperations
        );
        void SetValue(
          const PString & value
        );
        void AddValue(
          const PString & value
        );
      protected:
        virtual PBoolean IsBinary() const;
        virtual void SetLDAPModVars(struct ldapmod & mod);

        PStringArray m_values;
        PBaseArray<const char *> m_pointers;
    };

    class BinaryModAttrib : public ModAttrib {
        PCLASSINFO(BinaryModAttrib, ModAttrib);
      public:
        BinaryModAttrib(
          const PString & name,
          Operation op = Add
        );
        BinaryModAttrib(
          const PString & name,
          const PBYTEArray & value,
          Operation op = Add
        );
        BinaryModAttrib(
          const PString & name,
          const PArray<PBYTEArray> & values,
          Operation op = Add
        );
        void SetValue(
          const PBYTEArray & value
        );
        void AddValue(
          const PBYTEArray & value
        );
      protected:
        virtual PBoolean IsBinary() const;
        virtual void SetLDAPModVars(struct ldapmod & mod);

        PArray<PBYTEArray> m_values;
        PBaseArray<struct berval *> m_pointers;
        PBYTEArray m_bervals;
    };

    /**Add a new distringuished name to LDAP dirctory.
      */
    PBoolean Add(
      const PString & dn,
      const PArray<ModAttrib> & attributes
    );

    /**Add a new distringuished name to LDAP dirctory.
      */
    PBoolean Add(
      const PString & dn,
      const PStringToString & attributes
    );

    /**Add a new distringuished name to LDAP dirctory.
       The attributes list is a string array of the form attr=value
      */
    PBoolean Add(
      const PString & dn,
      const PStringArray & attributes
    );

    /**Add a new distringuished name to LDAP dirctory.
       The attributes list is a string array of the form attr=value
      */
    PBoolean Add(
      const PString & dn,
      const PLDAPStructBase & data
    );

    /**Modify an existing distringuished name to LDAP dirctory.
      */
    PBoolean Modify(
      const PString & dn,
      const PArray<ModAttrib> & attributes
    );

    /**Add a new distringuished name to LDAP dirctory.
      */
    PBoolean Modify(
      const PString & dn,
      const PStringToString & attributes
    );

    /**Add a new distringuished name to LDAP dirctory.
       The attributes list is a string array of the form attr=value
      */
    PBoolean Modify(
      const PString & dn,
      const PStringArray & attributes
    );

    /**Add a new distringuished name to LDAP dirctory.
       The attributes list is a string array of the form attr=value
      */
    PBoolean Modify(
      const PString & dn,
      const PLDAPStructBase & data
    );

    /**Delete the distinguished name from LDAP directory.
      */
    PBoolean Delete(
      const PString & dn
    );


    enum SearchScope {
      ScopeBaseOnly,
      ScopeSingleLevel,
      ScopeSubTree,
      NumSearchScope
    };

    class SearchContext {
      public:
        SearchContext();
        ~SearchContext();

        PBoolean IsCompleted() const { return m_completed; }

      private:
        int              m_msgid;
        struct ldapmsg * m_result;
        struct ldapmsg * m_message;
        bool             m_found;
        bool             m_completed;

      friend class PLDAPSession;
    };

    /**Start search for specified information.
      */
    PBoolean Search(
      SearchContext & context,
      const PString & filter,
      const PStringArray & attributes = PStringList(),
      const PString & base = PString::Empty(),
      SearchScope scope = ScopeSubTree
    );

    /**Get the current search result entry.
      */
    PBoolean GetSearchResult(
      SearchContext & context,
      PStringToString & data
    );

    /**Get an attribute of the current search result entry.
      */
    PBoolean GetSearchResult(
      SearchContext & context,
      const PString & attribute,
      PString & data
    );

    /**Get an attribute of the current search result entry.
      */
    PBoolean GetSearchResult(
      SearchContext & context,
      const PString & attribute,
      PStringArray & data
    );

    /**Get an attribute of the current search result entry.
      */
    PBoolean GetSearchResult(
      SearchContext & context,
      const PString & attribute,
      PArray<PBYTEArray> & data
    );

    /**Get all attributes of the current search result entry.
      */
    PBoolean GetSearchResult(
      SearchContext & context,
      PLDAPStructBase & data
    );

    /**Get the current search result distinguished name entry.
      */
    PString GetSearchResultDN(
      SearchContext & context
    );

    /**Get the next search result.
      */
    PBoolean GetNextSearchResult(
      SearchContext & context
    );

    /**Search for specified information, returning all matches.
       This can be used for simple LDAP databases where all attributes are
       represented by a string.
      */
    PList<PStringToString> Search(
      const PString & filter,
      const PStringArray & attributes = PStringList(),
      const PString & base = PString::Empty(),
      SearchScope scope = ScopeSubTree
    );


    /**Set the default base DN for use if not specified for searches.
      */
    void SetBaseDN(
      const PString & dn
    ) { m_defaultBaseDN = dn; }

    /**Set the default base DN for use if not specified for searches.
      */
    const PString & GetBaseDN() const { return m_defaultBaseDN; }

    /**Get the last OpenLDAP error code.
      */
    int GetErrorNumber() const { return m_errorNumber; }

    /**Get the last OpenLDAP error as text string.
      */
    PString GetErrorText() const;

    /**Get the OpenLDAP context structure.
      */
    struct ldap * GetOpenLDAP() const { return m_ldapContext; }

    /**Get the timeout for LDAP operations.
      */
    const PTimeInterval & GetTimeout() const { return m_timeout; }

    /**Set the timeout for LDAP operations.
      */
    void SetTimeout(
      const PTimeInterval & t
    ) { m_timeout = t; }

    /**Set a limit on the number of results to return
      */
     void SetSearchLimit(
      const unsigned s
    ) { m_searchLimit = s; }

  protected:
    struct ldap * m_ldapContext;
    int           m_errorNumber;
    unsigned      m_protocolVersion;
    PString       m_defaultBaseDN;
    unsigned      m_searchLimit;
    PTimeInterval m_timeout;
    PString       m_multipleValueSeparator;
};



class PLDAPStructBase;

class PLDAPAttributeBase : public PObject
{
    PCLASSINFO(PLDAPAttributeBase, PObject);
  public:
    PLDAPAttributeBase(const char * name, void * pointer, PINDEX size);

    const char * GetName() const { return m_name; }
    PBoolean IsBinary() const { return m_pointer != NULL; }

    virtual void Copy(const PLDAPAttributeBase & other) = 0;

    virtual PString ToString() const;
    virtual void FromString(const PString & str);
    virtual PBYTEArray ToBinary() const;
    virtual void FromBinary(const PArray<PBYTEArray> & data);

  protected:
    const char * m_name;
    void       * m_pointer;
    PINDEX       m_size;
};


class PLDAPStructBase : public PObject {
    PCLASSINFO(PLDAPStructBase, PObject);
  protected:
    PLDAPStructBase();
    PLDAPStructBase & operator=(const PLDAPStructBase &);
    PLDAPStructBase & operator=(const PStringArray & array);
    PLDAPStructBase & operator=(const PStringToString & dict);
  private:
    PLDAPStructBase(const PLDAPStructBase & obj) : PObject(obj) { }

  public:
    void PrintOn(ostream & strm) const;

    typedef PDictionary<PString, PLDAPAttributeBase> AttribDict;
    AttribDict & GetAttributes() { return m_attributes; }
    const AttribDict & GetAttributes() const { return m_attributes; }
    PLDAPAttributeBase * GetAttribute(const char * name) const { return m_attributes.GetAt(name); }

    void AddAttribute(PLDAPAttributeBase * var);
    static PLDAPStructBase & GetInitialiser() { return *PAssertNULL(m_initialiserInstance); }

  protected:
    void EndConstructor();

    AttribDict m_attributes;

    PLDAPStructBase        * m_initialiserStack;
    static PMutex            m_initialiserMutex;
    static PLDAPStructBase * m_initialiserInstance;
};

///////////////////////////////////////////////////////////////////////////

class PLDAPSchema : public PObject
{
  public:
    PLDAPSchema();

    enum AttributeType {
        AttibuteUnknown = -1,
        AttributeString,
        AttributeBinary,
        AttributeNumeric
    };

    class Attribute
    {
    public:
        Attribute() : m_type(AttibuteUnknown) { }
        Attribute(const PString & name, AttributeType type);
        PString       m_name;
        AttributeType m_type;
    };

    typedef std::list<Attribute> attributeList;

    static PLDAPSchema * CreateSchema(const PString & schemaname, PPluginManager * pluginMgr = NULL);
    static PStringArray GetSchemaNames(PPluginManager * pluginMgr = NULL);
    static PStringArray GetSchemaFriendlyNames(PPluginManager * pluginMgr = NULL);

    void OnReceivedAttribute(const PString & attribute, const PString & value);

    void OnSendSchema(PArray<PLDAPSession::ModAttrib> & attributes,
        PLDAPSession::ModAttrib::Operation op=PLDAPSession::ModAttrib::Add);

    void LoadSchema();

    PStringList SchemaName() { return PStringList(); }
    virtual void AttributeList(attributeList & /*attrib*/) {};


    PStringList GetAttributeList();
    PBoolean Exists(const PString & attribute);

    PBoolean SetAttribute(const PString & attribute, const PString & value);
    PBoolean SetAttribute(const PString & attribute, const PBYTEArray & value);

    PBoolean GetAttribute(const PString & attribute, PString & value);
    PBoolean GetAttribute(const PString & attribute, PBYTEArray & value);

    AttributeType GetAttributeType(const PString & attribute);


  protected:
    typedef std::map<PString,PString> ldapAttributes;
    typedef std::map<PString,PBYTEArray> ldapBinAttributes;


    attributeList           m_attributelist;
    ldapAttributes          m_attributes;
    ldapBinAttributes       m_binattributes;   
};


PCREATE_PLUGIN_SERVICE(PLDAPSchema);

#define LDAP_Schema(name) PCREATE_PLUGIN(name, PLDAPSchema)


////////////////////////////////////////////////////////////////////////////////////////////

#define PLDAP_STRUCT_BEGIN(name) \
  class name : public PLDAPStructBase { \
    public: name() : PLDAPStructBase() { EndConstructor(); } \
    public: name(const name & other) : PLDAPStructBase() { EndConstructor(); operator=(other); } \
    public: name(const PStringArray & array) : PLDAPStructBase() { EndConstructor(); operator=(array); } \
    public: name(const PStringToString & dict) : PLDAPStructBase() { EndConstructor(); operator=(dict); } \
    public: name & operator=(const name & other) { PLDAPStructBase::operator=(other); return *this; } \
    public: name & operator=(const PStringArray & array) { PLDAPStructBase::operator=(array); return *this; } \
    public: name & operator=(const PStringToString & dict) { PLDAPStructBase::operator=(dict); return *this; } \
    PLDAP_ATTR_INIT(name, PString, objectClass, #name);

#define PLDAP_ATTRIBUTE(base, type, attribute, pointer, init) \
    public: type attribute; \
    private: struct PLDAPAttr_##attribute : public PLDAPAttributeBase { \
      PLDAPAttr_##attribute() \
        : PLDAPAttributeBase(#attribute, pointer, sizeof(type)), \
          instance(((base &)base::GetInitialiser()).attribute) \
        { init } \
      virtual void PrintOn (ostream & s) const { s << instance; } \
      virtual void ReadFrom(istream & s)       { s >> instance; } \
      virtual void Copy(const PLDAPAttributeBase & other) \
                    { instance = ((PLDAPAttr_##attribute &)other).instance; } \
      type & instance; \
    } pldapvar_##attribute

#define PLDAP_ATTR_SIMP(base, type, attribute) \
        PLDAP_ATTRIBUTE(base, type, attribute, NULL, ;)

#define PLDAP_ATTR_INIT(base, type, attribute, init) \
        PLDAP_ATTRIBUTE(base, type, attribute, NULL, instance = init;)

#define PLDAP_BINATTRIB(base, type, attribute) \
        PLDAP_ATTRIBUTE(base, type, attribute, &((base &)base::GetInitialiser()).attribute, ;)

#define PLDAP_STRUCT_END() \
  };

#endif // P_LDAP

#endif // PTLIB_PLDAP_H


// End of file ////////////////////////////////////////////////////////////////
