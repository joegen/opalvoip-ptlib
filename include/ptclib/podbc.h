/*
 * podbc.h
 *
 * Virteos ODBC Implementation for PWLib Library.
 *
 * Virteos is a Trade Mark of ISVO (Asia) Pte Ltd.
 *
 * Copyright (c) 2005 ISVO (Asia) Pte Ltd. All Rights Reserved.
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

/**
  ODBC Support for PWLIB

  Class Description
   PODBC              :  Main DataBase connection class (Can derive class for error handling)
   PODBC::ConnectData :  Class used to store information for Connecting to DataSource
   PODBC::RecordSet   :  Retrieved Data from Table or Select SQL Query
   PODBC::Row         :  Record wrapper class for the PODBC::RecordSet (PArray of Fields)
   PODBC::Field       :  Database field information (Field structure & bound data)
   PODBC::Statement   :  Wrapper for ODBC "statement" (Internal)

  Example of Use

<pre><code>
  PODBC link;
  PODBC::ConnectData connectInfo;
  connectInfo.Source = PODBC::MSAccess;
  connectInfo.DBPath = "test.mdb";
  connectInfo ....

  if (link.Connect(connectInfo)) {
    // Load a Database Table (could also be a SELECT Query)
    PODBC::RecordSet table(link, "FooTable");

    // Bind to Column 1
    PODBC::Field & field = table.Column(1):

    // Display Contents
    cout << " Value " << field.AsString(); << endl;

    // Move to Record 2 of fooTable
    table[2];

    // Display contents of Record 2 Column 1
    cout << " Value " << field.AsString(); << endl;

    // Set New Value for Record 2 Field 1 of FooTable
    field.SetValue("NewValue");

    // Send Update to Database.
    field.Commit();

    // To Add New Record.(with Default Values)
    table.NewRow();

    // Alter the Value of field 1
    field.SetValue("Something");

    // Post the New Field to the Database
    field.Commit();

    // Run General Query;
    link.Query("INSERT foo into [FooTable] ...");
  }
  // Disconnect from ODBC Source
  link.Disconnect();
</code></pre>
*/
//--

#ifndef PTLIB_PODBC_H
#define PTLIB_PODBC_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#if P_ODBC

#include <ptclib/vartype.h>


/** PODBC Class
  The Main ODBC class. This Class should be used in the there is
  not a preconfigured DSN setup in the MDAC. This class will use
  the applicable ODBC drivers to connect to a compliant Datasource.
  It Supports a wide variety of Datasources but others can added
  by simply creating your custom connection string and then calling
  PODBC::Connect. For Defined sources the PODBC::DataSource function
  should be used.
*/

class PODBC  : public PObject
{
    PCLASSINFO(PODBC, PObject);
  public:
    /**@name Constructor/Destructor */
    //@{
    /** Constructs ODBC connection
    */
    PODBC();

    /** Destroys ODBC connection
    */
    ~PODBC();
    //@}

    /// Type for row index, may become 64 bit one day
    typedef unsigned RowIndex;
    enum { UndefinedRowIndex = 0 };

    class Field;
    class Row;
    class RecordSet;
    class Statement;  // Internal use
    struct FieldExtra;  // Internal use


    /** Class for Field Data
    */
    class Field : public PVarType
    {
        PCLASSINFO(Field, PObject);
      public:
        ~Field();

        Field & operator=(const Field & other) { PVarType::operator=(other); return *this; }
        Field & operator=(const PVarType & other) { PVarType::operator=(other); return *this; }

        // Overrides from PVarType, make sure can't change type
        virtual bool SetType(BasicType type, PINDEX options = 0);

        /** Initialise/Set the Default values for Field of New Record
        */
        void SetDefaultValues();

        /**Set value to NULL
          */
        void SetNULL();

        /// Is curreently NULL value
        bool IsNULL() const;

        /// Get column index number, 1 up
        PINDEX GetColumn() const { return m_column; }

        /// Get column name
        const PString GetName() const { return m_name; }

        int GetDataType() const { return m_odbcType; }
        unsigned GetScale() const { return m_scale; }
        bool IsNullable() const { return m_isNullable; }
        bool IsReadOnly() const { return m_isReadOnly; }
        bool IsUpdatable() const { return !IsReadOnly(); }
        bool IsAutoIncrement() const { return m_isAutoIncrement; }
        bool IsAutoIndex() { return IsAutoIncrement(); }
        unsigned GetPrecision() const { return m_decimals; }

        // For backward compatibility
        bool Post();

      protected:
        Field(Row & row, PINDEX column);

        // Call backs from PVarType
        virtual void OnGetValue();
        virtual void OnValueChanged();
        virtual void InternalCopy(const PVarType & other);

        Row   & m_row;       /// Back Reference to the Row
        PINDEX  m_column;    /// Column number

        PString    m_name;       /// Column Name
        int        m_odbcType;
        unsigned   m_scale;
        bool       m_isNullable;
        bool       m_isReadOnly;
        bool       m_isAutoIncrement;
        unsigned   m_decimals;     /// Number of decimal places to Round

        FieldExtra * m_extra; // Some types are not compatible with PVarType

      private:
        Field(const Field & other) : PVarType(), m_row(other.m_row) { }

      friend class Row;
      friend class RecordSet;
    };


    /**Database Row Class
    This class functions as a simple wrapper
    of the Statement class to fetch/Save
    data to the Datasource. Data is fetched
    on a need to basis and not cached except
    to create a new row.
    */
    class Row : public PObject
    {
        PCLASSINFO(Row, PObject);
      public:
        /** Constructor. If rowIndex == 0, an empty record is used, otherwise
            that record number is fethed from back end.
        */
        Row(RecordSet & recordSet);

        /** Columns. The Number of Columns in the RecordSet
        */
        PINDEX Columns() const { return m_fields.GetSize(); }

        /** Retrieve the Column Names
        */
        PStringArray ColumnNames() const;

        /** ColumnByName returns the column number of the column name
        If not found returns column value of 0;
        */
        PINDEX ColumnByName(const PCaselessString & columnName) const;

        /** Retrieve Field Data given the specifed column.
        Note: Columns atart at 1 and not exceed Statement::GetColumnCount()
        */
        Field & Column(PINDEX column) const;

        /** Retreive Field Data given the Column Name
        */
        Field & Column(const PString & name) const;

        /** Retrieve Field Data given specified column
        */
        Field & operator[](PINDEX column) const { return Column(column); }

        /** Retrieve Field Data given the column Name.
        */
        Field & operator[](const PString & columnName) const { return Column(columnName); }

        /** Column Name
        */
        PString ColumnName(PINDEX column) const { return Column(column).GetName(); }

        /** ColumnTypes
        */
        unsigned ColumnType(PINDEX column) const { return Column(column).GetDataType(); }

        /** Column Size
        */
        PINDEX ColumnSize(PINDEX column) const { return Column(column).GetSize(); }

        /** Column Scale
        */
        unsigned ColumnScale(PINDEX column) const { return Column(column).GetScale(); }

        /** ColumnPrecision Get the Number of Decimal places
        if Precision is set the precision is set to the
        lessor of the Two.
        */
        unsigned ColumnPrecision(PINDEX column) const { return Column(column).GetPrecision(); }

        /** IsColumn Nullable. Accepts NULL value
        */
        bool IsColumnNullable(PINDEX column) const { return Column(column).IsNullable(); }

        /** IsColumn Updateable ie is not ReadOnly
        */
        bool IsColumnUpdatable(PINDEX column) const { return Column(column).IsUpdatable(); }

        /** IsColumnAutoIndex (ie don't give default Value)
        */
        bool IsColumnAutoIndex(PINDEX column) const { return Column(column).IsAutoIndex(); }
        //@}

        /**Make this row a new one.
          */
        void SetNewRow();

        /** Move to Specified Row
        */
        bool MoveTo(RowIndex row);

        /** Move to row relative to current position
        */
        bool Move(int offset);

        /**First record
          */
        bool First();

        /**Next record
          */
        bool Next();

        /**Previous record
          */
        bool Previous();

        /**Last record
          */
        bool Last();

        /** Delete the Current Record from the
        RecordSet
        */
        bool Delete(RowIndex rowIndex = 0);

        /** Post the Row back to the Database.
        When Row::NewRow is true the data
        can be posted back to the Database;
        If Edit Invoked then releasea the
        RowHandler for Navigation.
        */
        bool Commit();

        RowIndex GetRowIndex() const { return m_rowIndex; }

        // For backward compatibility
        PINDEX Rows();
        PINDEX ColumnCount() { return Columns(); }
        bool PostNew() { return Commit(); }
        bool PostUpdate() { return Commit(); }
        bool Post() { return Commit(); }
        bool Navigate(RowIndex row) { return MoveTo(row); }

      protected:
        RecordSet   & m_recordSet;
        RowIndex      m_rowIndex;    /// Current row number, starting at 1, 0 == new
        PArray<Field> m_fields;    /// PODBC::Field Array Cache (Used for New Row)

      private:
        Row(const Row & other) : PObject(other), m_recordSet(other.m_recordSet) { }
        void operator=(const Row &) { }

      friend class Field;
      friend class RecordSet;
    };
    //@}


    /** PODBC::RecordSet
    This is the main Class to access Data returned by a Select Query.
    */
    class RecordSet : public PObject
    {
        PCLASSINFO(RecordSet, PObject);
      public:
        /**@name Constructor/Deconstructor */
        //@{
        /** Constructor
        Using the HDBC and TableName/Select SQL Query
        creates a virtual table in the OBDC driver.
        */
        RecordSet(PODBC & odbc, const PString & query = PString::Empty());

        // For backward compatibility
        RecordSet(PODBC * odbc, const PString & query);

        /// Destroy the record set and free resources used
        ~RecordSet();
        //@}

        /**@name Data Storage */
        //@{
        /** Set the SQL query for this record set.
            If \p query matches a table then a simple SELECT is made.
          */
        bool Query(const PString & query);

        /** Set the SQL query to a SELECT for this record set.
          */
        bool Select(
          const PString & table,
          const PString & whereClause = PString::Empty(),
          const PString & fields = PString::Empty(), ///< '*' if empty
          const PString & orderedBy = PString::Empty(),
          bool descending = false
        );

        /** Returns the Number of Rows in the Resultant RecordSet.
            Note this can be a very time expensive operation, so only set forceCount
            if really you want it.
        */
        RowIndex Rows(bool forceCount = true);

        /** Columns. Returns the Number of Columns in the Resultant RecordSet
        */
        PINDEX Columns() { return m_cursor.Columns(); }

        /** Column Name
        */
        PString ColumnName(PINDEX column) { return m_cursor.ColumnName(column); }

        /** ColumnNames. Return the list of column Names of the Resultant RecordSet
        */
        PStringArray ColumnNames() { return m_cursor.ColumnNames(); }

        /** Add New Row
        */
        Row & NewRow();

        /** Move to Specified Row
        */
        bool MoveTo(RowIndex row) { return m_cursor.MoveTo(row); }

        /** Move to row relative to current position
        */
        bool Move(int offset) { return m_cursor.Move(offset); }

        /**First record
          */
        bool First() { return m_cursor.First(); }

        /**Next record
          */
        bool Next() { return m_cursor.Next(); }

        /**Previous record
          */
        bool Previous() { return m_cursor.Previous(); }

        /**Last record
          */
        bool Last() { return m_cursor.Last(); }

        /** Delete Row 0 indicates Current Row
        */
        bool DeleteRow(RowIndex row = 0);

        /** Row return the fetched row in the Cached RecordSet. An Array of PODBC::Field
            Index is 1 based.
        */
        Row & operator[](PINDEX row);

        /** Returns the Field data at a predetermined position in the Resultant
        RecordSet. It Fetches the Row than isolates the Column from the fetched
        data.
        */
        Field & operator()(RowIndex row, PINDEX col);

        /** Returns the indicated Column Holder for the RecordSet,
        This can be used for iterative Row calls.
        */
        Field & Column(PINDEX column) { return m_cursor.Column(column); }

        /** Returns the indicated Column Holder Name for the RecordSet,
        */
        Field & Column(const PString & name) { return m_cursor.Column(name); }

        /**Commit data to record set.
        */
        bool Commit() { return m_cursor.Commit(); }
        //@}

        // For backward compatibility
        bool Post() { return m_cursor.Commit(); }

      protected:
        Statement * m_statement; // ODBC Fetched Statement Info
        RowIndex    m_totalRows;
        Row         m_cursor;

      private:
        RecordSet(const RecordSet & other);
        void operator=(const RecordSet &) { }

      friend class Field;
      friend class Row;
    };

    typedef RecordSet Table; // For backward compatibility


    /**@name DataSource Access */
    //@{
    /** Driver types that are supported by this implementation.
    */
    P_DECLARE_ENUM(DriverType,
      DSN,
      mySQL,
      postgreSQL,
      Oracle,
      IBM_DB2,
      MSSQL,
      MSAccess,
      Paradox,
      Foxpro,
      dBase,
      Excel,
      Ascii,
      ConnectionString
    );
    static const char * GetDriverName(DriverType type);
    friend std::ostream & operator<<(std::ostream & strm, DriverType type) { return strm << PODBC::GetDriverName(type); }


    /** MSSQL protocols.If your interested?
    */
    enum MSSQLProtocols
    {
      MSSQLNamedPipes,
      MSSQLWinSock,
      MSSQLIPX,
      MSSQLBanyan,
      MSSQLRPC
    };

    /** This class is a multipurpose use
    class for storing parameters when
    initiating connection to DataSource.
    Not all field are required. By default
    all non-essential params are set to a
    datasource specific default value.
    */
    struct ConnectData
    {
      ConnectData() : m_driver(DSN), m_port(0), m_exclusive(false), m_trusted(false), m_options(0) { }

      DriverType m_driver;    ///< Driver type
      PString    m_database;  ///< Database name or file Path (not Oracle,xxSQL)
      PDirectory m_directory; ///< Used with Paradox/DBase/Excel (& mySQL db)
      PString    m_username;  ///< UID
      PString    m_password;  ///< Password
      PString    m_host;      ///< Host name to connect to source
      unsigned   m_port;      ///< Port to connect to source
      bool       m_exclusive; ///< Whether Datasource is locked.
      bool       m_trusted;   ///< Whether Datasource is trusted.
      int        m_options;   ///< General Option Value.mySQL & Paradox
    };

    /**Connect to database using ConnectData
    This is the main function to call to contact a
    DataSource. Source specifies the Type of DataSource
    to contact and the Data parameter contain the relevent
    connection information. You can choose to call this function
    or use the specific Connection function.
    */
    bool Connect(const ConnectData & connectInfo);

    /** General Connect Function
    Custom connection strings should call this
    to connect.
    */
    bool Connect(
      const PString & source
    );

    /**@name Connection/Disconnect */
    //@{
    /** Connect to the MDAC using a pre-existing MDAC Defined DataSource
    This is different than calling PODBC::DataSource in that the
    Data Source is known defined externally within MDAC,
    */
    bool Connect(
      const PString & source,
      const PString & username,
      const PString & password
    );

    /** Connect to IBM DB2 DataSource
    */
    bool Connect_DB2(
      const PFilePath & dbPath
    );

    /** Connect to MS Office excel spreadsheet
    */
    bool Connect_XLS(
      const PFilePath & xlsPath,
      const PString & defDir = PString::Empty()
    );

    /** Connect to an ascii text or cvs file
    */
    bool Connect_TXT(
      const PFilePath & txtPath
    );

    /** Connect to a Foxpro dataSource
    */
    bool Connect_FOX(
      const PFilePath & dbPath,
      const PString & user = PString::Empty(),
      const PString & pass = PString::Empty(),
      const PString & type = "DBF",
      bool exclusive = false
    );

    /** Connect to a MS Access *.mdb DataSource.
    */
    bool Connect_MDB(
      const PFilePath & mdbPath,
      const PString & user = PString::Empty(),
      const PString & pass = PString::Empty(),
      bool exclusive = false
    );

    /** Connect to a paradox database datastore
    */
    bool Connect_PDOX(
      const PDirectory & dbPath,
      const PDirectory & defaultDir,
      int version = 5
    );

    /** Connect to an Oracle Datasource
    */
    bool Connect_Oracle(
      const PString & server,
      const PString & user = PString::Empty(),
      const PString & pass = PString::Empty());

    /** Connect to a DBase DataStore
    */
    bool Connect_DBASE(
      const PDirectory & dbPath
    );

    /** Connect to a MS SQL Server
    */
    bool Connect_MSSQL(
      const PString & user = PString::Empty(),
      const PString & pass = PString::Empty(),
      const PString & host = "(local)",
      bool trusted = true,
      MSSQLProtocols Proto = MSSQLNamedPipes
    );

    /** Connect to a mySQL Server
    */
    bool Connect_mySQL(
      const PString & user = PString::Empty(),
      const PString & pass = PString::Empty(),
      const PString & host = "localhost",
      int port = 3306
    );

    /** Connect to a mySQL Server's specified DataBase.
    */
    bool ConnectDB_mySQL(
      const PString & db,
      const PString & user = PString::Empty(),
      const PString & pass = PString::Empty(),
      const PString & host = "localhost",
      int port = 3306
    );

    /** Connect to a postgreSQL Server
    */
    bool Connect_postgreSQL(
      const PString & db,
      const PString & user,
      const PString & pass,
      const PString & host,
      int port = 5432
    );

    /** Return true if connected
      */
    bool IsConnected() const;

    /**Call back on successful connection.
      */
    virtual void OnConnected();

    /** General Disconnect from DataSource.
    */
    void Disconnect();

    /**Get a list of driver descriptions and attributes.
       The returned strings when\p withAttribute true are of the form
          name\tattr=val\tattr=val etc
     */
    PStringList GetDrivers(
      bool withAttributes = true
    ) const;

    /**Get a list of known data sources and their descriptions
       The returned strings when \p withDescription true are of the form
          name\tdescription
      */
    PStringList GetSources(
      bool system = false,
      bool withDescription = true
    ) const;
    //@}

    /**@name SQL */
    //@{
    /** Retrieve a List of Tables in the Datasource
    use the option field to specify the type of
    data to access. ie "CATALOGS", "SCHEMAS", "TABLE" or "VIEW"
    Note case is significant.
    */
    PStringArray TableList(const PString & options = PString::Empty());

    /** Added Information to the DataSource. Use this
    function if you just want to use a SQL statement
    to add data to a datasource without retreiving the
    data itself. ie "UPDATE" "APPEND" "INSERT" queries.
    */
    bool Execute(const PString & sql);

    // For backward compatibility
    __inline bool Query(const PString & sql) { return Execute(sql); }
    //@}


    /**@name Utilities */
    //@{
    /**Get valid field type for PVarType enumeration.
       This will get the database specific field name for the driver type.
      */
    static PString GetFieldType(DriverType driver, PVarType::BasicType type, unsigned size = 0);

    /** Set the Number of Decimal places to
    round to By Default it is 4. However if the field
    decimal places is less then Precision Value the
    field rounding will be used. This must be set prior
    to calling LoadTable()
    */
    void SetPrecision(unsigned precision);

    unsigned GetPrecision() const { return m_precision; }

    PTime::TimeFormat GetTimeFormat() const { return m_timeFormat; }
    void SetTimeFormat(PTime::TimeFormat fmt) { m_timeFormat = fmt; }

    PTime::TimeFormat GetDateFormat() const { return m_dateFormat; }
    void SetDateFormat(PTime::TimeFormat fmt) { m_dateFormat = fmt; }

    PTime::TimeFormat GetDateTimeFormat() const { return m_dateTimeFormat; }
    void SetDateTimeFormat(PTime::TimeFormat fmt) { m_dateTimeFormat = fmt; }

    /** Maximum size of an individual chunk of data.
        This is the maximum size for a field to be transferred in
        one go. Checks if database can do large transfers via the
        SQL_NEED_LONG_DATA_LEN info item. If needed then is set to 32k
        otherwise is P_MAX_INDEX
      */
    PINDEX GetMaxChunkSize() const { return m_needChunking ? m_maxChunkSize : P_MAX_INDEX; }

    /// Get last error
    int GetLastError() const { return m_lastError; }

    /// Get last error text
    PString GetLastErrorText() const { return m_lastErrorText; }

    /** OnSQL Error
    */
    virtual void OnSQLError(
      int native,               ///< Native error code
      const PString & code,     ///< 5 character SQLSTATE code
      const PString & message   ///< Human readable error message
    );
    //@}

    // For backward compatibility
    bool DataSource(DriverType driver, ConnectData Data);

  protected:
    struct Link;

    Link            * m_link;
    int               m_lastError;
    PString           m_lastErrorText;
    unsigned          m_precision;   /// Double Real Float Decimal digit rounding def= 4;
    PTime::TimeFormat m_timeFormat;
    PTime::TimeFormat m_dateFormat;
    PTime::TimeFormat m_dateTimeFormat;
    bool              m_needChunking;
    PINDEX            m_maxChunkSize;

    P_REMOVE_VIRTUAL_VOID(OnSQLError(const PString &, const PString &));

  friend class Statement;
};


// For backward compatibility
typedef PODBC::Statement PODBCStmt;
typedef PODBC::Row POBDCRecord;
typedef PODBC PDSNConnection;


#endif // P_ODBC

#endif // PTLIB_PODBC_H


// End Of File ///////////////////////////////////////////////////////////////
