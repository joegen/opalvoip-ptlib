/*
* podbc.cxx
*
* Virteos ODBC Implementation for PWLib Library.
*
* Virteos is a Trade Mark of ISVO (Asia) Pte Ltd.
*
* Copyright (c) 2004 ISVO (Asia) Pte Ltd. All Rights Reserved.
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
*
* The Original Code is derived from and used in conjunction with the
* pwlib Libaray of the OpenH323 Project (www.openh323.org/)
*
* The Initial Developer of the Original Code is ISVO (Asia) Pte Ltd.
*
*   Portions: Simple ODBC Wrapper Article www.codeproject.com
*
* Contributor(s): ______________________________________.
*/

#include <ptlib.h>

#if defined(P_ODBC) && !defined(_WIN32_WCE)

#include <ptclib/podbc.h>
#include <ptclib/guid.h>


#include <odbcinst.h>
#include <sql.h> 
#include <sqlext.h>

#ifdef _MSC_VER
 #include <tchar.h>
 #pragma comment(lib,"odbc32.lib")
 #pragma comment(lib,"odbcCP32.lib")
#else

  #ifdef UNICODE
  typedef WCHAR                 TCHAR;
  typedef LPWSTR                LPTSTR;
  typedef LPCWSTR               LPCTSTR;
  // Needs a definition one day ... #define _T(x)
  #else
  typedef CHAR                  TCHAR;
  typedef LPSTR                 LPTSTR;
  typedef LPCSTR                LPCTSTR;
  #define _T(x) x
  #endif

#endif // _MSC_VER


class PODBC::Statement : public PObject
{
    PCLASSINFO(Statement, PObject);
  public:
    /**@name Constructor/Deconstructor */
    //@{
    /** Constructor PODBC (Datasources call) or thro' DSNConnection (Connection call). 
    In General this class is constructed within the PODBC::RecordSet Class.
    */
    Statement(PODBC & odbc);

    /** Deconstructor. This Class should be available for the duration of which
    a specific query/table is required and be deconstructed at the time of
    the PODBC::RecordSet deconstruction.
    */
    ~Statement();
    //@}

    /**@name Data Management */
    //@{ 
    /** IsValid Checks to ensure a Handle has been allocated and
    is effective.
    */
    __inline bool IsValid() const { return m_hStmt != SQL_NULL_HSTMT; }

    /** GetChangedRowCount retreives the number of rows updated/altered by
    UPDATE/INSERT statements.
    */
    DWORD GetChangedRowCount();

    /** Execute function is the Main function to pass SQL statements to retreive/
    add/Modify database data. It accepts generally acceptable SQL Statements.
    ie. Select * from [table-x]
    */
    bool Execute(const PString & sql);

    // Close cursor, remove all bindings.
    bool CloseCursor() { return SQL_OK(SQLCloseCursor(m_hStmt)); }
    //@}

    /**@name Data Retrieval */
    //@{  
    /** FetchRow More detailed fetching of Rows. This allows you to fetch an
    Absolute row or a row relative to the current row fetched.
    */
    bool FetchScroll(SQLSMALLINT orientation, SQLLEN offset = 0)
    { return SQL_OK(SQLFetchScroll(m_hStmt, orientation, offset)); }

    // Commit the data
    bool Commit(unsigned operation);

    /** Retreive the List of tables from the current Datasource
    The option field can be used to specify the Table Types
    ie "TABLE" for Tables or "VIEW" for preconfigured datasource
    queries. *Further investigation is required*
    */
    PStringArray TableList(const PString & options);

    /** Is the SQL Instruction OK
    If an Error is detected then GetLastError is called
    to Retrieve the SQL Error Information and Returns false
    */
    bool SQL_OK(SQLRETURN res);
    //@}

    __inline bool NumResultCols(SQLSMALLINT *columnCount) { return SQL_OK(SQLNumResultCols(m_hStmt, columnCount)); }

    __inline bool GetData(
      SQLUSMALLINT column,
      SQLSMALLINT type,
      SQLPOINTER data,
      SQLLEN size,
      SQLLEN * len
    ) { return SQL_OK(SQLGetData(m_hStmt, column, type, data, size, len)); }

    __inline bool BindCol(
      SQLUSMALLINT column,
      SQLSMALLINT type,
      SQLPOINTER data, 
      SQLLEN size,
      SQLLEN * len
      ) { return SQL_OK(SQLBindCol(m_hStmt, column, type, data, size, len)); }

    __inline bool DescribeCol(
      SQLUSMALLINT column,
      SQLCHAR *namePtr,
      SQLSMALLINT nameSize,
      SQLSMALLINT *nameLength,
      SQLSMALLINT *dataType,
      SQLULEN *columnSize,
      SQLSMALLINT *decimalDigits,
      SQLSMALLINT *nullable
    ) { return SQL_OK(SQLDescribeCol(m_hStmt, column, namePtr, nameSize, nameLength, dataType, columnSize, decimalDigits, nullable)); }

    __inline bool ColAttribute (
      SQLUSMALLINT column,
      SQLUSMALLINT field,
      SQLPOINTER attrPtr,
      SQLSMALLINT attrSize,
      SQLSMALLINT *length,
      SQLLEN * numericPtr
    ) { return SQL_OK(SQLColAttribute(m_hStmt, column, field, attrPtr, attrSize, length, numericPtr)); }


    // Member variables
    PODBC   & m_odbc;
    HSTMT     m_hStmt;
    SQLRETURN m_lastResult;

  private:
    Statement(const Statement & other) : PObject(other), m_odbc(other.m_odbc) { }
    void operator=(const Statement &) { }
};


// Must be simple struct, no virtuals!
struct PODBC::FieldExtra
{
  // Union must be first
  union {
    DATE_STRUCT      date;      /// Date Structure  
    TIME_STRUCT      time;      /// Time Structure
    TIMESTAMP_STRUCT timestamp; /// TimeStamp Structure
    char             datetime[SQL_TIMESTAMP_LEN];
  };

  SQLLEN bindLenOrInd;

  FieldExtra() { memset(this, 0, sizeof(*this)); }
};


struct PODBC::Link
{
  HENV m_hEnv; // Handle to environment
  HDBC m_hDBC; // Handle to database connection
};


static bool SQLFailed(PODBC & odbc, SQLSMALLINT type, SQLHANDLE handle, SQLRETURN result)
{
  if (result == SQL_SUCCESS)
    return false;

  if (result == SQL_NEED_DATA || result == SQL_NO_DATA)
    return true;

  bool error = result != SQL_SUCCESS_WITH_INFO;

  SQLINTEGER  nativeError;
  SQLSMALLINT msgLen, index = 1;
  char errStr[6], msg[SQL_MAX_MESSAGE_LENGTH];

  while (SQL_SUCCEEDED(SQLGetDiagRec(type,
                                     handle,
                                     index++,
                                     (unsigned char *)errStr,
                                     &nativeError,
                                     (unsigned char *)msg,
                                     sizeof(msg),
                                     &msgLen))) {
    PTRACE(error ? 2 : 4, "ODBC", errStr << ": " << msg);
    odbc.OnSQLError(nativeError, errStr, msg);
  }

  if (error && index == 2) {
    PTRACE(2, "ODBC\tSQL function failed but no error information available");
    odbc.OnSQLError(-1, PString::Empty(), "Unknown error");
  }

  return error;
}


#define new PNEW


#ifdef _MSC_VER
#pragma warning(disable:4244)
#endif


/////////////////////////////////////////////////////////////////
///PODBC

PODBC::PODBC()
  : m_link(new Link)
  , m_lastError(0)
  , m_precision(4)
  , m_timeFormat(PTime::MediumDate)
  , m_dateFormat(PTime::LongTime)
  , m_dateTimeFormat(PTime::MediumDateTime)
  , m_needChunking(false)
  , m_maxChunkSize(32768)
{
  m_link->m_hDBC = NULL;
  if (!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_link->m_hEnv)))
    m_lastErrorText = "Unable to allocated ODBC environment";
  else
    SQLFailed(*this, SQL_HANDLE_ENV, m_link->m_hEnv, SQLSetEnvAttr(m_link->m_hEnv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0));
}


PODBC::~PODBC()
{
  Disconnect();

  if (m_link->m_hEnv != NULL)
    SQLFreeHandle(SQL_HANDLE_ENV, m_link->m_hEnv);
  delete m_link;
}


PStringList PODBC::GetDrivers(bool withAttributes) const
{
  PStringList drivers;
  SQLCHAR description[1000], attributes[2000];
  SQLSMALLINT descLen, attrLen;

  if (m_link->m_hEnv != NULL &&
      SQL_SUCCEEDED(SQLDrivers(m_link->m_hEnv, SQL_FETCH_FIRST,
                               description, sizeof(description), &descLen,
                               attributes, sizeof(attributes), &attrLen))) {
    do {
      if (withAttributes) {
        for (PINDEX i = 0; i < (PINDEX)attrLen-1; ++i) {
          if (attributes[i] == '\0')
            attributes[i] = '\t';
        }
        drivers.AppendString(PString((const char *)description, descLen) + '\t' +
                              PString((const char *)attributes, attrLen));
      }
      else
        drivers.AppendString(PString((const char *)description, descLen));
    } while (SQL_SUCCEEDED(SQLDrivers(m_link->m_hEnv, SQL_FETCH_NEXT,
                                      description, sizeof(description), &descLen,
                                      attributes, sizeof(attributes), &attrLen)));
  }

  return drivers;
}


PStringList PODBC::GetSources(bool system, bool withDescription) const
{
  PStringList sources;
  SQLCHAR server[1000], description[1000];
  SQLSMALLINT servLen, descLen;

  if (m_link->m_hEnv != NULL &&
      SQL_SUCCEEDED(SQLDataSources(m_link->m_hEnv, system ? SQL_FETCH_FIRST_SYSTEM : SQL_FETCH_FIRST_USER,
                                   server, sizeof(server), &servLen,
                                   description, sizeof(description), &descLen))) {
    do {
      if (withDescription)
        sources.AppendString(PString((const char *)server, servLen) + '\t' +
                             PString((const char *)description, descLen));
      else
        sources.AppendString(PString((const char *)server, servLen));
    } while (SQL_SUCCEEDED(SQLDataSources(m_link->m_hEnv, SQL_FETCH_NEXT,
                                          server, sizeof(server), &servLen,
                                          description, sizeof(description), &descLen)));
  }

  return sources;
}


bool PODBC::Connect(const PString & source)
{
  if (m_link->m_hEnv == NULL)
    return false;

  Disconnect();

  if (SQLFailed(*this, SQL_HANDLE_ENV, m_link->m_hEnv, SQLAllocHandle(SQL_HANDLE_DBC, m_link->m_hEnv, &m_link->m_hDBC)))
    return false;

  short shortResult = 0;
  SQLCHAR szOutConnectString[1024];
  static SQLINTEGER LoginTimeout = 5;

  if (SQLFailed(*this, SQL_HANDLE_DBC, m_link->m_hDBC,
                SQLSetConnectAttr(m_link->m_hDBC, SQL_LOGIN_TIMEOUT, &LoginTimeout, 0)) ||
      SQLFailed(*this, SQL_HANDLE_DBC, m_link->m_hDBC,
                 SQLDriverConnect(m_link->m_hDBC,
                                  NULL,
                                  (SQLCHAR*)source.GetPointer(),
                                  source.GetLength(),
                                  szOutConnectString,
                                  sizeof(szOutConnectString),
                                  &shortResult,
                                  SQL_DRIVER_NOPROMPT
                                  ))) {
    PTRACE(2, "ODBC\tCould not connect to \"" << source << '"');
    // Don't call Disconnect() or m_lastError is changed.
    SQLFreeHandle(SQL_HANDLE_DBC, m_link->m_hDBC);
    m_link->m_hDBC = NULL;
    return false;
  }

  PTRACE(3, "ODBC\tConnected to " << szOutConnectString);
  OnConnected();
  return true;
}


const char * PODBC::GetDriverName(DriverType type)
{
  static const char * const Names[] = {
    "DSN",
    "MySQL ODBC 3.51 Driver",
    "PostgreSQL",
    "Microsoft ODBC for Oracle",
    "Microsoft ODBC for DB2",
    "SQL Server",
    "Microsoft Access Driver (*.mdb)",
    "Microsoft Paradox Driver (*.db )",
    "Microsoft Visual Foxpro Driver",
    "Microsoft dBASE Driver (*.dbf)",
    "Microsoft Excel Driver (*.xls)",
    "Microsoft Text Driver (*.txt; *.csv)"
  };
  return type < PARRAYSIZE(Names) ? Names[type] : "";
}


static PString AddDriver(PODBC::DriverType type)
{
  PStringStream strm;
  strm << "DRIVER={" << PODBC::GetDriverName(type) << "};";
  return strm;
}


static PString AddString(const char * title, const PString & value, const PString & dflt = PString::Empty())
{
  PStringStream strm;
  strm << title << '=';

  if (value.IsEmpty()) {
    if (dflt.IsEmpty())
      return PString::Empty();
    strm << dflt;
  }
  else
    strm << value;

  strm << ';';
  return strm;
}


static PString AddNumber(const char * title, unsigned value, unsigned dflt = 0)
{
  PStringStream strm;
  strm << title << '=';

  if (value == 0) {
    if (dflt == 0)
      return PString::Empty();
    strm << dflt;
  }
  else
    strm << value;

  strm << ';';
  return strm;
}


static PString AddBool(const char * title, bool value)
{
  PStringStream strm;
  strm << title << '=' << (value ? "Yes" : "No") << ';';
  return strm;
}


bool PODBC::Connect(const PString & source, const PString & username, const PString & password)
{
  return Connect(AddString("DSN", source) +
                 AddString("Uid", username) +
                 AddString("Pwd", password));
}


bool PODBC::IsConnected() const
{
  return m_link->m_hDBC != NULL;
}


void PODBC::Disconnect()
{
  if (m_link->m_hDBC != NULL) {
    SQLFailed(*this, SQL_HANDLE_DBC, m_link->m_hDBC, SQLDisconnect(m_link->m_hDBC));
    SQLFailed(*this, SQL_HANDLE_DBC, m_link->m_hDBC, SQLFreeHandle(SQL_HANDLE_DBC, m_link->m_hDBC));
    m_link->m_hDBC = NULL;
  }
}


bool PODBC::Connect_MSSQL(const PString & user,
                          const PString & pass,
                          const PString & host,
                          bool trusted,
                          MSSQLProtocols Proto)
{
  PString network;

  switch(Proto) {
  case MSSQLNamedPipes:
    network ="dbnmpntw";
    break;
  case MSSQLWinSock:
    network ="dbmssocn";
    break;
  case MSSQLIPX:
    network = "dbmsspxn";
    break;
  case MSSQLBanyan:
    network = "dbmsvinn";
    break;
  case MSSQLRPC:
    network = "dbmsrpcn";
    break;
  default:
    network = "dbmssocn";
    break;
  }

  return PODBC::Connect(AddDriver(MSSQL) +
                        AddString("Server", host) +
                        AddString("Uid", user) +
                        AddString("Pwd", pass) +
                        AddString("Network", network) +
                        AddBool("Trusted_Connection", trusted));
}


bool PODBC::Connect_DB2(const PFilePath & dbPath)
{
  return PODBC::Connect(AddDriver(IBM_DB2) +
                        AddNumber("DriverID", 277) +
                        AddString("Dbq", dbPath));
}


bool PODBC::Connect_XLS(const PFilePath & xlsPath, const PString & defDir)
{
  return PODBC::Connect(AddDriver(Excel) +
                        AddNumber("DriverId", 790) +
                        AddString("bq", xlsPath) +
                        AddString("DefaultDir", defDir));
}


bool PODBC::Connect_TXT(const PFilePath & txtPath)
{
  return PODBC::Connect(AddDriver(Ascii) +
                        AddString("Dbq", txtPath) +
                        AddString("Extensions", "asc,csv,tab,txt"));
}


bool PODBC::Connect_FOX(const PFilePath & dbPath,
                        const PString & user,
                        const PString & pass,
                        const PString & type,
                        bool exclusive)
{
  return PODBC::Connect(AddDriver(Foxpro) +
                        AddString("Uid", user) +
                        AddString("Pwd", pass) +
                        AddString("SourceDB", dbPath) +
                        AddString("SourceType", type) +
                        AddBool("Exclusive", exclusive));
}


bool PODBC::Connect_MDB(const PFilePath & mdbPath,
                        const PString & user,
                        const PString & pass,
                        bool exclusive)
{
  return PODBC::Connect(AddDriver(MSAccess) +
                        AddString("Dbq", mdbPath) +
                        AddString("Uid", user) +
                        AddString("Pwd", pass) +
                        AddBool("Exclusive", exclusive));
}


bool PODBC::Connect_PDOX(const PDirectory & dbPath, const PDirectory & defaultDir, int version)
{
  return PODBC::Connect(AddDriver(Paradox) +
                        AddNumber("DriverID", 538) +
                        "Fil=Paradox " + PString(version) + ".X;" +
                        AddString("DefaultDir", defaultDir) +
                        AddString("Dbq", dbPath) +
                        "CollatingSequence=ASCII;");
}


bool PODBC::Connect_DBASE(const PDirectory & dbPath)
{
  return PODBC::Connect(AddDriver(dBase) +
                        AddNumber("DriverID", 277) +
                        AddString("Dbq", dbPath));
}


bool PODBC::Connect_Oracle(const PString & server, const PString & user, const PString & pass)
{
  return PODBC::Connect(AddDriver(Oracle) +
                        AddString("Server", server) +
                        AddString("Uid", user) +
                        AddString("Pwd", pass));
}


static PConstString const LocalHost("localhost");

bool PODBC::Connect_mySQL(const PString & user,
                          const PString & pass,
                          const PString & host,
                          int port)
{
  return PODBC::Connect(AddDriver(mySQL) +
                        AddString("Server", host, LocalHost) +
                        AddNumber("Port", port, 3306) +
                        AddString("Uid", user) +
                        AddString("Pwd", pass));
}


bool PODBC::ConnectDB_mySQL(const PString & db,
                            const PString & user,
                            const PString & pass,
                            const PString & host,
                            int port)
{
  if (db.IsEmpty())
    return Connect_mySQL(user, pass, host, port);

  return PODBC::Connect(AddDriver(mySQL) +
                        AddString("Server", host, LocalHost) +
                        AddNumber("Port", port, 3306) +
                        AddString("Database", db) +
                        AddString("Uid", user) +
                        AddString("Pwd", pass));
}


bool PODBC::Connect_postgreSQL(const PString & db,
                               const PString & user,
                               const PString & pass,
                               const PString & host,
                               int port)
{
  return PODBC::Connect(AddDriver(postgreSQL) +
                        AddString("Server", host, LocalHost) +
                        AddNumber("Port", port, 5432) +
                        AddString("Database", db) +
                        AddString("Uid", user) +
                        AddString("Pwd", pass));
}


bool PODBC::DataSource(DriverType driver, ConnectData connectInfo)
{
  connectInfo.m_driver = driver;
  return Connect(connectInfo);
}


bool PODBC::Connect(const ConnectData & connectInfo)
{
  switch (connectInfo.m_driver) {
    case DSN :
      return Connect(connectInfo.m_database,connectInfo.m_username, connectInfo.m_password);
    case mySQL:
      return ConnectDB_mySQL(connectInfo.m_database,connectInfo.m_username,connectInfo.m_password,connectInfo.m_host,connectInfo.m_port);
    case MSSQL:
      return Connect_MSSQL(connectInfo.m_username,connectInfo.m_password,connectInfo.m_host,connectInfo.m_trusted, (MSSQLProtocols)connectInfo.m_options);
    case Oracle:
      return Connect_Oracle(connectInfo.m_username,connectInfo.m_username, connectInfo.m_password);
    case IBM_DB2:
      return Connect_DB2(connectInfo.m_database);
    case dBase:
      return Connect_DBASE(connectInfo.m_database);
    case Paradox:
      return Connect_PDOX(connectInfo.m_database,connectInfo.m_directory,connectInfo.m_options);
    case Excel:
      return Connect_XLS(connectInfo.m_database,connectInfo.m_directory);
    case Ascii:
      return Connect_TXT(connectInfo.m_database);
    case Foxpro:
      return Connect_FOX(connectInfo.m_database,connectInfo.m_username,connectInfo.m_password,"DBF",connectInfo.m_exclusive);
    case MSAccess:
      return Connect_MDB(connectInfo.m_database,connectInfo.m_username,connectInfo.m_password,connectInfo.m_exclusive);
    case postgreSQL:
      return Connect_postgreSQL(connectInfo.m_database,connectInfo.m_username,connectInfo.m_password,connectInfo.m_host,connectInfo.m_port);
    case ConnectionString :
      return Connect(connectInfo.m_database);
    default :
      return false;
  }
}


PStringArray PODBC::TableList(const PString & options)
{
  Statement data(*this);
  return data.TableList(options);
}


bool PODBC::Execute(const PString & query)
{
  if (m_link->m_hDBC == NULL)
    return false;

  Statement stmt(*this);
  return stmt.Execute(query);
}


void PODBC::SetPrecision(unsigned precision)
{
  m_precision = precision;
}


void PODBC::OnConnected()
{
  char f[2];
  SQLFailed(*this, SQL_HANDLE_DBC, m_link->m_hDBC,
            SQLGetInfo(m_link->m_hDBC, SQL_NEED_LONG_DATA_LEN, f, sizeof(f), NULL));
  m_needChunking = toupper(f[0]) == 'Y';
}


void PODBC::OnSQLError(int native, const PString & code, const PString & message)
{
  m_lastError = native;
  m_lastErrorText = code;
  if (!code.IsEmpty() && !message.IsEmpty())
    m_lastErrorText += ": ";
  m_lastErrorText += message;
}


PString PODBC::GetFieldType(DriverType driver, PVarType::BasicType type, unsigned size)
{
  switch (type) {
    case PVarType::VarBoolean :
      switch (driver) {
        case mySQL :
          return "tinyint";
        case postgreSQL :
          return "boolean";
        default :
          return "bit";
      }
    case PVarType::VarChar :
      return "char(1)";
    case PVarType::VarInt8 :
    case PVarType::VarUInt8 :
      switch (driver) {
        case MSAccess :
          return "byte";
        case postgreSQL :
          return "smallint";
        default :
          return "tinyint";
      }
    case PVarType::VarInt16 :
    case PVarType::VarUInt16 :
      switch (driver) {
        case MSAccess :
          return "integer";
        default :
          return "smallint";
      }
    case PVarType::VarInt32 :
    case PVarType::VarUInt32 :
      switch (driver) {
        case MSAccess :
          return "long";
        default :
          return "int";
      }
    case PVarType::VarInt64 :
    case PVarType::VarUInt64 :
      switch (driver) {
        case MSAccess :
          return "numeric";
        default :
          return "bigint";
      }
    case PVarType::VarFloatSingle :
      switch (driver) {
        case MSAccess :
          return "single";
        default :
          return "real";
      }
    case PVarType::VarFloatDouble :
      switch (driver) {
        case MSAccess :
          return "double";
        case postgreSQL :
          return "double precision";
        default :
          return "float(53)";
      }
    case PVarType::VarFloatExtended :
      return "numeric";
    case PVarType::VarGUID :
      switch (driver) {
        case MSAccess :
          return "guid";
        default :
          return "uniqueidentifier";
      }
    case PVarType::VarTime :
      switch (driver) {
        case postgreSQL :
          return "timestamp";
        default :
          return "datetime";
      }
    case PVarType::VarStaticBinary :
    case PVarType::VarDynamicBinary :
      switch (driver) {
        case postgreSQL :
          return "bytea";
        case MSAccess :
          if (size == 0 || size > 255)
            return "longbinary";
          // Do next case
        default :
          if (size == 0)
            return "varbinary";
          return psprintf("binary(%u)", size);
      }
    case PVarType::VarFixedString :
      if (size != 0)
        return psprintf("char(%u)", size);
      // Do next case
    default :
      switch (driver) {
        case MSAccess :
          if (size == 0 || size > 255)
            return "longtext";
          // do next case
        default :
          if (size == 0)
            return "text";
          return psprintf("varchar(%u)", size);
      }
  }
}


/////////////////////////////////////////////////////////////////////////////
// PODBC::Statement

PODBC::Statement::Statement(PODBC & odbc)
  : m_odbc(odbc)
  , m_lastResult(SQL_SUCCESS)
{
  if (SQLFailed(odbc, SQL_HANDLE_DBC, odbc.m_link->m_hDBC, SQLAllocHandle(SQL_HANDLE_STMT, odbc.m_link->m_hDBC, &m_hStmt))) {
    m_hStmt = SQL_NULL_HSTMT;
    return;
  }

  SQLSetStmtAttr(m_hStmt, SQL_ATTR_CONCURRENCY,    (SQLPOINTER) SQL_CONCUR_ROWVER, 0);
  SQLSetStmtAttr(m_hStmt, SQL_ATTR_CURSOR_TYPE,    (SQLPOINTER)SQL_CURSOR_KEYSET_DRIVEN, 0);
  SQLSetStmtAttr(m_hStmt, SQL_ATTR_ROW_BIND_TYPE,  (SQLPOINTER)SQL_BIND_BY_COLUMN, 0);
  SQLSetStmtAttr(m_hStmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)1, 0);
  SQLSetStmtAttr(m_hStmt, SQL_ATTR_ROW_STATUS_PTR, NULL, 0);
}


PODBC::Statement::~Statement()
{
  if (IsValid()) {
    CloseCursor();
    SQLFreeHandle(SQL_HANDLE_STMT, m_hStmt);
  }
}


DWORD PODBC::Statement::GetChangedRowCount(void)
{
  SQLLEN nRows = 0;
  return SQL_OK(SQLRowCount(m_hStmt,&nRows)) ? nRows : 0;
}


bool PODBC::Statement::Execute(const PString & sql)
{
  SQLCloseCursor(m_hStmt);
  return SQL_OK(SQLExecDirect(m_hStmt, (SQLCHAR *)sql.GetPointer(), sql.GetLength()));
}


bool PODBC::Statement::Commit(unsigned operation)
{
  SQLRETURN nRet = operation == SQL_ADD ? SQLBulkOperations(m_hStmt, SQL_ADD)
                                        : SQLSetPos(m_hStmt, 1, operation, SQL_LOCK_NO_CHANGE);

  /// Everything OK but no Long Data
  if (SQL_OK(nRet))
    return true;

  /// Error Somewhere else.
  if (nRet != SQL_NEED_DATA)
    return false;

  PINDEX chunkSize = m_odbc.GetMaxChunkSize();
  SQLPOINTER pColumn;

  /// If More Data Required
  while ((nRet = SQLParamData(m_hStmt, &pColumn)) == SQL_NEED_DATA) {
    Field & field = *(Field*)pColumn;

    const uint8_t * ptr = (const uint8_t *)field.GetPointer();
    PINDEX len = field.GetSize();

    while (len > chunkSize) {
      SQLPutData(m_hStmt, (SQLPOINTER)ptr, chunkSize);
      len -= chunkSize;
      ptr += chunkSize;
    }
    SQLPutData(m_hStmt, (SQLPOINTER)ptr, len);
  }

  return SQL_OK(nRet);
}


PStringArray PODBC::Statement::TableList(const PString & options)
{
  PStringArray list;

  SQLUSMALLINT column = 3;
  SQLRETURN result;

  if (options == "CATALOGS") {
    result = SQLTables(m_hStmt, (SQLCHAR *)SQL_ALL_CATALOGS, SQL_NTS, NULL, 0, NULL, 0, NULL, 0);
    column = 1;
  }
  else if (options *= "SCHEMAS") {
    result = SQLTables(m_hStmt, NULL, 0, (SQLCHAR *)SQL_ALL_SCHEMAS, SQL_NTS, NULL, 0, NULL, 0);
    column = 2;
  }
  else if (options *= "TYPES") {
    result = SQLTables(m_hStmt, NULL, 0, NULL, 0, NULL, 0, (SQLCHAR *)SQL_ALL_TABLE_TYPES, SQL_NTS);
    column = 4;
  }
  else if (options.Find('%') != P_MAX_INDEX)
    result = SQLTables(m_hStmt, NULL, 0, NULL, 0, (SQLCHAR *)options.GetPointer(), SQL_NTS, NULL, 0);
  else if (options.IsEmpty())
    result = SQLTables(m_hStmt, NULL, 0, NULL, 0, NULL, 0, (SQLCHAR *)"TABLE,VIEW", SQL_NTS);
  else 
    result = SQLTables(m_hStmt, NULL, 0, NULL, 0, NULL, 0, (SQLCHAR *)options.GetPointer(), SQL_NTS);

  if (SQL_OK(result)) {
    while (SQL_OK(SQLFetch(m_hStmt))) {
      char entry[1000];
      SQLLEN cb = SQL_NULL_DATA;
      if (SQL_OK(SQLGetData(m_hStmt, column, SQL_C_CHAR, entry, sizeof(entry), &cb)) && cb > 0)
        list.Append(new PCaselessString(entry));
    }
  }

  return list;
}


bool PODBC::Statement::SQL_OK(SQLRETURN result)
{
  m_lastResult = result;
  return !SQLFailed(m_odbc, SQL_HANDLE_STMT, m_hStmt, result);
}


/////////////////////////////////////////////////////////////////////////////
// PODBC::Field

PODBC::Field::Field(Row & row, PINDEX column)
  : m_row(row)
  , m_column(column)
  , m_odbcType(UINT_MAX)
  , m_scale(UINT_MAX)
  , m_isNullable(false)
  , m_isReadOnly(false)
  , m_isAutoIncrement(false)
  , m_decimals(UINT_MAX)
  , m_extra(new FieldExtra)
{
  Statement & statement = *m_row.m_recordSet.m_statement;

  SQLULEN suColSize = 0;
  SWORD swType = 0;
  {
    SWORD swNameLen = 0, swScale = 0, swNull = 0;
    SQLCHAR nameBuf[256];
    if (!statement.DescribeCol(m_column,        // ColumnNumber
                               nameBuf,         // ColumnName
                               sizeof(nameBuf), // BufferLength
                               &swNameLen,      // NameLengthPtr
                               &swType,         // DataTypePtr
                               &suColSize,      // ColumnSizePtr
                               &swScale,        // DecimalDigitsPtr
                               &swNull))        // NullablePtr
      return;

    m_name = PString((const char *)nameBuf, swNameLen);
    m_odbcType = swType;
    m_scale = swScale;
    m_isNullable = swNull == SQL_NULLABLE;

    SQLLEN attr = SQL_ATTR_READONLY;
    statement.ColAttribute(m_column, SQL_DESC_UPDATABLE, NULL, 0, NULL, &attr);
    m_isReadOnly = attr == SQL_ATTR_READONLY;

    attr = SQL_FALSE;
    statement.ColAttribute(m_column, SQL_DESC_AUTO_UNIQUE_VALUE, NULL, 0, NULL, &attr);
    m_isAutoIncrement = attr == SQL_TRUE;

    attr = 0;
    statement.ColAttribute(m_column, SQL_DESC_PRECISION, NULL, 0, NULL, &attr);
    m_decimals = attr;
  }

  switch (swType) {
    case SQL_BIT :
      PVarType::SetType(VarBoolean);
      m_odbcType = SQL_C_BIT;
      break;

    case SQL_TINYINT :
      PVarType::SetType(VarInt8);
      m_odbcType = SQL_C_STINYINT;
      break;

    case SQL_SMALLINT :
      PVarType::SetType(VarInt16);
      m_odbcType = SQL_C_SSHORT;
      break;

    case SQL_INTEGER :
      PVarType::SetType(VarInt32);
      m_odbcType = SQL_C_LONG;
      break;

    case SQL_BIGINT :
      PVarType::SetType(VarInt64);
      m_odbcType = SQL_C_SBIGINT;
      break;

    case SQL_NUMERIC :
    case SQL_DECIMAL :
    case SQL_FLOAT :
    case SQL_REAL :
    case SQL_DOUBLE :
      PVarType::SetType(VarFloatDouble);
      m_odbcType = SQL_C_DOUBLE;
      break;

    case SQL_GUID:
      PVarType::SetType(VarGUID);
      m_odbcType = SQL_C_GUID;
      break;

    case SQL_DATETIME:
    case SQL_TYPE_DATE:
    case SQL_TYPE_TIME:
    case SQL_TYPE_TIMESTAMP:
      switch (m_odbcType) {
        case SQL_DATETIME:
        case SQL_TYPE_TIMESTAMP:
          PVarType::SetType(VarTime, statement.m_odbc.GetDateTimeFormat());
          m_odbcType = SQL_C_TYPE_TIMESTAMP;
          break;

        case SQL_TYPE_DATE:
          PVarType::SetType(VarTime, statement.m_odbc.GetDateFormat());
          m_odbcType = SQL_C_TYPE_DATE;
          break;

        case SQL_TYPE_TIME:
          PVarType::SetType(VarTime, statement.m_odbc.GetTimeFormat());
          m_odbcType = SQL_C_TYPE_TIME;
          break;
      }
      if (!m_isReadOnly)
        statement.BindCol(m_column, m_odbcType, m_extra, sizeof(*m_extra), &m_extra->bindLenOrInd);
      return;

    case SQL_CHAR :
    case SQL_VARCHAR :
      PVarType::SetType(suColSize <= 1 ? VarChar : VarFixedString, suColSize+1);
      m_odbcType = SQL_C_CHAR;
      break;

    case SQL_BINARY :
    case SQL_VARBINARY :
      PVarType::SetType(VarDynamicBinary, suColSize);
      m_odbcType = SQL_C_BINARY;
      break;

    case SQL_LONGVARCHAR :
    case SQL_LONGVARBINARY :
      m_odbcType = m_odbcType == SQL_LONGVARCHAR ? SQL_C_CHAR : SQL_C_BINARY;

      if (m_isReadOnly || statement.m_odbc.GetMaxChunkSize() == P_MAX_INDEX) {
        PVarType::SetType(m_odbcType == SQL_C_CHAR ? VarDynamicString : VarDynamicBinary, 256);
        m_extra->bindLenOrInd = 0;
      }
      else {
        PVarType::SetType(m_odbcType == SQL_C_CHAR ? VarDynamicString : VarDynamicBinary, statement.m_odbc.GetMaxChunkSize());
        m_extra->bindLenOrInd = SQL_DATA_AT_EXEC;
      }

      if (!m_isReadOnly)
        statement.BindCol(m_column, m_odbcType, (SQLPOINTER)GetPointer(), GetSize(), &m_extra->bindLenOrInd);
      return;

    default :
      PTRACE(2, "ODBC\tUnknown/unsupported column data type " << swType << " for " << m_name);
      m_odbcType = swType;
      // Do next case, assume a char field
  }

  m_extra->bindLenOrInd = GetSize();

  if (!m_isReadOnly)
    statement.BindCol(m_column, m_odbcType, (SQLPOINTER)GetPointer(), m_extra->bindLenOrInd, &m_extra->bindLenOrInd);
}


PODBC::Field::~Field()
{
  if (!m_isReadOnly)
    m_row.m_recordSet.m_statement->BindCol(m_column, m_odbcType, NULL, 0, NULL);
  delete m_extra;
}


void PODBC::Field::InternalCopy(const PVarType & other)
{
  if (m_type == other.GetType()) {
    PVarType::InternalCopy(other);
    return;
  }

  switch (m_type) {
    case VarBoolean :
      m_.boolean = other.AsBoolean();
      break;
    case VarInt8 :
      m_.int8 = other.AsInteger();
      break;
    case VarInt16 :
      m_.int16 = other.AsInteger();
      break;
    case VarInt32 :
      m_.int32 = other.AsInteger();
      break;
    case VarInt64 :
      m_.int64 = other.AsInteger64();
      break;
    case VarUInt8 :
      m_.uint8 = other.AsUnsigned();
      break;
    case VarUInt16 :
      m_.uint16 = other.AsUnsigned();
      break;
    case VarUInt32 :
      m_.uint32 = other.AsUnsigned();
      break;
    case VarUInt64 :
      m_.uint64 = other.AsUnsigned64();
      break;
    case VarFloatSingle :
      m_.floatSingle = other.AsFloat();
      break;
    case VarFloatDouble :
      m_.floatDouble = other.AsFloat();
      break;
    case VarFloatExtended :
      m_.floatExtended = other.AsFloat();
      break;
    default :
      SetValue(other.AsString());
      return;
  }

  OnValueChanged();
}


bool PODBC::Field::SetType(BasicType, PINDEX)
{
  return false;
}


void PODBC::Field::SetDefaultValues()
{
  if (IsReadOnly() || IsAutoIncrement())
    return;

  if (IsNullable()) {
    SetValue(PString::Empty());
    SetNULL();
  }
  else
    SetValue("0");
}


void PODBC::Field::OnGetValue()
{
  Statement & statement = *m_row.m_recordSet.m_statement;

  if (m_isReadOnly && m_type == VarTime)
    statement.GetData(m_column, m_odbcType, m_extra, sizeof(*m_extra), &m_extra->bindLenOrInd);

  switch (m_odbcType) {
    case SQL_DATETIME :
      if (m_extra->bindLenOrInd == SQL_NULL_DATA)
        m_.time.seconds = -1;
      else
        m_.time.seconds = PTime(m_extra->datetime).GetTimeInSeconds();
      break;

    case SQL_C_TYPE_DATE:
      if (m_extra->bindLenOrInd == SQL_NULL_DATA)
        m_.time.seconds = -1;
      else
        m_.time.seconds = PTime(0, 0, 0, m_extra->date.day, m_extra->date.month, m_extra->date.year).GetTimeInSeconds();
      break;

    case SQL_C_TYPE_TIME:
      if (m_extra->bindLenOrInd == SQL_NULL_DATA)
        m_.time.seconds = -1;
      else
        m_.time.seconds = PTime(m_extra->time.second, m_extra->time.minute, m_extra->time.hour, 0, 0, 0).GetTimeInSeconds();
      break;

    case SQL_C_TYPE_TIMESTAMP:
      if (m_extra->bindLenOrInd == SQL_NULL_DATA)
        m_.time.seconds = -1;
      else
        m_.time.seconds = PTime(m_extra->timestamp.second, m_extra->timestamp.minute, m_extra->timestamp.hour,
                                m_extra->timestamp.day, m_extra->timestamp.month, m_extra->timestamp.year).GetTimeInSeconds();
      break;

    default :
      if (m_extra->bindLenOrInd != 0) {
        if (m_isReadOnly)
          statement.GetData(m_column, m_odbcType, (SQLPOINTER)GetPointer(), GetSize(), &m_extra->bindLenOrInd);
      }
      else {
        size_t nullTerminatedString = m_odbcType == SQL_C_CHAR ? 1 : 0;
        size_t total = 0;
        SQLINTEGER chunk = m_.dynamic.size;
        SQLLEN lenOrInd = 0;
        while (statement.GetData(m_column, m_odbcType, m_.dynamic.data+total, chunk, &lenOrInd)) {
          total += chunk - nullTerminatedString;

          if (lenOrInd == SQL_NO_TOTAL)
            m_.dynamic.Realloc(m_.dynamic.size+1000);
          else if (lenOrInd > chunk)
            m_.dynamic.Realloc(lenOrInd + nullTerminatedString);
          else
            break;

          chunk = m_.dynamic.size - total;
        }
      }
  }
}


void PODBC::Field::OnValueChanged()
{
  Statement & statement = *m_row.m_recordSet.m_statement;

  switch (m_type) {
    case VarFixedString :
    case VarStaticString :
    case VarDynamicString :
      m_extra->bindLenOrInd = strlen(m_.dynamic.data); // Allow for trailing '\0'

      // Bind again as pointer might have changed
      if (!m_isReadOnly)
        statement.BindCol(m_column, m_odbcType, (SQLPOINTER)GetPointer(), GetSize(), &m_extra->bindLenOrInd);
      break;

    case VarDynamicBinary :
      m_extra->bindLenOrInd = GetSize();

      // Bind again as pointer might have changed
      if (!m_isReadOnly)
        statement.BindCol(m_column, m_odbcType, (SQLPOINTER)GetPointer(), GetSize(), &m_extra->bindLenOrInd);
      break;

    case VarTime:
      if (m_.time.seconds <= 0)
        SetNULL();
      else {
        PTime time(m_.time.seconds);
        switch (m_odbcType) {
          case SQL_DATETIME :
            strcpy(m_extra->datetime, time.AsString("YYYY-MM-dd hh:mm:ss"));
            m_extra->bindLenOrInd = sizeof(m_extra->datetime);
            break;

          case SQL_C_TYPE_DATE:
            m_extra->date.day = time.GetDay();
            m_extra->date.month = time.GetMonth();
            m_extra->date.year = time.GetYear();
            m_extra->bindLenOrInd = sizeof(m_extra->date);
            break;

          case SQL_C_TYPE_TIME:
            m_extra->time.second = time.GetSecond();
            m_extra->time.minute = time.GetMinute();
            m_extra->time.hour = time.GetHour();
            m_extra->bindLenOrInd = sizeof(m_extra->time);
            break;

          case SQL_C_TYPE_TIMESTAMP:
            m_extra->timestamp.fraction = time.GetMicrosecond()*1000;
            m_extra->timestamp.second = time.GetSecond();
            m_extra->timestamp.minute = time.GetMinute();
            m_extra->timestamp.hour = time.GetHour();
            m_extra->timestamp.day = time.GetDay();
            m_extra->timestamp.month = time.GetMonth();
            m_extra->timestamp.year = time.GetYear();
            m_extra->bindLenOrInd = sizeof(m_extra->timestamp);
            break;
        }
      }
      break;

    default :
      m_extra->bindLenOrInd = GetSize();
  }
}


void PODBC::Field::SetNULL()
{
  m_extra->bindLenOrInd = SQL_NULL_DATA;
}


bool PODBC::Field::IsNULL() const
{
  return m_extra->bindLenOrInd == SQL_NULL_DATA;
}


bool PODBC::Field::Post()
{
  return m_row.m_recordSet.Commit();
}


///////////////////////////////////////////////////////////////////
// PODBC::Row

PODBC::Row::Row(RecordSet & table)
  : m_recordSet(table)
  , m_rowIndex(UndefinedRowIndex)
{
}


PODBC::Field & PODBC::Row::Column(PINDEX col) const
{
  /// Column = 0 return blank field
  return m_fields[PAssert(col > 0 && col <= m_fields.GetSize(), PInvalidParameter) ? col-1 : 0];
}


PODBC::Field & PODBC::Row::Column(const PString & name) const
{
  return Column(ColumnByName(name));
}


PStringArray PODBC::Row::ColumnNames() const
{
  PStringArray names;
  for (PINDEX i = 0; i < m_fields.GetSize(); ++i)
    names.AppendString(m_fields[i].GetName());
  return names;
}


PINDEX PODBC::Row::Rows()
{
  return m_recordSet.Rows();
}


void PODBC::Row::SetNewRow()
{
  m_rowIndex = UndefinedRowIndex;
  for (PINDEX i = 0; i < m_fields.GetSize(); i++)
    m_fields[i].SetDefaultValues();
}


bool PODBC::Row::MoveTo(RowIndex row)
{
  if (row == UndefinedRowIndex)
    return false;

  if (m_rowIndex == row)
    return true;

  if (!m_recordSet.m_statement->FetchScroll(SQL_FETCH_ABSOLUTE, row))
    return false;

  m_rowIndex = row;
  return true;
}


bool PODBC::Row::Move(int offset)
{
  switch (offset) {
    case 0 :
      return true;
    case 1 :
      return Next();
    case -1 :
      return Previous();
  }

  if (!m_recordSet.m_statement->FetchScroll(SQL_FETCH_RELATIVE, offset))
    return false;

  if (m_rowIndex != UndefinedRowIndex)
    m_rowIndex += offset;
  return true;
}


bool PODBC::Row::First()
{
  if (!m_recordSet.m_statement->FetchScroll(SQL_FETCH_FIRST))
    return false;

  m_rowIndex = 1;
  return true;
}


bool PODBC::Row::Next()
{
  if (!m_recordSet.m_statement->FetchScroll(SQL_FETCH_NEXT)) {
    if (m_recordSet.m_statement->m_lastResult == SQL_NO_DATA && m_recordSet.m_totalRows == UndefinedRowIndex)
      m_recordSet.m_totalRows = m_rowIndex;
    m_rowIndex = UndefinedRowIndex;
    return false;
  }

  if (m_rowIndex != UndefinedRowIndex)
    ++m_rowIndex;
  return true;
}


bool PODBC::Row::Previous()
{
  if (!m_recordSet.m_statement->FetchScroll(SQL_FETCH_PRIOR))
    return false;

  if (m_rowIndex != UndefinedRowIndex)
    --m_rowIndex;
  return true;
}


bool PODBC::Row::Last()
{
  if (!m_recordSet.m_statement->FetchScroll(SQL_FETCH_LAST))
    return false;

  m_rowIndex = m_recordSet.Rows(false);
  return true;
}


bool PODBC::Row::Commit()
{
  if (m_rowIndex != 0)
    return m_recordSet.m_statement->Commit(SQL_UPDATE);

  if (!m_recordSet.m_statement->Commit(SQL_ADD))
    return false;

  if (m_recordSet.m_totalRows != UndefinedRowIndex)
    ++m_recordSet.m_totalRows;

  return true;
}


bool PODBC::Row::Delete(RowIndex row)
{
  return m_recordSet.DeleteRow(row);
}


PINDEX PODBC::Row::ColumnByName(const PCaselessString & columnName) const
{
  PINDEX nCols = Columns();
  for(PINDEX i = 1; i <= nCols; ++i) {
    if (columnName == ColumnName(i))
      return i;
  }
  return 0;
}


/////////////////////////////////////////////////////////////////////////////
// PODBC::Table

PODBC::RecordSet::RecordSet(PODBC & odbc, const PString & query)
  : m_statement(new Statement(odbc))
  , m_totalRows(UndefinedRowIndex)
  , P_DISABLE_MSVC_WARNINGS(4355, m_cursor(*this))
{
  Query(query);
}


PODBC::RecordSet::RecordSet(PODBC * odbc, const PString & query)
  : m_statement(new Statement(*odbc))
  , m_totalRows(UndefinedRowIndex)
  , P_DISABLE_MSVC_WARNINGS(4355, m_cursor(*this))
{
  Query(query);
}


PODBC::RecordSet::RecordSet(const RecordSet & other)
  : PObject(other)
  , m_statement(NULL)
  , P_DISABLE_MSVC_WARNINGS(4355, m_cursor(*this))
{
}


PODBC::RecordSet::~RecordSet()
{
  m_cursor.m_fields.RemoveAll(); // Delete fields before m_statement
  delete m_statement;
}


bool PODBC::RecordSet::Query(const PString & query)
{
  m_statement->CloseCursor();
  m_cursor.m_rowIndex = 0;
  m_cursor.m_fields.RemoveAll();

  if (query.IsEmpty())
    return false;

  PCaselessString trimmed = query.Trim();
  if (trimmed.NumCompare("SELECT") != EqualTo &&
      m_statement->m_odbc.TableList().GetStringsIndex(trimmed) != P_MAX_INDEX)
    return Select(trimmed);

  if (!m_statement->Execute(query))
    return false;

  // See if succeeded
  SQLSMALLINT numColumns = 0;
  if(!m_statement->NumResultCols(&numColumns))
    return false;

  // Go to the First Row
  m_cursor.First();

  // Get initial values and structure
  for (SQLSMALLINT i = 1; i <= numColumns; i++)
    m_cursor.m_fields.Append(new Field(m_cursor, i));

  return true;
}


bool PODBC::RecordSet::Select(const PString & table,
                              const PString & whereClause,
                              const PString & fields,
                              const PString & orderedBy,
                              bool descending)
{
  PStringStream query;
  query << "SELECT ";

  if (fields.IsEmpty())
    query << '*';
  else
    query << fields;

  query << " FROM ";
  if (table.FindOneOf(" .") == P_MAX_INDEX)
    query << table;
  else
    query << '[' << table << ']';

  if (!whereClause.IsEmpty())
    query << " WHERE (" << whereClause << ')';

  if (!orderedBy.IsEmpty()) {
    query << " ORDERED BY ";
    if (orderedBy.FindOneOf(" .") == P_MAX_INDEX)
      query << orderedBy;
    else
      query << '[' << orderedBy << ']';
    if (descending)
      query << " DESC";
  }

  query << ';';
  return Query(query);
}


PODBC::RowIndex PODBC::RecordSet::Rows(bool forceCount)
{
  if (m_totalRows == UndefinedRowIndex && forceCount) {
    RowIndex save = m_cursor.m_rowIndex;
    if (m_cursor.First()) {
      do {
      } while (m_cursor.Next());
    }
    m_cursor.MoveTo(save);
  }
  return m_totalRows;
}


PODBC::Row & PODBC::RecordSet::NewRow()
{
  m_cursor.SetNewRow();
  return m_cursor;
}


bool PODBC::RecordSet::DeleteRow(RowIndex row)
{
  if (row != 0)
    m_cursor.Navigate(row);

  if (!m_statement->Commit(SQL_DELETE))
    return false;

  if (m_totalRows != UndefinedRowIndex)
    --m_totalRows;

  return true;
}


PODBC::Row & PODBC::RecordSet::operator[](PINDEX row)
{
  m_cursor.Navigate(row);
  return m_cursor;
}


PODBC::Field & PODBC::RecordSet::operator()(RowIndex row, PINDEX col)
{
  m_cursor.Navigate(row);
  return m_cursor.Column(col);
}


#endif // P_ODBC

