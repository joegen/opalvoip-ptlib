/*
 * RemConn.CXX
 *
 * Simple proxy service for internet access under Unix
 *
 * Copyright 1995 Equivalence
 */

#pragma implementation "remconn.h"

#include <ptlib.h>
#include <pipechan.h>
#include <remconn.h>

#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#if defined(P_LINUX)
#include <linux/if.h>
#define HAS_IFREQ
#endif

#if defined(P_SUN4) || defined(P_SOLARIS)
#include <net/if.h>
#include <sys/sockio.h>
#define HAS_IFREQ
#endif

#include "uerror.h"

static const PString RasStr      = "ras";
static const PString NumberStr   = "Number";
static const PCaselessString UsernameStr = "$USERID";
static const PCaselessString PasswordStr = "$PASSWORD";
static const PString NameServerStr = "NameServer";

static const PString OptionsStr = "Options";

static const PString DeviceStr     = "Device";
static const PString DefaultDevice = "ppp0";

static const PString PPPDStr     = "PPPD";
static const PString DefaultPPPD = "pppd";

static const PString ChatStr     = "Chat";
static const PString DefaultChat = "chat";

static const PString PortStr     = "Port";
static const PString DefaultPort = "/dev/modem";

static const PString DialPrefixStr     = "DialPrefix";
static const PString DefaultDialPrefix = "ATDT";

static const PString LoginStr     = "Login";
static const PString DefaultLogin = "";

static const PString TimeoutStr     = "TimeoutStr";
static const PString DefaultTimeout = "90";

static const PString PPPDOptsStr     = "PPPDOpts";
static const PString PPPDOpts        = "-detach";
static const PString DefaultPPPDOpts = "crtscts modem defaultroute lock";

static const PString BaudRateStr     = "BaudRate";
static const PString DefaultBaudRate = "57600";

static const PString ErrorsStr     = "Errors";
static const PString DefaultErrors = "ABORT 'NO CARRIER' ABORT BUSY ABORT 'NO DIALTONE'";

static const PString InitStr     = "Init";
static const PString DefaultInit = "'' ATE1Q0Z OK";


static const PXErrorStruct ErrorTable[] =
{
  // Remote connection errors
  { 1000, "Attempt to open remote connection with empty system name" },
  { 1001, "Attempt to open connection to unknown remote system"},
  { 1002, "pppd could not connect to remote system"},
};

static int PPPDeviceStatus(const char * devName);

PRemoteConnection::PRemoteConnection()
{
  Construct();
}

PRemoteConnection::PRemoteConnection(const PString & name)
  : remoteName(name)
{
  Construct();
}

PRemoteConnection::~PRemoteConnection()
{
  Close();
}


BOOL PRemoteConnection::Open(const PString & name)
{
  return Open(name, "", "");
}

BOOL PRemoteConnection::Open(const PString & name,
                             const PString & user,
                             const PString & pword)
{
  userName = user;
  password = pword;

  // cannot open remote connection with an empty name
  if (name.IsEmpty()) {
    status = NoNameOrNumber;
    PProcess::PXShowSystemWarning(1000, ErrorTable[0].str);
    return FALSE;
  }

  // cannot open remote connection not in config file
  PConfig config(0, RasStr);
  PString phoneNumber;
  if ((phoneNumber = config.GetString(name, NumberStr, "")).IsEmpty()) {
    status = NoNameOrNumber;
    PProcess::PXShowSystemWarning(1001, ErrorTable[1].str);
    return FALSE;
  }

  // if there is a connection active, check to see if it has the same name
  if (pipeChannel != NULL &&
      pipeChannel->IsRunning() &&
      name == remoteName &&
      PPPDeviceStatus(deviceStr) > 0) {
    status = Connected;
    return TRUE;
  }
  Close();

  // name        = name of configuration
  // sectionName = name of config section
  remoteName = name;

  ///////////////////////////////////////////
  //
  // get global options
  //
  config.SetDefaultSection(OptionsStr);
  deviceStr          = config.GetString(DeviceStr,     DefaultDevice);
  PString pppdStr    = config.GetString(PPPDStr,       DefaultPPPD);
  PString chatStr    = config.GetString(ChatStr,       DefaultChat);
  PString baudRate   = config.GetString(BaudRateStr,   DefaultBaudRate);
  PString chatErrs   = config.GetString(ErrorsStr,     DefaultErrors);
  PString modemInit  = config.GetString(InitStr,       DefaultInit);
  PString dialPrefix = config.GetString(DialPrefixStr, DefaultDialPrefix);
  PString portName   = config.GetString(PortStr,       DefaultPort);
  PString pppdOpts   = config.GetString(PPPDOptsStr,   DefaultPPPDOpts);

  ///////////////////////////////////////////
  //
  // get remote system parameters
  //
  config.SetDefaultSection(remoteName);
  PString loginStr   = config.GetString(LoginStr,    DefaultLogin);
  PString timeoutStr = config.GetString(TimeoutStr,  DefaultTimeout);
  PINDEX timeout = timeoutStr.AsInteger();
  PString nameServerStr = config.GetString(NameServerStr, "");

  ///////////////////////////////////////////
  //
  // replace metastrings in the login string
  //
  loginStr.Replace(UsernameStr, user);
  loginStr.Replace(PasswordStr, pword);

  ///////////////////////////////////////////
  //
  // setup the chat command
  //
  PString chatCmd     = chatErrs & modemInit & dialPrefix + phoneNumber & loginStr;
  PString commandLine = pppdStr & portName & baudRate & PPPDOpts & pppdOpts;

  if (!nameServerStr.IsEmpty())
    commandLine &= "ipparam " & nameServerStr;

  if (!chatCmd.IsEmpty()) 
    commandLine &= PString("connect \"" + chatStr & "-t") + timeoutStr &
                   chatCmd + "\"";

  ///////////////////////////////////////////
  //
  // instigate a dial using pppd
  //
  PStringArray argArray;
  argArray[0]  = "-c";
  argArray[1]  = commandLine;
  pipeChannel  = PNEW PPipeChannel("/bin/sh", argArray);

  ///////////////////////////////////////////
  //
  //  wait until the dial succeeds, or times out
  //
  PTimer timer(timeout*1000);
  for (;;) {
    if (pipeChannel == NULL || !pipeChannel->IsRunning()) 
      break;

    if (PPPDeviceStatus(deviceStr) > 0) 
      return TRUE;

    if (!timer.IsRunning())
      break;

    PThread::Current()->Sleep(1000);
  }

  ///////////////////////////////////////////
  //
  //  dial failed
  //
  Close();

  return FALSE;
}


PObject::Comparison PRemoteConnection::Compare(const PObject & obj) const
{
  return remoteName.Compare(((const PRemoteConnection &)obj).remoteName);
}


PINDEX PRemoteConnection::HashFunction() const
{
  return remoteName.HashFunction();
}


void PRemoteConnection::Construct()
{
  pipeChannel  = NULL;
  status       = Idle;
}


BOOL PRemoteConnection::Open()
{
  return Open(remoteName);
}


void PRemoteConnection::Close()
{
  if (pipeChannel != NULL) {

    // give pppd a chance to clean up
    pipeChannel->PXKill(SIGINT);

    PTimer timer(10*1000);
    for (;;) {
      if (!pipeChannel->IsRunning() ||
          (PPPDeviceStatus(deviceStr) <= 0) ||
          !timer.IsRunning())
        break;
      PThread::Current()->Sleep(1000);
    }

    // kill the connection for real
    delete pipeChannel;
    pipeChannel = NULL;
  }
}

PRemoteConnection::Status PRemoteConnection::GetStatus() const
{
  if (pipeChannel != NULL &&
      pipeChannel->IsRunning() &&
      PPPDeviceStatus(deviceStr) > 0) 
    return Connected;

  return Idle;
}

PStringArray PRemoteConnection::GetAvailableNames() 
{
  PStringArray names;

  // get the list of remote system names from the system config file
  PConfig config(0, RasStr);

  // remotes have section names of the form "Remote-x" where X is some
  // unique identifier, usually a number or the system name
  PStringList sections = config.GetSections();
  for (PINDEX i = 0; i < sections.GetSize(); i++) {
    PString sectionName = sections[i];
    if (sectionName != OptionsStr)
      names[names.GetSize()] = sectionName;
  }

  return names;
}

//
//  <0 = does not exist
//   0 = exists, but is down
//  >0 = exists, is up
//
static int PPPDeviceStatus(const char * devName)
{
#if defined(HAS_IFREQ)
  int skfd;
  struct ifreq ifr;

  // Create a channel to the NET kernel. 
  if ((skfd = socket(AF_INET, SOCK_DGRAM,0)) < 0) 
    return -1;

  // attempt to get the status of the ppp connection
  int stat;
  strcpy(ifr.ifr_name, devName);
  if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0)
    stat = -1;
  else 
    stat = (ifr.ifr_flags & IFF_UP) ? 1 : 0;

  // attempt to get the status of the ppp connection
  close(skfd);
  return stat;
#else
#warning "No PPPDeviceExists implementation defined"
  return FALSE;
#endif
}


// End of File ////////////////////////////////////////////////////////////////
