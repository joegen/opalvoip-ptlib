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

#define MAX_PPP_DEVICES		 4
#define	DEFAULT_TIMEOUT		 "90"
#define	DEFAULT_PPD_OPTS_STR	 "-detach crtscts modem defaultroute lock"
#define	DEFAULT_LOGIN_STR	 ""
#define	DEFAULT_BAUDRATE	 "9600"
#define	DEFAULT_ERRORS_STR	 "ABORT 'NO CARRIER' ABORT BUSY ABORT 'NO DIALTONE'"
#define	DEFAULT_MODEMINIT_STR	 "'' ATE1Q0Z OK"
#define	DEFAULT_DIALPREFIX_STR	 "ATDT"
#define DEFAULT_PPPD_STR         "pppd"
#define DEFAULT_CHAT_STR         "chat"

static const PXErrorStruct ErrorTable[] =
{
  // Remote connection errors
  { 1000, "Attempt to open remote connection with empty system name" },
  { 1001, "Attempt to open connection to unknown remote system \"%s\""},
  { 1002, "Cannot find \"Device\" entry for remote system \"%s\""},
  { 1003, "Cannot find \"Dial\" entry for remote system \"%s\""},
  { 1004, "Cannot find free PPP device"},
};

static int PPPDeviceExists(const char * devName);

static PRemoteConnection::Status status;

PDECLARE_CLASS(PXRemoteThread, PThread)
  public:
    PXRemoteThread(PXRemoteThread ** myPtr, const PString & cmdLine);
    void Main();
    void KillPipeChannel();

  protected:
    int retVal;
    PPipeChannel * pipeChannel;
    PXRemoteThread ** myPtr;
};

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

static const PString RemoteStr = "Remote-";
static const PString PortStr = "Port";
static const PString DeviceStr = "Device";
static const PString NumberStr   = "Number";
static const PString DialPrefixStr   = "DialPrefix";
static const PString LoginStr = "Login";
static const PString TimeoutStr = "TimeoutStr";
static const PString PPPDOptsStr = "PPPDOpts";
static const PString BaudRateStr = "BaudRate";
static const PString ErrorsStr = "Errors";
static const PString InitStr = "Init";
static const PString GeneralStr = "General";
static const PString PPPDStr    = "PPPD";
static const PString ChatStr    = "Chat";

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
  PConfig config(PConfig::System);
  PString str;
  PString sectionName = RemoteStr + name;
  if ((str = config.GetString(sectionName, PortStr, "")).IsEmpty()) {
    status = NoNameOrNumber;
    PProcess::PXShowSystemWarning(1001, ErrorTable[1].str);
    return FALSE;
  }

  // Try and find a connected PPP device. If we find one, then we assume
  // we are connected and use that device. Otherwise, we assume PPPD will
  // create the device when it runs
  PINDEX i;
  for (i = 0; i < MAX_PPP_DEVICES; i++) {
    pppDeviceName = psprintf("ppp%i", i);
    if (PPPDeviceExists(pppDeviceName) > 0) {
      Close();
      status = Connected;
      remoteThread = NULL;
      return TRUE;
    }
  }

  // cannot open remote connection with port not in config file
  PString portSectionName = PortStr + "-" + str;
  if ((str = config.GetString(portSectionName, DeviceStr, "")).IsEmpty()) {
    status = NoNameOrNumber;
    PProcess::PXShowSystemWarning(1002, ErrorTable[2].str);
    return FALSE;
  }
  PString device = str;

  // if already have a connection open, and it isn't the same one,
  // then close it
  if (name != remoteName) 
    Close();

  // name        = name of configuration
  // sectionName = name of config section
  remoteName = name;

  ///////////////////////////////////////////
  //
  // get name of program, if specified, in the general section
  //
  config.SetDefaultSection(GeneralStr);
  PString pppdStr = config.GetString(PPPDStr, DEFAULT_PPPD_STR);
  PString chatStr = config.GetString(ChatStr, DEFAULT_CHAT_STR);

  ///////////////////////////////////////////
  //
  // get remote system parameters
  //
  config.SetDefaultSection(sectionName);
  PString dialNumber = config.GetString(NumberStr);
  PString loginStr   = config.GetString(LoginStr,    DEFAULT_LOGIN_STR);
  PString timeout    = config.GetString(TimeoutStr,  DEFAULT_TIMEOUT);
  PString pppdOpts   = config.GetString(PPPDOptsStr, DEFAULT_PPD_OPTS_STR);

  // make sure we have a dial string
  if (dialNumber.IsEmpty()) {
    status = NoNameOrNumber;
    PProcess::PXShowSystemWarning(1003, ErrorTable[3].str);
    return FALSE;
  }

  ///////////////////////////////////////////
  //
  // get port parameters
  //
  config.SetDefaultSection(portSectionName);
  PString baudRate   = config.GetString(BaudRateStr,   DEFAULT_BAUDRATE);
  PString chatErrs   = config.GetString(ErrorsStr,     DEFAULT_ERRORS_STR);
  PString modemInit  = config.GetString(InitStr,       DEFAULT_MODEMINIT_STR);
  PString dialPrefix = config.GetString(DialPrefixStr, DEFAULT_DIALPREFIX_STR);

  ///////////////////////////////////////////
  //
  // instigate a dial using pppd
  //
  PString chatCmd     = chatErrs & modemInit & dialPrefix + dialNumber & loginStr;
  PString commandLine = pppdStr & device & baudRate & pppdOpts;
  if (!chatCmd.IsEmpty()) 
    commandLine &= PString("connect \"" + chatStr & "-t") + timeout &
                   chatCmd + "\"";

  remoteThread = PNEW PXRemoteThread(&remoteThread, commandLine);

  // dialling is now in progress
  status = InProgress;
  return TRUE;
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
  remoteThread = NULL;
  status       = Idle;
}


BOOL PRemoteConnection::Open()
{
  return Open(remoteName);
}


void PRemoteConnection::Close()
{
  if (remoteThread != NULL) {
    remoteThread->KillPipeChannel();
    for (PINDEX i = 0; i < 30 && remoteThread != NULL; i++)
      PThread::Current()->Sleep(1000);
  }
  remoteThread = NULL;
}

PRemoteConnection::Status PRemoteConnection::GetStatus() const
{
  if (remoteThread != NULL) {
    if (PPPDeviceExists(pppDeviceName) > 0)
      return Connected;
    else
      return InProgress;
  }

  if (PPPDeviceExists(pppDeviceName) > 0)
    return Connected;
  return Idle;
}

PStringArray PRemoteConnection::GetAvailableNames() 
{
  PStringArray names;

  // get the list of remote system names from the system config file
  PConfig config(PConfig::System);

  // remotes have section names of the form "Remote-x" where X is some
  // unique identifier, usually a number or the system name
  PStringList sections = config.GetSections();
  for (PINDEX i = 0; i < sections.GetSize(); i++) {
    PString sectionName = sections[i];
    if (sectionName.GetLength() > RemoteStr.GetLength()+1 &&
        sectionName.Find(RemoteStr) == 0)
      names[names.GetSize()] = sectionName.Mid(RemoteStr.GetLength(), P_MAX_INDEX);
  }

  return names;
}

PXRemoteThread::PXRemoteThread(PXRemoteThread ** ptr, const PString & cmdLine)
  : PThread(20000, AutoDeleteThread), myPtr(ptr)
{
  PStringArray argArray;
  argArray[0] = "-c";
  argArray[1] = cmdLine;
  pipeChannel = PNEW PPipeChannel("/bin/sh", argArray);
  Resume();
}

void PXRemoteThread::Main()
{
  pipeChannel->PXWaitForTerminate();
  retVal = pipeChannel->GetReturnCode();
  delete pipeChannel;
  pipeChannel = NULL;
  *myPtr = NULL;
}

void PXRemoteThread::KillPipeChannel()
{
  if (pipeChannel != NULL) {
    pipeChannel->PXKill();
    pipeChannel->PXWaitForTerminate();
  }
}

//
//  <0 = does not exist
//   0 = exists, but is down
//  >0 = exists, is up
//
static int PPPDeviceExists(const char * devName)
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
