/*
* ODBCTest.cxx
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
* The Original Code is derived from and used in conjunction with the 
* pwlib Libaray of the OpenH323 Project (www.openh323.org/)
*
* The Initial Developer of the Original Code is ISVO (Asia) Pte Ltd.
*
*
* Contributor(s): ______________________________________.
*
*/


#include <ptlib.h>
#include <ptlib/pprocess.h>
#include <ptclib/podbc.h>

#if P_ODBC

class ODBCtest : public PProcess
{
  PCLASSINFO(ODBCtest, PProcess)
public:
  void Main();
};


PCREATE_PROCESS(ODBCtest)

  void ODBCtest::Main()
{
  cout << "ODBC Component for the Pwlib Library Test Program\n"
          "=================================================\n\n";

  PArgList & args = GetArguments();
  args.Parse("d-database:"
             "H-host:"
             "P-port:"
             "u-username:"
             "p-password:"
#if PTRACING
             "o-output:"
             "t-trace."
#endif
             "h-help."
    );

  if (args.HasOption('h')) {
    cerr << "usage: " << GetFile().GetTitle() << " [ options ] [ <driver> ]\n"
         << "Options:\n"
         << "  -d --database X    : Database name\n"
         << "  -H --host X        : Host name\n"
         << "  -P --port X        : Port number\n"
         << "  -u --username X    : User name\n"
         << "  -p --password X    : Password\n"
#if PTRACING
         << "  -t --trace         : Enable trace, use multiple times for more detail\n"
         << "  -o --output        : File for trace output, default is stderr\n"
#endif
         << endl;
    return;
  }

#if PTRACING
  PTrace::Initialise(args.GetOptionCount('t'),
                     args.HasOption('o') ? (const char *)args.GetOptionString('o') : NULL,
         PTrace::Blocks | PTrace::Timestamp | PTrace::Thread | PTrace::FileAndLine);
#endif

  PODBC link;

  {
    cout << "Drivers available:" << endl;
    PStringList drivers = link.GetDrivers();
    for (PStringList::iterator it = drivers.begin(); it != drivers.end(); ++it) {
      it->Replace("\t", "\n    ", true);
      cout << "  " << *it << endl;
    }
  }

  {
    cout << "User sources available:" << endl;
    PStringList sources = link.GetSources(false);
    for (PStringList::iterator it = sources.begin(); it != sources.end(); ++it)
      cout << "  " << *it << endl;
  }

  {
    cout << "System sources available:" << endl;
    PStringList sources = link.GetSources(true);
    for (PStringList::iterator it = sources.begin(); it != sources.end(); ++it)
      cout << "  " << *it << endl;
  }

  PODBC::ConnectData data;

  if (args.GetCount() == 0)
    data.m_driver = PODBC::MSAccess;
  else {
    for (data.m_driver = PODBC::BeginDriverType; data.m_driver < PODBC::EndDriverType; ++data.m_driver) {
      if (args[0].NumCompare(PODBC::GetDriverName(data.m_driver)) == EqualTo)
        break;
    }
    if (data.m_driver == PODBC::EndDriverType) {
      cout << "\n\nCannot use driver \"" << args[0] << '"' << endl;
      return;
    }
  }

  data.m_database = args.GetOptionString('d', "test.mdb");
  data.m_host = args.GetOptionString('H');
  data.m_port = args.GetOptionString('P', "0").AsUnsigned();
  data.m_username = args.GetOptionString('u');
  data.m_password = args.GetOptionString('p');

  cout << "\n\nOpening " << data.m_driver << ", database " << data.m_database << endl;

  if (!link.Connect(data)) {
    cout << "ODBC cannot connect: " << link.GetLastErrorText() << endl;
    return;
  }

  cout << "Connected Access Database\n" << endl;

  /// Settings
  link.SetPrecision(2);			// Number of Decimal Places (def = 4)   
  link.SetDateTimeFormat(PTime::MediumDate);	// Set the Default Display Time

  /// Enumerate catalogs in Database
  {
    cout << "Catalogs in Database:" << endl;
    PStringArray catalogs = link.TableList("CATALOGS");
    for (PINDEX c = 0; c < catalogs.GetSize(); c++)
      cout << "  " << catalogs[c] << endl;
  }

  /// Enumerate schemas in Database
  {
    cout << "Schemas in Database:" << endl;
    PStringArray schemas = link.TableList("SCHEMAS");
    for (PINDEX s = 0; s < schemas.GetSize(); s++)
      cout << "  " << schemas[s] << endl;
  }

  /// Enumerate schemas in Database
  {
    cout << "Types in Database:" << endl;
    PStringArray types = link.TableList("TYPES");
    for (PINDEX t = 0; t < types.GetSize(); t++)
      cout << "  " << types[t] << endl;
  }

  /// Enumerate Tables in Database
  ///+++++++++++++++++++++++++++++
  /// You can also use the QUERY keyword to view Queries
  cout << "Tables in Database:" << endl;
  PStringArray tables = link.TableList("TABLE");
  for (PINDEX t = 0; t < tables.GetSize(); t++)
    cout << "  " << tables[t] << endl;

  cout << "Views in Database:" << endl;
  PStringArray views = link.TableList("VIEW");
  for (PINDEX v = 0; v < views.GetSize(); v++)
    cout << "  " << views[v] << endl;

  cout << "------------------------------------------------------------------\n" << endl;

  /// Viewing Database Contents
  ///++++++++++++++++++++++++++
  /// Add Select SQL Statement
  tables.AppendString("SELECT Clients.Ref, Calls.Date, Calls.CalledParty, Calls.Duration "
                      "FROM Clients "
                      "INNER JOIN Calls ON Clients.Ref = Calls.Ref "
                      "WHERE ((Clients.Ref)=1)");

  for (PINDEX t = 0; t < tables.GetSize(); t++) {   
    if (t > 0)
      cout << "=================" << endl;

    cout << "Query Table: " << tables[t] << endl;
    PODBC::RecordSet table(&link, tables[t]);
    cout << "Columns: " << table.Columns() << " Rows: " << table.Rows() << endl;
    if (table.Columns() == 0)
      continue; // Did not load.

    cout << "ColumnNames : " << endl;
    PStringArray columnNames = table.ColumnNames();

    for (PINDEX c = 0; c < columnNames.GetSize(); c++) {
      if (c > 0)
        cout << ", ";
      cout << columnNames[c];
    }
    cout << endl;

    // You can also Reference via Record Handler 
    //	  PODBC:: Row & row = table.RecordHandler();
    //	  PODBC::Field & f1 = row.Column(1);
    //		...etc...
    // or Access the Field directly via the Recordset 
    //	  PODBC::Field f1 = table(row.col) 

    // Display Table Contents
    for (PODBC::RowIndex r = 1; r <= table.Rows(); r++) {
      for (PINDEX c = 1; c <= table.Columns(); c++) {
        if (c > 1)
          cout << ", ";
        cout << table(r, c).AsString();
      }
      cout << endl;
    }
  }
  cout << endl;

  /// Table Modification Examples 
  /// +++++++++++++++++++++++++++
  cout << "Modify Table Calls" << endl;
  PODBC::Table ntable(&link,"Calls");

  cout << endl;

  /// Delete a Record (Directly Via RecordSet)
  cout << "Delete the Last Record #" << ntable.Rows() << endl;

  if (ntable.DeleteRow(ntable.Rows()))
    cout << "Last Record Deleted.." << endl;
  else
    cout << "Error Deleting Last Record: " << link.GetLastErrorText() << endl;

  cout << endl;

  /// Update a Field (Using RecordHolder callRef is field 2)
  {
    cout << "Add 1 to the callRef field of the First Row" << endl;
    PODBC::Row & row = ntable[1];
    int num = row[2].AsInteger();
    cout << "Old Value " << num << " ";
    row[2].SetValue(num+1);
    if (row.Commit())
      cout << "Committed." << endl;
    else
      cout << "Commit failed: " << link.GetLastErrorText() << endl;
    row.Next(); // Move away
    cout << "New Value " << ntable[1][2].AsString() << endl;
  }

  cout << endl;

  {
    /// Adding a New Record (Using Column Names)
    cout << "Add New Record to Calls Table" << endl;
    PODBC::Row & row = ntable.NewRow();
    row[1] = 12;
    ntable.Column("CallRef").SetValue(1324);
    row.Column("Ref").SetValue("2");
    row["Date"] = PTime();
    row["Duration"] = 3.14;
    row["CalledParty"] = "Fred's brother";
    if (ntable.Commit())
      cout << "New Record Added!" << endl;
    else
      cout << "new record add failed: " << link.GetLastErrorText() << endl;

    cout << endl;
  }

  {
    /// Display the RecordSet Contents thro' the RecordSet. (x,y)
    cout << "Display Table with new Record" << endl;
    PODBC::Row & row = ntable[1];
    do {
      for (PINDEX c = 1; c <= ntable.Columns(); c++) {
        if (c > 1)
          cout << ", ";
        cout << row[c];
      }
      cout << endl;
    } while (row.Next());
    cout << "Rows " << ntable.Rows() << endl;
  }

  {
    link.Execute("DROP TABLE test_create;");
    PStringStream sql;
    sql   << "CREATE TABLE test_create (\n";
    for (PVarType::BasicType t = PVarType::VarBoolean; t <= PVarType::VarTime; t = (PVarType::BasicType)(t+1))
      sql << "  field" << (unsigned)t << ' ' << PODBC::GetFieldType(data.m_driver, t) << ",\n";
    sql   << "  char10 " << PODBC::GetFieldType(data.m_driver, PVarType::VarFixedString, 10) << ",\n"
             "  varchar20 " << PODBC::GetFieldType(data.m_driver, PVarType::VarStaticString, 20) << ",\n"
             "  varchar2000 " << PODBC::GetFieldType(data.m_driver, PVarType::VarDynamicString, 2000) << ",\n"
             "  varchar_infinite " << PODBC::GetFieldType(data.m_driver, PVarType::VarDynamicString) << ",\n"
             "  binary_100 " << PODBC::GetFieldType(data.m_driver, PVarType::VarStaticBinary, 100) << ",\n"
             "  binary_1000 " << PODBC::GetFieldType(data.m_driver, PVarType::VarDynamicBinary, 1000) << ",\n"
             "  binary_infinite " << PODBC::GetFieldType(data.m_driver, PVarType::VarDynamicBinary) << "\n"
             ");";
    if (link.Execute(sql))
      cout << "Created new table";
    else
      cout << "Create of new table failed: " << link.GetLastErrorText() << '\n' << sql << endl;
    cout << '\n' << sql << endl;

    {
      PODBC::RecordSet add(link, "test_create");
      add.NewRow().Column("varchar_infinite") = "This is a very, very long string! 01\n"
                                                "This is a very, very long string! 02\n"
                                                "This is a very, very long string! 03\n"
                                                "This is a very, very long string! 04\n"
                                                "This is a very, very long string! 05\n"
                                                "This is a very, very long string! 06\n"
                                                "This is a very, very long string! 07\n"
                                                "This is a very, very long string! 08\n"
                                                "This is a very, very long string! 09\n"
                                                "This is a very, very long string! 10\n"
                                                "This is a very, very long string! 11\n"
                                                "This is a very, very long string! 12\n";
      add.Commit();
    }

    PODBC::RecordSet check(link, "test_create");
    cout << "long text data:\n" << check[1]["varchar_infinite"] << endl;
  }
}

#else
#error Cannot compile Lua test program without ODBC support!
#endif // P_ODBC
