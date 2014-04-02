/*
 * netif.cxx
 *
 * Sample program to query the network interfaces.
 *
 * Portable Windows Library
 *
 * Copyright (c) 2001 Roger Hardiman
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
 * The Initial Developer of the Original Code is Roger Hardiman
 *
 * $Revision$
 * $Author$
 * $Date$
 */

/*
 * The network interface test program displays information about the
 * network connections and setup.
 * It displays the results of
 * GetInterfaceTable()
 * and GetRouteTable()
 */


#include <ptlib.h>
#include <ptlib/pprocess.h>
#include <ptlib/sockets.h>

class NetTest : public PProcess
{
  PCLASSINFO(NetTest, PProcess)
  public:
    void Main();
};


PCREATE_PROCESS(NetTest)

void NetTest::Main()
{
  cout << "Net Test Utility\n\n";

  // Read the interface table
  PIPSocket::InterfaceTable if_table;

  // Read the Interface Table
  if ( !PIPSocket::GetInterfaceTable( if_table ) ) {
    cout << "GetInterfaceTable() failed. No interface table" << endl;
    return;
  }

  // Display the interface table
  cout << "The interface table has " << if_table.GetSize() << " entries\n"
       << left;

  for (PINDEX i=0; i < if_table.GetSize(); i++) {
    PIPSocket::InterfaceEntry if_entry = if_table[i];
    cout << setw(2) << i
         << ' ' << setw(10) << if_entry.GetName()
         << ' ' << setw(30) << if_entry.GetAddress()
         << ' ' << setw(40) << if_entry.GetNetMask()
         << ' ' << setw(17) << if_entry.GetMACAddress()
         << endl;
  }
  cout << endl;

  // Read the Route Table
  PIPSocket::RouteTable rt_table;

  if ( !PIPSocket::GetRouteTable( rt_table ) ) {
    cout << "GetRouteTable() failed. No routing table" << endl;
    return;
  }

  // Display the route table
  cout << "The route table has " << rt_table.GetSize() <<" entries" << endl;

  for (PINDEX i=0; i < rt_table.GetSize(); i++) {
    PIPSocket::RouteEntry rt_entry = rt_table[i];
    cout << setw(2) << i
         << ' ' << setw(30) << rt_entry.GetNetwork()
         << ' ' << setw(40) << rt_entry.GetNetMask()
         << ' ' << setw(20) << rt_entry.GetDestination()
         << ' ' << setw(10) << rt_entry.GetInterface()
         << ' '             << rt_entry.GetMetric()
         << endl;
  }
  cout << endl;

  // Get the Default Gateway
  PIPSocket::Address addr = PIPSocket::GetGatewayAddress();
  if (addr.IsValid())
    cout << "Default v4 gateway is " << addr << endl;
  else
    cout << "Could not determine default v4 gateway\n";

  addr = PIPSocket::GetGatewayAddress(6);
  if (addr.IsValid())
    cout << "Default v6 gateway is " << addr << endl;
  else
    cout << "Could not determine default v6 gateway\n";
  cout << endl;

  // Get the interface for the Default Gateway
  PString gw_if = PIPSocket::GetGatewayInterface();
  if (gw_if.IsEmpty() )
    cout << "No default v4 gateway interface" << endl;
  else
    cout << "Default v4 gateway is on " << gw_if << endl;

  gw_if = PIPSocket::GetGatewayInterface(6);
  if (gw_if.IsEmpty() )
    cout << "No default v6 gateway interface" << endl;
  else
    cout << "Default v6 gateway is on " << gw_if << endl;
  cout << endl;


  PArgList & args = GetArguments();
  args.Parse("l");

  if (args.HasOption('l')) {
    for (PINDEX arg = 0; arg < args.GetCount(); ++arg) {
      PSimpleTimer start;
      bool result = PIPSocket::IsLocalHost(args[arg]);
      cout << args[arg] << " is " << (result ? "local" : "remote") << " - " << start.GetElapsed() << " seconds" << endl;
    }
  }
  else {
    for (PINDEX arg = 0; arg < args.GetCount(); ++arg) {
      PIPSocket::Address addr(args[arg]);
      if (addr.IsValid())
        cout << "Interface used for " << addr
             << " is " << PIPSocket::GetRouteInterfaceAddress(addr) << endl;
      else
        cout << "Invalid IP address \"" << args[arg] << '"' << endl;
    }
  }
}

// End of netif.cxx

