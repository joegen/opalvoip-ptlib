/*
 * cli.cxx
 *
 * Command line interpreter
 *
 * Copyright (C) 2006-2008 Post Increment
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
 * The Original Code is WOpenMCU
 *
 * The Initial Developer of the Original Code is Post Increment
 *
 * Contributor(s): Craig Southeren (craigs@postincrement.com)
 *                 Robert Jongbloed (robertj@voxlucida.com.au)
 *
 * Portions of this code were written by Post Increment (http://www.postincrement.com)
 * with the assistance of funding from US Joint Forces Command Joint Concept Development &
 * Experimentation (J9) http://www.jfcom.mil/about/abt_j9.htm
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#include <ptlib.h>
#include <ptclib/cli.h>


///////////////////////////////////////////////////////////////////////////////

PCLI::Context::Context(PCLI & cli)
  : m_cli(cli)
  , m_newLine(cli.GetNewLine())
  , m_ignoreNextLF(false)
  , m_thread(NULL)
{
}


PCLI::Context::~Context()
{
  Stop();
}


bool PCLI::Context::Start()
{
  if (!IsOpen()) {
    PTRACE(2, "PCLI\tCannot start context, not open.");
    return false;
  }

  if (m_thread == NULL)
    m_thread = PThread::Create(PCREATE_NOTIFIER(ThreadMain), "CLI Context");

  return true;
}


void PCLI::Context::Stop()
{
  Close();

  if (m_thread != NULL) {
    m_thread->WaitForTermination(10000);
    delete m_thread;
    m_thread = NULL;
  }
}


void PCLI::Context::OnStart()
{
  *this << m_cli.GetPrompt() << ::flush;
}


void PCLI::Context::OnStop()
{
}


void PCLI::Context::WriteLine(const PString & str)
{
  *this << str << m_newLine << ::flush;
}


bool PCLI::Context::ReadAndProcessInput()
{
  if (!IsOpen())
    return false;

  int ch = ReadChar();
  if (ch < 0) {
    PTRACE(2, "PCLI\tRead error: " << GetErrorText(PChannel::LastReadError));
    return false;
  }
  
  ProcessInput((char)ch);
  return true;
}


void PCLI::Context::ProcessInput(char ch)
{
  switch (ch) {
    case '\n' :
      if (m_ignoreNextLF) {
        m_ignoreNextLF = false;
        return;
      }
      break;

    case '\r' :
      m_ignoreNextLF = true;
      break;

    case '\b' : // Backspace
    case '\x7f' : // Delete
      if (!m_commandLine.IsEmpty()) {
        m_commandLine.Delete(m_commandLine.GetLength()-1, 1);
        WriteString("\b \b");
      }
      // Do next case

    default :
      if (isprint(ch))
        m_commandLine += ch;
      m_ignoreNextLF = false;
      return;
  }

  OnCompletedLine();
  m_commandLine.MakeEmpty();
  WriteString(m_cli.GetPrompt());
}


void PCLI::Context::OnCompletedLine()
{
  PCaselessString line = m_commandLine.Trim();
  if (line.IsEmpty())
    return;

  PTRACE(4, "PCLI\tProcessing command line \"" << line << '"');

  if (line.NumCompare(m_cli.GetRepeatCommand()) == EqualTo) {
    if (m_commandHistory.IsEmpty()) {
      *this << m_cli.GetNoHistoryError() << m_newLine << ::flush;
      return;
    }

    line = m_commandHistory.back();
  }

  if (line == m_cli.GetHistoryCommand()) {
    unsigned cmdNum = 1;
    for (PStringList::iterator cmd = m_commandHistory.begin(); cmd != m_commandHistory.end(); ++cmd)
      *this << cmdNum++ << ' ' << *cmd << m_newLine;
    flush();
    return;
  }

  if (line.NumCompare(m_cli.GetHistoryCommand()) == EqualTo) {
    PINDEX cmdNum = line.Mid(m_cli.GetHistoryCommand().GetLength()).AsUnsigned();
    if (cmdNum <= 0 || cmdNum > m_commandHistory.GetSize()) {
      *this << m_cli.GetNoHistoryError() << m_newLine << ::flush;
      return;
    }

    line = m_commandHistory[cmdNum-1];
  }

  if (line == m_cli.GetHelpCommand())
    m_cli.ShowHelp(*this);
  else {
    Arguments args(*this, line);
    m_cli.OnReceivedLine(args);
  }

  m_commandHistory += line;
}


void PCLI::Context::ThreadMain(PThread &, INT)
{
  PTRACE(4, "PCLI\tContext thread started");

  if (IsOpen()) {
    OnStart();
    while (ReadAndProcessInput())
      ;
    OnStop();
  }

  PTRACE(4, "PCLI\tContext thread ended");
}


///////////////////////////////////////////////////////////////////////////////

PCLI::Arguments::Arguments(Context & context, const PString & rawLine)
  : PArgList(rawLine)
  , m_context(context)
{
}


void PCLI::Arguments::WriteUsage()
{
  if (m_usage.IsEmpty())
    return;

  PStringArray lines = m_usage.Lines();
  for (PINDEX i = 0; i < lines.GetSize(); ++i)
    m_context << lines[i] << m_context.GetNewLine();
  m_context.flush();
}


void PCLI::Arguments::WriteError(const PString & error)
{
  PStringArray lines = (m_error&error).Lines();
  for (PINDEX i = 0; i < lines.GetSize(); ++i)
    m_context << lines[i] << m_context.GetNewLine();
  m_context.flush();
}


///////////////////////////////////////////////////////////////////////////////

PCLI::PCLI(const char * prompt)
  : m_prompt(prompt != NULL ? prompt : "CLI> ")
  , m_newLine('\n')
  , m_helpCommand("?")
  , m_helpOnHelp("Use ? to display help\n"
                 "Use ! to list history of commands\n"
                 "Use !n to repeat the n'th command\n"
                 "Use !! to repeat last command\n"
                 "\n"
                 "Command available are:")
  , m_repeatCommand("!!")
  , m_historyCommand("!")
  , m_noHistoryError("No command history")
  , m_commandUsagePrefix("Usage:")
  , m_commandErrorPrefix(": error:")
  , m_unknownCommandError("Unknown command")
{
}


PCLI::~PCLI()
{
  Stop();
}


bool PCLI::Start(bool runInBackground)
{
  if (runInBackground) {
    PTRACE(4, "PCLI\tStarting background contexts");
    m_contextMutex.Wait();
    for (ContextList_t::iterator iter = m_contextList.begin(); iter != m_contextList.end(); ++iter)
      (*iter)->Start();
    m_contextMutex.Signal();
    return true;
  }

  if (m_contextList.size() != 1) {
    PTRACE(2, "PCLI\tCan only start in foreground if have one context.");
    return false;
  }

  Context * context = m_contextList.front();
  if (!context->IsOpen()) {
    PTRACE(2, "PCLI\tCannot start foreground processing, context not open.");
    return false;
  }

  context->OnStart();
  while (context->ReadAndProcessInput())
    ;
  return true;
}


void PCLI::Stop()
{
  m_contextMutex.Wait();
  for (ContextList_t::iterator iter = m_contextList.begin(); iter != m_contextList.end(); ++iter) {
    (*iter)->Stop();
    delete *iter;
  }
  m_contextMutex.Signal();
}


bool PCLI::StartContext(PChannel * channel, bool autoDelete, bool runInBackground)
{
  PCLI::Context * context = AddContext();
  if (context == NULL)
    return false;

  if (!context->Open(channel, autoDelete)) {
    PTRACE(2, "PCLI\tCould not open context: " << context->GetErrorText());
    return false;
  }

  if (runInBackground)
    return context->Start();

  return true;
}


bool PCLI::StartContext(PChannel * readChannel,
                        PChannel * writeChannel,
                        bool autoDeleteRead,
                        bool autoDeleteWrite,
                        bool runInBackground)
{
  PCLI::Context * context = AddContext();
  if (context == NULL)
    return false;
  
  if (!context->Open(readChannel, writeChannel, autoDeleteRead, autoDeleteWrite)) {
    PTRACE(2, "PCLI\tCould not open context: " << context->GetErrorText());
    return false;
  }

  if (runInBackground)
    return context->Start();

  return true;
}


PCLI::Context * PCLI::CreateContext()
{
  return new Context(*this);
}


PCLI::Context * PCLI::AddContext(Context * context)
{
  if (context == NULL) {
    context = CreateContext();
    if (context == NULL) {
      PTRACE(2, "PCLI\tCould not create a context!");
      return context;
    }
  }

  m_contextMutex.Wait();
  m_contextList.push_back(context);
  m_contextMutex.Signal();

  return context;
}


void PCLI::RemoveContext(Context * context)
{
  if (!PAssert(context != NULL, PInvalidParameter))
    return;

  context->Close();

  m_contextMutex.Wait();

  for (ContextList_t::iterator iter = m_contextList.begin(); iter != m_contextList.end(); ++iter) {
    if (*iter == context) {
      delete context;
      m_contextList.erase(iter);
      break;
    }
  }

  m_contextMutex.Signal();
}


void PCLI::OnReceivedLine(Arguments & args)
{
  for (PINDEX nesting = 1; nesting <= args.GetCount(); ++nesting) {
    PString names;
    for (PINDEX i = 0; i < nesting; ++i)
      names &= args[i];

    CommandMap_t::iterator cmd = m_commands.find(names);
    if (cmd != m_commands.end()) {
      args.Shift(nesting);
      args.m_usage = GetCommandUsagePrefix() & cmd->second.m_usage;
      args.m_error = cmd->first & GetCommandErrorPrefix();
      cmd->second.m_notifier(args, 0);
      return;
    }
  }

  args.GetContext().WriteLine(GetUnknownCommandError());
}


void PCLI::Broadcast(const PString & message) const
{
  for (ContextList_t::const_iterator iter = m_contextList.begin(); iter != m_contextList.end(); ++iter)
    (*iter)->WriteLine(message);
}


bool PCLI::SetCommand(const char * command, const PNotifier & notifier, const char * help, const char * usage)
{
  if (!PAssert(command != NULL && *command != '\0' && !notifier.IsNULL(), PInvalidParameter))
    return false;

  bool good = true;

  PStringArray synonymArray = PString(command).Lines();
  for (PINDEX s = 0; s < synonymArray.GetSize(); ++s) {
    // Normalise command to remove any duplicate spaces, should only
    // have one as in " conf  show   members   " -> "conf show members"
    PStringArray nameArray = synonymArray[s].Tokenise(' ', false);
    PString names;
    for (PINDEX n = 0; n < nameArray.GetSize(); ++n)
      names &= nameArray[n];

    if (m_commands.find(names) != m_commands.end())
      good = false;
    else {
      InternalCommand & cmd = m_commands[names];
      cmd.m_notifier = notifier;
      cmd.m_help = help;
      if (usage != NULL && *usage != '\0')
        cmd.m_usage = names & usage;
    }
  }

  return good;
}


void PCLI::ShowHelp(Context & context)
{
  PINDEX i;
  CommandMap_t::const_iterator cmd;

  PINDEX maxCommandLength = GetHelpCommand().GetLength();
  for (cmd = m_commands.begin(); cmd != m_commands.end(); ++cmd) {
    PINDEX len = cmd->first.GetLength();
    if (maxCommandLength < len)
      maxCommandLength = len;
  }

  PStringArray lines = GetHelpOnHelp().Lines();
  for (i = 0; i < lines.GetSize(); ++i)
    context << lines[i] << context.GetNewLine();

  for (cmd = m_commands.begin(); cmd != m_commands.end(); ++cmd) {
    if (cmd->second.m_help.IsEmpty() && cmd->second.m_usage.IsEmpty())
      context << cmd->first;
    else {
      context << left << setw(maxCommandLength) << cmd->first << "   ";

      if (cmd->second.m_help.IsEmpty())
        context << GetCommandUsagePrefix(); // Earlier condiiton says must have usage
      else {
        lines = cmd->second.m_help.Lines();
        context << lines[0];
        for (i = 1; i < lines.GetSize(); ++i)
          context << context.GetNewLine() << setw(maxCommandLength+3) << ' ' << lines[i];
      }

      lines = cmd->second.m_usage.Lines();
      for (i = 0; i < lines.GetSize(); ++i)
        context << context.GetNewLine() << setw(maxCommandLength+5) << ' ' << lines[i];
    }
    context << context.GetNewLine();
  }

  context.flush();
}


///////////////////////////////////////////////////////////////////////////////

PCLIStandard::PCLIStandard(const char * prompt)
  : PCLI(prompt)
{
}


bool PCLIStandard::Start(bool runInBackground)
{
  if (m_contextList.empty())
    StartContext(new PConsoleChannel(PConsoleChannel::StandardInput),
                 new PConsoleChannel(PConsoleChannel::StandardOutput),
                 true, true, runInBackground);
  return PCLI::Start(runInBackground);
}


///////////////////////////////////////////////////////////////////////////////

PCLISocket::PCLISocket(const char * prompt, WORD port, bool singleThreadForAll)
  : PCLI(prompt)
  , m_singleThreadForAll(singleThreadForAll)
  , m_listenSocket(port)
{
}


PCLISocket::~PCLISocket()
{
  Stop();
}


bool PCLISocket::Start(bool runInBackground)
{
  if (!Listen())
    return false;

  if (runInBackground) {
    m_thread = PThread::Create(PCREATE_NOTIFIER(ThreadMain), "CLI Server");
    return m_thread != NULL;
  }

  if (!m_contextList.empty())
    return false;

  while (m_singleThreadForAll ? HandleSingleThreadForAll() : HandleIncoming())
    ;
  return true;
}


void PCLISocket::Stop()
{
  if (m_thread != NULL) {
    m_listenSocket.Close();
    m_thread->WaitForTermination(10000);
    delete m_thread;
    m_thread = NULL;
  }

  PCLI::Stop();
}


PCLI::Context * PCLISocket::AddContext(Context * context)
{
  context = PCLI::AddContext(context);

  PTCPSocket * socket = dynamic_cast<PTCPSocket *>(context->GetReadChannel());
  if (socket != NULL) {
    m_contextMutex.Wait();
    m_contextBySocket[socket] = context;
    m_contextMutex.Signal();
  }

  return context;
}


void PCLISocket::RemoveContext(Context * context)
{
  if (context == NULL)
    return;

  PTCPSocket * socket = dynamic_cast<PTCPSocket *>(context->GetReadChannel());
  if (socket != NULL) {
    m_contextMutex.Wait();

    ContextMap_t::iterator iter = m_contextBySocket.find(socket);
    if (iter != m_contextBySocket.end())
      m_contextBySocket.erase(iter);

    m_contextMutex.Signal();
  }

  PCLI::RemoveContext(context);
}


bool PCLISocket::Listen(WORD port)
{
  if (!m_listenSocket.Listen(5, port, PSocket::CanReuseAddress)) {
    PTRACE(2, "PCLI\tCannot open PCLI socket on port " << port
           << ", error: " << m_listenSocket.GetErrorText());
    return false;
  }

  PTRACE(4, "PCLI\tCLI socket opened on port " << port);
  return true;
}


void PCLISocket::ThreadMain(PThread &, INT)
{
  PTRACE(4, "PCLI\tServer thread started on port " << GetPort());

  while (m_singleThreadForAll ? HandleSingleThreadForAll() : HandleIncoming())
    ;

  PTRACE(4, "PCLI\tServer thread ended for port " << GetPort());
}


bool PCLISocket::HandleSingleThreadForAll()
{
  // create list of listening sockets
  PSocket::SelectList readList;
  readList += m_listenSocket;

  m_contextMutex.Wait();
  for (ContextMap_t::iterator iter = m_contextBySocket.begin(); iter != m_contextBySocket.end(); ++iter)
    readList += *iter->first;
  m_contextMutex.Signal();

  // wait for something to become available
  if (PIPSocket::Select(readList) == PChannel::NoError) {
    // process sockets
    for (PSocket::SelectList::iterator socket = readList.begin(); socket != readList.end(); ++socket) {
      if (&*socket == &m_listenSocket)
        HandleIncoming();
      else {
        ContextMap_t::iterator iterContext = m_contextBySocket.find(&*socket);
        if (iterContext != m_contextBySocket.end()) {
          char buffer[1024];
          if (socket->Read(buffer, sizeof(buffer)-1)) {
            for (PINDEX i = 0; i < socket->GetLastReadCount(); ++i)
              iterContext->second->ProcessInput(buffer[i]);
          }
          else
            socket->Close();
        }
      }
    }
  }

  m_contextMutex.Wait();
  for (ContextMap_t::iterator iter = m_contextBySocket.begin(); iter != m_contextBySocket.end(); ++iter) {
    if (!iter->first->IsOpen())
      RemoveContext(iter->second);
  }
  m_contextMutex.Signal();

  return m_listenSocket.IsOpen();
}


bool PCLISocket::HandleIncoming()
{
  PTCPSocket * socket = new PTCPSocket;
  if (m_listenSocket.Accept(*socket)) {
    PTRACE(3, "PCLI\tIncoming connection from " << socket->GetPeerHostName());
    Context * context = CreateContext();
    if (context != NULL && context->Open(socket, true)) {
      if (m_singleThreadForAll)
        context->OnStart();
      else
        context->Start();
      return true;
    }
  }

  delete socket;
  return false;
}
