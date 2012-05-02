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
*
* $Revision$
* $Author$
* $Date$
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
    bool IsValid() const { return m_hStmt != SQL_NULL_HSTMT; }

    /** GetChangedRowCount retreives the number of rows updated/altered by
    UPDATE/INSERT statements.
    */
    DWORD GetChangedRowCount();

    /** Query function is the Main function to pass SQL statements to retreive/
    add/Modify database data. It accepts generally acceptable SQL Statements.
    ie. Select * from [table-x]
    */
    bool Query(const PString & sql);
    //@}

    /**@name Data Retrieval */
    //@{  
    /** FetchRow More detailed fetching of Rows. This allows you to fetch an
    Absolute row or a row relative to the current row fetched.
    */
    bool FetchScroll(SQLSMALLINT orientation, SQLLEN offset = 0)
    { return SQL_OK(SQLFetchScroll(m_hStmt, orientation, offset)); }

    /** Cancel the Current Statement
    */
    bool Cancel() { return SQL_OK(SQLCancel(m_hStmt)); }

    // Commit the data
    bool Commit(PODBC::Row & row, unsigned operation);

    /** Retreive the List of tables from the current Datasource
    The option field can be used to specify the Table Types
    ie "TABLE" for Tables or "VIEW" for preconfigured datasource
    queries. *Further investigation is required*
    */
    PStringArray TableList(const PString & option = PString::Empty());

    /** Is the SQL Instruction OK
    If an Error is detected then GetLastError is called
    to Retrieve the SQL Error Information and Returns false
    */
    bool SQL_OK(SQLRETURN res);
    //@}

    bool NumResultCols(SQLSMALLINT *columnCount) { return SQL_OK(SQLNumResultCols(m_hStmt, columnCount)); }

    bool GetData(
      SQLUSMALLINT column,
      SQLSMALLINT type,
      SQLPOINTER data,
      SQLLEN size,
      SQLLEN * len
    ) { return SQL_OK(SQLGetData(m_hStmt, column, type, data, size, len)); }

    bool BindCol(
      SQLUSMALLINT column,
      SQLSMALLINT type,
      SQLPOINTER data, 
      SQLLEN size,
      SQLLEN * len
      ) { return SQL_OK(SQLBindCol(m_hStmt, column, type, data, size, len)); }

    bool DescribeCol(
      SQLUSMALLINT column,
      SQLCHAR *namePtr,
      SQLSMALLINT nameSize,
      SQLSMALLINT *nameLength,
      SQLSMALLINT *dataType,
      SQLULEN *columnSize,
      SQLSMALLINT *decimalDigits,
      SQLSMALLINT *nullable
    ) { return SQL_OK(SQLDescribeCol(m_hStmt, column, namePtr, nameSize, nameLength, dataType, columnSize, decimalDigits, nullable)); }

    bool ColAttribute (
      SQLUSMALLINT column,
      SQLUSMALLINT field,
      SQLPOINTER attrPtr,
      SQLSMALLINT attrSize,
      SQLSMALLINT *length,
      SQLLEN * numericPtr
    ) { return SQL_OK(SQLColAttribute(m_hStmt, column, field, attrPtr, attrSize, length, numericPtr)); }


    PODBC   & m_odbc;   /// Reference to the PODBC Class
    HSTMT     m_hStmt;
    SQLRETURN m_lastError;

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


#define new PNEW


#ifdef _MSC_VER
#pragma warning(disable:4244)
#endif


/////////////////////////////////////////////////////////////////
///PODBC

PODBC::PODBC()
  : m_link(new Link)
  , m_lastError(SQL_ERROR)
  , m_precision(4)
  , m_timeFormat(PTime::MediumDate)
  , m_dateFormat(PTime::LongTime)
  , m_dateTimeFormat(PTime::MediumDateTime)
  , m_needChunking(false)
  , m_maxChunkSize(32768)
{
  m_link->m_hDBC = NULL;
  m_lastError = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_link->m_hEnv);
  if (SQL_SUCCEEDED(m_lastError))
    m_lastError = SQLSetEnvAttr(m_link->m_hEnv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
}


PODBC::~PODBC()
{
  Disconnect();

  if (m_link->m_hEnv != NULL)
    m_lastError = SQLFreeHandle(SQL_HANDLE_ENV, m_link->m_hEnv);
}


PINDEX PODBC::GetSources(PStringList & servers, PStringList & descriptions, bool system)
{
  if (m_link->m_hEnv == NULL)
    return 0;

  PString server, description;
  SQLSMALLINT servLen, descLen;
  SQLUSMALLINT fetch = system ? SQL_FETCH_FIRST_SYSTEM : SQL_FETCH_FIRST_USER;
  PINDEX count = 0;

  if (SQL_SUCCEEDED(m_lastError = SQLDataSources(m_link->m_hEnv, fetch, NULL, 0, &servLen, NULL, 0, &descLen))) {
    do {
      m_lastError = SQLDataSources(m_link->m_hEnv, fetch,
                                   (SQLCHAR*)server.GetPointerAndSetLength(servLen), servLen, &servLen,
                                   (SQLCHAR*)description.GetPointerAndSetLength(descLen), descLen, &descLen);
      servers.AppendString(server);
      descriptions.AppendString(description);
      fetch = SQL_FETCH_NEXT;
      ++count;
    } while (SQL_SUCCEEDED(m_lastError = SQLDataSources(m_link->m_hEnv, fetch, NULL, 0, &servLen, NULL, 0, &descLen)));
  }

  return count;
}


bool PODBC::InternalConnectSetUp()
{
  if (m_link->m_hEnv == NULL)
    return false;

  Disconnect();

  if (!SQL_SUCCEEDED(m_lastError = SQLAllocHandle(SQL_HANDLE_DBC, m_link->m_hEnv, &m_link->m_hDBC)))
    return false;

  SQLSetConnectOption(m_link->m_hDBC, SQL_LOGIN_TIMEOUT, 5);
  return true;
}


bool PODBC::Connect(const PString & source)
{
  short shortResult = 0;
  SQLCHAR szOutConnectString[1024];

  if (InternalConnectSetUp() &&
      SQL_SUCCEEDED(m_lastError = SQLDriverConnect(m_link->m_hDBC,                 // Connection Handle
                                                   NULL,                           // Window Handle
                                                   (SQLCHAR*)source.GetPointer(),  // InConnectionString
                                                   source.GetLength(),
                                                   szOutConnectString,             // OutConnectionString
                                                   sizeof(szOutConnectString),     // Buffer length
                                                   &shortResult,                   // StringLength2Ptr
                                                   SQL_DRIVER_NOPROMPT             // no User prompt
                                                   ))) {
    PTRACE(4, "ODBC\tConnected to " << source);
    OnConnected();
    return true;
  }

  Disconnect();
  return false;
}


bool PODBC::Connect(const PString & source, const PString & username, const PString & password)
{
  if (InternalConnectSetUp() &&
      SQL_SUCCEEDED(m_lastError = SQLConnect(m_link->m_hDBC,
                                             (SQLCHAR *)source.GetPointer(),   source.GetLength(),
                                             (SQLCHAR *)username.GetPointer(), username.GetLength(),
                                             (SQLCHAR *)password.GetPointer(), password.GetLength()))) {
    PTRACE(4, "ODBC\tConnected to " << source);
    OnConnected();
    return true;
  }

  Disconnect();
  return false;
}


bool PODBC::IsConnected() const
{
  return m_link->m_hDBC != NULL;
}


void PODBC::Disconnect()
{
  if (m_link->m_hDBC != NULL) {
    m_lastError = SQLDisconnect(m_link->m_hDBC);
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

  return PODBC::Connect("Driver={SQL Server};"
                        "Server=" + host + ";"
                        "Uid=" + user + ";"
                        "Pwd=" + pass + ";"
                        "Trusted_Connection=" + (trusted ? "Yes" : "No") + ";"
                        "Network=" + network + ";");
}


bool PODBC::Connect_DB2(const PFilePath & dbPath)
{
  return PODBC::Connect("Driver={Microsoft dBASE Driver (*.dbf)};"
                        "DriverID=277;"
                        "Dbq=" + dbPath + ";");
}


bool PODBC::Connect_XLS(const PFilePath & xlsPath, const PString & defDir)
{
  return PODBC::Connect("Driver={Microsoft Excel Driver (*.xls)};"
                        "DriverId=790;"
                        "bq=" + xlsPath + ";"
                        "DefaultDir=" + defDir + ";");
}


bool PODBC::Connect_TXT(const PFilePath & txtPath)
{
  return PODBC::Connect("Driver={Microsoft Text Driver (*.txt; *.csv)};"
                        "Dbq=" + txtPath + ";"
                        "Extensions=asc,csv,tab,txt;");
}


bool PODBC::Connect_FOX(const PFilePath & dbPath,
                        const PString & user,
                        const PString & pass,
                        const PString & type,
                        bool exclusive)
{
  return PODBC::Connect("Driver={Microsoft Visual Foxpro Driver};"
                        "Uid=" + user + ";"
                        "Pwd=" + pass + ";"
                        "SourceDB=" + dbPath + ";"
                        "SourceType=" + type + ";"
                        "Exclusive=" + (exclusive ? "yes": "no") + ";");
}


bool PODBC::Connect_MDB(const PFilePath & mdbPath,
                        const PString & user,
                        const PString & pass,
                        bool exclusive)
{
  return PODBC::Connect("Driver={Microsoft Access Driver (*.mdb)};"
                        "Dbq=" + mdbPath + ";"
                        "Uid=" + user + ";"
                        "Pwd=" + pass + ";"
                        "Exclusive=" + (exclusive ? "yes" : "no"));
}


bool PODBC::Connect_PDOX(const PDirectory & dbPath, const PDirectory & defaultDir, int version)
{
  PString driver = "3.X";
  if (version == 4)
    driver = "4.X";
  if (version > 4)
    driver = "5.X";

  return PODBC::Connect("Driver={Microsoft Paradox Driver (*.db )};"
                        "DriverID=538;"
                        "Fil=Paradox " + driver + ";"
                        "DefaultDir=" + defaultDir + ";"
                        "Dbq=" + dbPath + ";"
                        "CollatingSequence=ASCII;");
}


bool PODBC::Connect_DBASE(const PDirectory & dbPath)
{
  return PODBC::Connect("Driver={Microsoft dBASE Driver (*.dbf)};"
                        "DriverID=277;Dbq=" + dbPath + ";");
}


bool PODBC::Connect_Oracle(const PString & server, const PString & user, const PString & pass)
{
  return PODBC::Connect("Driver={Microsoft ODBC for Oracle};"
                        "Server=" + server + ";"
                        "Uid=" + user + ";"
                        "Pwd=" + pass + ";");
}


bool PODBC::Connect_mySQL(const PString & user,
                          const PString & pass,
                          const PString & host,
                          int port)
{
  return PODBC::Connect("Driver={MySQL ODBC 3.51 Driver};"
                        "Uid=" + user + ";"
                        "Pwd=" + pass + ";"
                        "Server=" + (host.IsEmpty() ? "localhost" : host) + ";"
                        "Port=" + PString(port == 0 ? 3306 : port) + ";");
}


bool PODBC::ConnectDB_mySQL(const PString & db,
                            const PString & user,
                            const PString & pass,
                            const PString & host,
                            int port)
{
  if (db.IsEmpty())
    return Connect_mySQL(user, pass, host, port);

  return PODBC::Connect("Driver={MySQL ODBC 3.51 Driver};"
                        "Database=" + db + ";"
                        "Uid=" + user + ";"
                        "Pwd=" + pass +";"
                        "Server=" + (host.IsEmpty() ? "localhost" : host) + ";"
                        "Port=" + PString(port == 0 ? 3306 : port) + ";");
}


bool PODBC::Connect_postgreSQL(const PString & db,
                               const PString & user,
                               const PString & pass,
                               const PString & host,
                               int port)
{
  return PODBC::Connect("Driver={PostgreSQL};"
                        "Database=" + db + ";"
                        "Uid=" + user + ";"
                        "Pwd=" + pass +";"
                        "Server=" + (host.IsEmpty() ? "localhost" : host) + ";"
                        "Port=" + PString(port == 0 ? 5432 : port) + ";");
}


bool PODBC::DataSource(DataSources Source, ConnectData connectInfo)
{
  connectInfo.Source = Source;
  return Connect(connectInfo);
}


bool PODBC::Connect(const ConnectData & connectInfo)
{
  m_dbase = connectInfo.Source;

  switch (m_dbase)
  {
  case PODBC::mySQL:
    return ConnectDB_mySQL(connectInfo.DefDir,connectInfo.User,connectInfo.Pass,connectInfo.Host,connectInfo.Port);
  case PODBC::MSSQL:
    return Connect_MSSQL(connectInfo.User,connectInfo.Pass,connectInfo.Host,connectInfo.Excl_Trust, (MSSQLProtocols)connectInfo.Option);
  case PODBC::Oracle:
    return Connect_Oracle(connectInfo.Host,connectInfo.User, connectInfo.Pass);
  case PODBC::IBM_DB2:
    return Connect_DB2(connectInfo.DBPath);
  case PODBC::DBASE:
    return Connect_DBASE(connectInfo.DBPath);
  case PODBC::Paradox:
    return Connect_PDOX(connectInfo.DBPath,connectInfo.DefDir,connectInfo.Option);
  case PODBC::Excel:
    return Connect_XLS(connectInfo.DBPath,connectInfo.DefDir);
  case PODBC::Ascii:
    return Connect_TXT(connectInfo.DBPath);
  case PODBC::Foxpro:
    return Connect_FOX(connectInfo.DBPath,connectInfo.User,connectInfo.Pass,"DBF",connectInfo.Excl_Trust);
  case PODBC::MSAccess:
    return Connect_MDB(connectInfo.DBPath,connectInfo.User,connectInfo.Pass,connectInfo.Excl_Trust);
  case PODBC::postgreSQL:
    return Connect_postgreSQL(connectInfo.User,connectInfo.Pass,connectInfo.Host,connectInfo.Port,connectInfo.Option);
  };

  return false;
}


PStringArray PODBC::TableList(const PString & option)
{
  Statement data(*this);
  return data.TableList(option);
}


bool PODBC::Query(const PString & query)
{
  if (m_link->m_hDBC == NULL)
    return false;

  Statement stmt(*this);
  return stmt.Query(query);
}


void PODBC::SetPrecision(unsigned precision)
{
  m_precision = precision;
}


void PODBC::OnConnected()
{
  char f[2];
  m_lastError = SQLGetInfo(m_link->m_hDBC, SQL_NEED_LONG_DATA_LEN, f, sizeof(f), NULL);
  m_needChunking = toupper(f[0]) == 'Y';
}


/////////////////////////////////////////////////////////////////////////////
// PODBC::Statement

PODBC::Statement::Statement(PODBC & odbc)
  : m_odbc(odbc)
  , m_lastError(SQL_SUCCESS)
{
  SQLRETURN m_nReturn = SQLAllocHandle(SQL_HANDLE_STMT, odbc.m_link->m_hDBC, &m_hStmt);
  SQLSetStmtAttr(m_hStmt, SQL_ATTR_CONCURRENCY,    (SQLPOINTER) SQL_CONCUR_ROWVER, 0);
  SQLSetStmtAttr(m_hStmt, SQL_ATTR_CURSOR_TYPE,    (SQLPOINTER)SQL_CURSOR_KEYSET_DRIVEN, 0);
  SQLSetStmtAttr(m_hStmt, SQL_ATTR_ROW_BIND_TYPE,  (SQLPOINTER)SQL_BIND_BY_COLUMN, 0);
  SQLSetStmtAttr(m_hStmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)1, 0);
  SQLSetStmtAttr(m_hStmt, SQL_ATTR_ROW_STATUS_PTR, NULL, 0);

  if(!SQL_OK(m_nReturn))
    m_hStmt = SQL_NULL_HSTMT;
}


PODBC::Statement::~Statement()
{
  SQLCloseCursor(m_hStmt);

  if (IsValid())
    SQLFreeHandle(SQL_HANDLE_STMT, m_hStmt);
}


DWORD PODBC::Statement::GetChangedRowCount(void)
{
  SQLLEN nRows = 0;
  return SQL_OK(SQLRowCount(m_hStmt,&nRows)) ? nRows : 0;
}


bool PODBC::Statement::Query(const PString & query)
{
  return SQL_OK(SQLExecDirect(m_hStmt, (SQLCHAR *)query.GetPointer(), query.GetLength()));
}


bool PODBC::Statement::Commit(PODBC::Row & row, unsigned operation)
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
    Field & field = row[*(PINDEX*)pColumn];

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


PStringArray PODBC::Statement::TableList(const PString & option)
{
  PStringArray list;
  SQLLEN cb = 0;

  /// This Statement will need reviewing as it
  /// depends on the Database, Might work on some
  /// but not on others

  if (SQL_OK(SQLTables(m_hStmt, 0, SQL_NTS, 0, SQL_NTS, 0, SQL_NTS, (SQLCHAR *)option.GetPointer(), option.GetLength()))) {
    char entry[130];
    while (SQL_OK(SQLFetch(m_hStmt))) {
      SQLGetData(m_hStmt, 3, SQL_C_CHAR, entry, sizeof(entry), &cb);
      if (entry[0] != '\0')
        list += entry;
    }
  }

  return list;
}


bool PODBC::Statement::SQL_OK(SQLRETURN res)
{
  if (SQL_SUCCEEDED(res))
    return true;

  m_lastError = res;
  if (res == SQL_NEED_DATA)
    return false;

  SQLINTEGER  nativeError;
  SQLSMALLINT msgLen, i = 1;
  char errStr[6], msg[SQL_MAX_MESSAGE_LENGTH];

  while (SQLGetDiagRec(SQL_HANDLE_STMT,
                       m_hStmt,
                       i++,
                       (unsigned char *)errStr,
                       &nativeError,
                       (unsigned char *)msg,
                       sizeof(msg),
                       &msgLen) != SQL_NO_DATA) {
    PTRACE(2, "ODBC\tError " << errStr << " - " << msg);
    m_odbc.OnSQLError(errStr, msg);
  }

  return false;
}


/////////////////////////////////////////////////////////////////////////////
// PODBC::Field

PODBC::Field::Field(Row & row, PINDEX column)
  : m_row(row)
  , m_column(column)
  , m_odbcType(UINT_MAX)
  , m_size(P_MAX_INDEX)
  , m_scale(UINT_MAX)
  , m_isNullable(false)
  , m_isReadOnly(false)
  , m_isAutoIncrement(false)
  , m_decimals(UINT_MAX)
  , m_extra(new FieldExtra)
{
  Statement & statement = *m_row.m_recordSet.m_statement;

  SWORD swCol = 0, swType = 0, swScale = 0, swNull = 0;
  SQLULEN pcbColDef = 0;
  TCHAR name[256] = _T("");
  if (!statement.DescribeCol(m_column,        // ColumnNumber
                             (SQLCHAR*)name, // ColumnName
                             sizeof(name),    // BufferLength
                             &swCol,          // NameLengthPtr
                             &swType,         // DataTypePtr
                             &pcbColDef,      // ColumnSizePtr
                             &swScale,        // DecimalDigitsPtr
                             &swNull))        // NullablePtr
    return;

  m_name = name;
  m_odbcType = swType;
  m_size = pcbColDef;
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

  bool useExtra = false;

  switch (swType) {
    case SQL_C_BIT :
      PVarType::SetType(VarBoolean);
      break;

    case SQL_C_TINYINT :
      PVarType::SetType(VarInt8);
      break;

    case SQL_C_UTINYINT :
      PVarType::SetType(VarUInt8);
      break;

    case SQL_C_SSHORT :
      PVarType::SetType(VarInt16);
      break;

    case SQL_C_USHORT :
      PVarType::SetType(VarUInt16);
      break;

    case SQL_C_LONG :
      PVarType::SetType(VarInt32);
      break;

    case SQL_C_ULONG :
      PVarType::SetType(VarUInt32);
      break;

    case SQL_C_SBIGINT :
      PVarType::SetType(VarInt64);
      break;

    case SQL_C_UBIGINT :
      PVarType::SetType(VarUInt64);
      break;

    case SQL_NUMERIC :
    case SQL_DECIMAL :
    case SQL_FLOAT :
    case SQL_REAL :
    case SQL_DOUBLE :
      PVarType::SetType(VarFloatDouble);
      m_odbcType = SQL_DOUBLE;
      break;

    case SQL_C_GUID:
      PVarType::SetType(VarGUID);
      break;

    case SQL_DATETIME:
    case SQL_C_TYPE_TIMESTAMP:
      PVarType::SetType(VarTime, statement.m_odbc.GetDateTimeFormat());
      useExtra = true;
      break;

    case SQL_C_TYPE_DATE:
      PVarType::SetType(VarTime, statement.m_odbc.GetDateFormat());
      useExtra = true;
      break;

    case SQL_C_TYPE_TIME:
      PVarType::SetType(VarTime, statement.m_odbc.GetTimeFormat());
      useExtra = true;
      break;

    default :
      PTRACE(2, "ODBC\tUnknown/unsupported column data type " << swType);
      // Do next case, assume a char field

    case SQL_VARCHAR :
    case SQL_C_CHAR :
      PVarType::SetType(m_size <= 1 ? VarChar : VarFixedString, m_size+1);
      m_odbcType = SQL_C_CHAR;
  }

  if (!m_isReadOnly)
    statement.BindCol(m_column,
                      m_odbcType,
                      useExtra ? m_extra : (SQLPOINTER)GetPointer(),
                      useExtra ? sizeof(*m_extra) : GetSize(),
                      &m_extra->bindLenOrInd);
}


PODBC::Field & PODBC::Field::operator=(const PVarType & other)
{
  if (m_type == other.GetType())
    InternalCopy(other);
  else
    SetValue(other.AsString());
  return *this;
}


PODBC::Field::~Field()
{
  delete m_extra;
}


bool PODBC::Field::SetType(BasicType, PINDEX)
{
  return false;
}


void PODBC::Field::SetDefaultValues()
{
  if (IsReadOnly() || IsAutoIncrement())
    return;

  if (IsNullable())
    SetValue(PString::Empty());
  else
    SetValue("0");
}


void PODBC::Field::OnGetValue()
{
  Statement & statement = *m_row.m_recordSet.m_statement;

  if (m_type == VarTime) {
    if (m_isReadOnly)
      statement.GetData(m_column, m_odbcType, m_extra, sizeof(*m_extra), &m_extra->bindLenOrInd);

    switch (m_odbcType) {
      case SQL_DATETIME :
        m_.time.seconds = PTime(m_extra->datetime).GetTimeInSeconds();
        break;
      case SQL_C_TYPE_DATE:
        m_.time.seconds = PTime(0, 0, 0, m_extra->date.day, m_extra->date.month, m_extra->date.year).GetTimeInSeconds();
        break;
      case SQL_C_TYPE_TIME:
        m_.time.seconds = PTime(m_extra->time.second, m_extra->time.minute, m_extra->time.hour, 0, 0, 0).GetTimeInSeconds();
        break;
      case SQL_C_TYPE_TIMESTAMP:
        m_.time.seconds = PTime(m_extra->timestamp.second, m_extra->timestamp.minute, m_extra->timestamp.hour,
                                m_extra->timestamp.day, m_extra->date.month, m_extra->date.year).GetTimeInSeconds();
        break;
    }
  }
  else {
    if (m_isReadOnly)
      statement.GetData(m_column, m_odbcType, (SQLPOINTER)GetPointer(), GetSize(), &m_extra->bindLenOrInd);
  }
}


void PODBC::Field::OnValueChanged()
{
  switch (m_type) {
    case VarFixedString :
      m_extra->bindLenOrInd = m_.dynamic.size;
      break;

    default :
      break;
  }
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
    if (m_recordSet.m_statement->m_lastError == SQL_NO_DATA && m_recordSet.m_totalRows == UndefinedRowIndex)
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
    return m_recordSet.m_statement->Commit(*this, SQL_UPDATE);

  if (!m_recordSet.m_statement->Commit(*this, SQL_ADD))
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

#ifdef _MSC_VER
#pragma warning(disable:4355)
#endif

PODBC::RecordSet::RecordSet(PODBC & odbc, const PString & query)
  : m_statement(new Statement(odbc))
  , m_totalRows(UndefinedRowIndex)
  , m_cursor(*this)
{
  Construct(query);
}


PODBC::RecordSet::RecordSet(PODBC * odbc, const PString & query)
  : m_statement(new Statement(*odbc))
  , m_totalRows(UndefinedRowIndex)
  , m_cursor(*this)
{
  Construct(query);
}


PODBC::RecordSet::RecordSet(const RecordSet & other)
  : PObject(other)
  , m_statement(NULL)
  , m_cursor(*this)
{
}

#ifdef _MSC_VER
#pragma warning(default:4355)
#endif


PODBC::RecordSet::~RecordSet()
{
  delete m_statement;
}


void PODBC::RecordSet::Construct(const PString & query)
{
  PCaselessString select = query.Trim();
  if (select.NumCompare("SELECT") != EqualTo)    // Select Query
    select = "SELECT * FROM [" + select + "];";

  if (!m_statement->Query(select))
    return;

  // See if succeeded
  SQLSMALLINT numColumns = 0;
  if(!m_statement->NumResultCols(&numColumns))
    return;

  // Go to the First Row
  m_cursor.First();

  // Get initial values and structure
  for (SQLSMALLINT i = 1; i <= numColumns; i++)
    m_cursor.m_fields.Append(new Field(m_cursor, i));
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

  if (!m_statement->Commit(m_cursor, SQL_DELETE))
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

