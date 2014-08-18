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
 * Further assistance for enhancements from Imagicle spa
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#include <ptlib.h>
#include <ptclib/cli.h>

#if P_CLI

#include <ptclib/telnet.h>

#if P_CURSES==1
  #include <ncurses.h>
#endif


#define PTraceModule() "CLI"


///////////////////////////////////////////////////////////////////////////////

PCLI::Context::Context(PCLI & cli)
  : m_cli(cli)
  , m_editPosition(0)
  , m_ignoreNextEOL(false)
  , m_historyPosition(0)
  , m_thread(NULL)
  , m_state(cli.GetUsername().IsEmpty() ? (cli.GetPassword().IsEmpty() ? e_CommandEntry : e_Password) : e_Username)
{
}


PCLI::Context::~Context()
{
  Stop();
  delete m_thread;
}


PBoolean PCLI::Context::Write(const void * buf, PINDEX len)
{
  if (m_cli.GetNewLine().IsEmpty())
    return PIndirectChannel::Write(buf, len);

  const char * newLinePtr = m_cli.GetNewLine();
  PINDEX newLineLen = m_cli.GetNewLine().GetLength();

  PINDEX written = 0;

  const char * str = (const char *)buf;
  const char * nextline;
  while (len > 0 && (nextline = strchr(str, '\n')) != NULL) {
    PINDEX lineLen = nextline - str;

    if (!PIndirectChannel::Write(str, lineLen))
      return false;

    written += GetLastWriteCount();

    if (!PIndirectChannel::Write(newLinePtr, newLineLen))
      return false;

    written += GetLastWriteCount();

    len -= lineLen+1;
    str = nextline+1;
  }

  if (!PIndirectChannel::Write(str, len))
    return false;

  lastWriteCount = written + GetLastWriteCount();
  return true;
}


bool PCLI::Context::Start()
{
  if (!IsOpen()) {
    PTRACE(2, "Cannot start context, not open.");
    return false;
  }

  if (m_thread == NULL)
    m_thread = PThread::Create(PCREATE_NOTIFIER(ThreadMain), "CLI Context");

  return true;
}


void PCLI::Context::Stop()
{
  // Close this way in case we are inside a ^C interrupt
  PChannel * base = GetBaseReadChannel();
  if (base != NULL)
    base->Close();

  if (m_thread != NULL && PThread::Current() != m_thread) {
    m_thread->WaitForTermination(10000);
    delete m_thread;
    m_thread = NULL;
  }
}


bool PCLI::Context::Run()
{
  if (!IsOpen())
    return false;

  OnStart();
  while (ReadAndProcessInput())
    ;
  OnStop();
  return true;
}


void PCLI::Context::OnStart()
{
  WritePrompt();
}


void PCLI::Context::OnStop()
{
}


bool PCLI::Context::WritePrompt()
{
  switch (m_state) {
    case e_Username :
      if (!m_cli.GetUsername().IsEmpty())
        return WriteString(m_cli.GetUsernamePrompt());
      // Do next case

    case e_Password :
      SetLocalEcho(false);
      if (!m_cli.GetPassword().IsEmpty())
        return WriteString(m_cli.GetPasswordPrompt());
      // Do next case

    default :
      return WriteString(m_cli.GetPrompt());
  }
}


bool PCLI::Context::ReadAndProcessInput()
{
  if (!IsOpen())
    return false;

  int ch = ReadChar();
  if (ch < 0) {
    PTRACE(2, "Read error: " << GetErrorText(PChannel::LastReadError));
    return false;
  }
  
  return ProcessInput(ch);
}


static bool IsCode(const PIntArray & codes, int ch)
{
  for (PINDEX i = 0; i < codes.GetSize(); ++i) {
    if (codes[i] == ch)
      return true;
  }
  return false;
}


bool PCLI::Context::InternalMoveCursor(bool left, PINDEX count)
{
  if (m_cli.GetRequireEcho() && m_state != e_Password) {
    while (count-- > 0) {
      if (!EchoInput(left ? '\b' : m_commandLine[m_editPosition]))
        return false;
      m_editPosition += left ? -1 : 1;
    }
  }

  return true;
}


bool PCLI::Context::InternalEchoCommandLine(PINDEX echoPosition, PINDEX moveLeftCount)
{
  if (m_cli.GetRequireEcho() && m_state != e_Password) {
    PINDEX i;
    // Move to the left
    for (i = 0; i < moveLeftCount; ++i) {
      if (!EchoInput('\b'))
        return false;
    }
    // Rewrite rest of command line from new position
    for (i = echoPosition; i < m_commandLine.GetLength(); ++i) {
      if (!EchoInput(m_commandLine[i]))
        return false;
    }
    // Space out characters left behind
    for (i = 0; i < moveLeftCount; ++i) {
      if (!EchoInput(' '))
        return false;
    }
    // Then move back over the erased bit
    for (i = 0; i < moveLeftCount; ++i) {
      if (!EchoInput('\b'))
        return false;
    }
    // Finally move back to the edit position
    for (i = m_editPosition; i < m_commandLine.GetLength(); ++i) {
      if (!EchoInput('\b'))
        return false;
    }
  }

  return true;
}


bool PCLI::Context::InternalMoveHistoryCommand(int direction)
{
  PINDEX oldPosition = m_commandLine.GetLength();
  if (!InternalMoveCursor(false, oldPosition-m_editPosition))
    return false;
  m_commandLine = m_commandHistory[m_historyPosition += direction];
  m_editPosition = m_commandLine.GetLength();
  return InternalEchoCommandLine(0, oldPosition);
}


bool PCLI::Context::ProcessInput(int ch)
{
  if (ch != '\n' && ch != '\r') {
    if (IsCode(m_cli.GetEditCodes(), ch)) {
      if (m_editPosition > 0) {
        m_commandLine.Delete(--m_editPosition, 1);
        if (!InternalEchoCommandLine(m_editPosition, 1))
          return false;
      }
    }
    else if (IsCode(m_cli.GetEraseCodes(), ch)) {
      PINDEX count = m_editPosition;
      m_commandLine.Delete(0, m_editPosition);
      m_editPosition = 0;
      if (!InternalEchoCommandLine(0, count))
        return false;
    }
    else if (IsCode(m_cli.GetLeftCodes(), ch)) {
      if (m_editPosition > 0) {
        if (!InternalMoveCursor(true, 1))
          return false;
      }
    }
    else if (IsCode(m_cli.GetRightCodes(), ch)) {
      if (m_editPosition < m_commandLine.GetLength()) {
        if (!InternalMoveCursor(false, 1))
          return false;
      }
    }
    else if (IsCode(m_cli.GetBeginCodes(), ch)) {
      if (!InternalMoveCursor(true, m_editPosition))
        return false;
    }
    else if (IsCode(m_cli.GetEndCodes(), ch)) {
      if (!InternalMoveCursor(false, m_commandLine.GetLength()-m_editPosition))
        return false;
    }
    else if (IsCode(m_cli.GetPrevCmdCodes(), ch)) {
      if (m_historyPosition > 0) {
        if (!InternalMoveHistoryCommand(-1))
          return false;
      }
    }
    else if (IsCode(m_cli.GetNextCmdCodes(), ch)) {
      if (m_historyPosition < m_commandHistory.GetSize()-1) {
        if (!InternalMoveHistoryCommand(1))
          return false;
      }
    }
    else if (IsCode(m_cli.GetAutoFillCodes(), ch)) {
      m_cli.ShowHelp(*this, m_commandLine);
    }
    else if (ch > 0 && ch < 256 && isprint(ch)) {
      m_commandLine.Splice(PString((char)ch), m_editPosition++);
      if (!InternalEchoCommandLine(m_editPosition-1, 0))
        return false;
    }

    m_ignoreNextEOL = false;
    return true;
  }

  m_editPosition = 0;
  m_historyPosition = m_commandHistory.GetSize()+1;

  if (m_ignoreNextEOL) {
    m_ignoreNextEOL = false;
    return true;
  }

  m_ignoreNextEOL = true;

  switch (m_state) {
    case e_Username :
      if (m_cli.GetPassword().IsEmpty()) {
        if (m_cli.OnLogIn(m_commandLine, PString::Empty()))
          m_state = e_CommandEntry;
      }
      else {
        m_enteredUsername = m_commandLine;
        m_state = e_Password;
      }
      break;

    case e_Password :
      if (!EchoInput('\n'))
        return false;

      if (m_cli.OnLogIn(m_enteredUsername, m_commandLine))
        m_state = e_CommandEntry;
      else
        m_state = m_cli.GetUsername().IsEmpty() ? (m_cli.GetPassword().IsEmpty() ? e_CommandEntry : e_Password) : e_Username;

      SetLocalEcho(m_state != e_Password);
      m_enteredUsername.MakeEmpty();
      break;

    default :
      OnCompletedLine();
  }

  m_commandLine.MakeEmpty();
  return WritePrompt();
}


bool PCLI::Context::EchoInput(char ch)
{
  switch (ch) {
    case '\n' :
      return WriteString(m_cli.GetNewLine());

    case '\x7f' :
      return WriteString("\b \b");

    default :
      return WriteChar(ch);
  }
}


static bool CheckInternalCommand(PCaselessString & line, const PCaselessString & cmdStr, bool noSpaceNeeded = false)
{
  PStringArray cmds = cmdStr.Lines();
  for (PINDEX i = 0; i < cmds.GetSize(); ++i) {
    if (line.NumCompare(cmds[i]) == PObject::EqualTo) {
      line.Delete(0, cmds[i].GetLength());
      if (line.IsEmpty())
        return true;
      if (!isspace(line[0]))
        return noSpaceNeeded;
      while (isspace(line[0]))
        line.Delete(0, 1);
      return true;
    }
  }

  return false;
}


void PCLI::Context::OnCompletedLine()
{
  PCaselessString line = m_commandLine.Trim();
  if (line.IsEmpty())
    return;

  PTRACE(4, "Processing command line \"" << line << '"');

  if (CheckInternalCommand(line, m_cli.GetCommentCommand(), true))
    return;

  if (CheckInternalCommand(line, m_cli.GetRepeatCommand())) {
    if (m_commandHistory.IsEmpty()) {
      *this << m_cli.GetNoHistoryError() << endl;
      return;
    }

    line = m_commandHistory.back();
  }

  if (CheckInternalCommand(line, m_cli.GetExitCommand())) {
    Stop();
    return;
  }

  if (CheckInternalCommand(line, m_cli.GetHistoryCommand())) {
    unsigned cmdNum = 1;
    for (PStringList::iterator cmd = m_commandHistory.begin(); cmd != m_commandHistory.end(); ++cmd)
      *this << cmdNum++ << ' ' << *cmd << '\n';
    flush();
    return;
  }

  if (CheckInternalCommand(line, m_cli.GetScriptCommand())) {
    PTextFile script;
    if (script.Open(line, PFile::ReadOnly)) {
      PString prompt = m_cli.GetPrompt();
      m_cli.SetPrompt(PString::Empty());
      m_state = e_ProcessingCommand;

      while (script.good()) {
        script >> line;
        if (line.IsEmpty() || CheckInternalCommand(line, m_cli.GetCommentCommand()))
          continue;

        Arguments args(*this, line);
        m_cli.OnReceivedLine(args);
      }

      m_state = e_CommandEntry;
      m_cli.SetPrompt(prompt);
    }
    else
      *this << m_cli.GetNoScriptError() << endl;
    return;
  }

  if (line.NumCompare(m_cli.GetHistoryCommand()) == EqualTo) {
    PINDEX cmdNum = line.Mid(m_cli.GetHistoryCommand().GetLength()).AsUnsigned();
    if (cmdNum <= 0 || cmdNum > m_commandHistory.GetSize()) {
      *this << m_cli.GetNoHistoryError() << endl;
      return;
    }

    line = m_commandHistory[cmdNum-1];
  }

  if (CheckInternalCommand(line, m_cli.GetHelpCommand()))
    m_cli.ShowHelp(*this, line);
  else {
    Arguments args(*this, line);
    m_state = e_ProcessingCommand;
    m_cli.OnReceivedLine(args);
    m_state = e_CommandEntry;
  }

  m_commandHistory += line;
}


void PCLI::Context::ThreadMain(PThread &, P_INT_PTR)
{
  PTRACE(4, "Context thread started");
  Run();
  PTRACE(4, "Context thread ended");
}


///////////////////////////////////////////////////////////////////////////////

PCLI::Arguments::Arguments(Context & context, const PString & rawLine)
  : PArgList(rawLine)
  , m_context(context)
{
}


PCLI::Context & PCLI::Arguments::WriteUsage()
{
  if (!m_usage.IsEmpty())
    m_context << m_context.GetCLI().GetCommandUsagePrefix() << m_usage << endl;
  return m_context;
}


PCLI::Context & PCLI::Arguments::WriteError(const PString & error)
{
  m_context << GetCommandName() << m_context.GetCLI().GetCommandErrorPrefix();
  if (!error.IsEmpty())
    m_context << error << endl;
  return m_context;
}


///////////////////////////////////////////////////////////////////////////////

static int EditCodes[]     = { '\b', '\x7f' };
static int EraseCodes[]    = { 'U'-64 };
static int LeftCodes[]     = { 'L'-64 };
static int RightCodes[]    = { 'R'-64 };
static int BeginCodes[]    = { 'B'-64 };
static int EndCodes[]      = { 'E'-64 };
static int PrevCmdCodes[]  = { 'P'-64 };
static int NextCmdCodes[]  = { 'N'-64 };
static int AutoFillCodes[] = { '\t' };

PCLI::PCLI(const char * prompt)
  : m_newLine("\r\n")
  , m_requireEcho(false)
  , m_editCodes(EditCodes, PARRAYSIZE(EditCodes), false)
  , m_eraseCodes(EraseCodes, PARRAYSIZE(EraseCodes), false)
  , m_leftCodes(LeftCodes, PARRAYSIZE(LeftCodes), false)
  , m_rightCodes(RightCodes, PARRAYSIZE(RightCodes), false)
  , m_beginCodes(BeginCodes, PARRAYSIZE(BeginCodes), false)
  , m_endCodes(EndCodes, PARRAYSIZE(EndCodes), false)
  , m_prevCmdCodes(PrevCmdCodes, PARRAYSIZE(PrevCmdCodes), false)
  , m_nextCmdCodes(NextCmdCodes, PARRAYSIZE(NextCmdCodes), false)
  , m_autoFillCodes(AutoFillCodes, PARRAYSIZE(AutoFillCodes), false)
  , m_prompt(prompt != NULL ? prompt : "CLI> ")
  , m_usernamePrompt("Username: ")
  , m_passwordPrompt("Password: ")
  , m_commentCommand("#\n;\n//\nrem ")
  , m_exitCommand("exit\nquit")
  , m_helpCommand("?\nhelp")
  , m_helpOnHelp("\n"
                 "Help"
                 "----"
                 "Use ? or 'help' to display help\n"
                 "Use ! to list history of commands\n"
                 "Use !n to repeat the n'th command\n"
                 "Use !! to repeat last command\n"
                 "Use read <filename> to read a script file as commands\n"
                 "\n"
                 "Commands available are:")
  , m_repeatCommand("!!")
  , m_historyCommand("!")
  , m_noHistoryError("No command history")
  , m_commandUsagePrefix("Usage: ")
  , m_commandErrorPrefix(": error: ")
  , m_unknownCommandError("Unknown command")
  , m_ambiguousCommandError("Ambiguous command")
  , m_scriptCommand("<\nread")
  , m_noScriptError("Script file could not be found")
{
}


PCLI::~PCLI()
{
  Stop();
}


bool PCLI::Start(bool runInBackground)
{
  if (runInBackground) {
    PTRACE(4, "Starting background contexts");
    m_contextMutex.Wait();
    for (ContextList_t::iterator iter = m_contextList.begin(); iter != m_contextList.end(); ++iter)
      (*iter)->Start();
    m_contextMutex.Signal();
    return true;
  }

  if (m_contextList.empty())
    StartForeground();

  if (m_contextList.size() != 1) {
    PTRACE(2, "Can only start in foreground if have one context.");
    return false;
  }

  Context * context = m_contextList.front();
  bool result = context->Run();
  RemoveContext(context);
  PTRACE_IF(2, !result, "Cannot start foreground processing, context not open.");
  return result;
}


void PCLI::Stop()
{
  m_contextMutex.Wait();
  for (ContextList_t::iterator iter = m_contextList.begin(); iter != m_contextList.end(); ++iter)
    (*iter)->Stop();
  m_contextMutex.Signal();

  GarbageCollection();
}


PCLI::Context * PCLI::StartContext(PChannel * readChannel,
                                   PChannel * writeChannel,
                                   bool autoDeleteRead,
                                   bool autoDeleteWrite,
                                   bool runInBackground)
{
  PCLI::Context * context = AddContext();
  if (context == NULL)
    return NULL;
  
  if (!context->Open(readChannel, writeChannel, autoDeleteRead, autoDeleteWrite)) {
    PTRACE(2, "Could not open context: " << context->GetErrorText());
    RemoveContext(context);
    return NULL;
  }

  if (runInBackground) {
    if (!context->Start()) {
      RemoveContext(context);
      return NULL;
    }
  }

  return context;
}


PCLI::Context * PCLI::StartForeground()
{
  return NULL;
}


bool PCLI::Run(PChannel * readChannel,
               PChannel * writeChannel,
               bool autoDeleteRead,
               bool autoDeleteWrite)
{
  Context * context = StartContext(readChannel, writeChannel, autoDeleteRead, autoDeleteWrite, false);
  if (context == NULL)
    return false;

  context->Run();
  RemoveContext(context);
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
      PTRACE(2, "Could not create a context!");
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


void PCLI::GarbageCollection()
{
  m_contextMutex.Wait();

  ContextList_t::iterator iter = m_contextList.begin();
  while (iter != m_contextList.end()) {
    Context * context = *iter;
    if (context->IsProcessingCommand() || context->IsOpen())
      ++iter;
    else {
      RemoveContext(context);
      iter = m_contextList.begin();
    }
  }

  m_contextMutex.Signal();
}


bool PCLI::InternalCommand::operator<(const InternalCommand & other) const
{
  PINDEX count = std::min(m_words.GetSize(), other.m_words.GetSize());
  for (PINDEX i = 0; i < count; ++i) {
    switch (m_words[i].Compare(other.m_words[i])) {
      case LessThan :
        return true;
      case GreaterThan :
        return false;
      default :
        break;
    }
  }

  return m_words.GetSize() < other.m_words.GetSize();
}


bool PCLI::InternalCommand::IsMatch(const PArgList & args, bool partial) const
{
  PINDEX count = m_words.GetSize();
  if (partial && count > args.GetCount())
    count = args.GetCount();

  for (PINDEX i = 0; i < count; ++i) {
    if (m_words[i].NumCompare(args[i]) != EqualTo)
      return false;
  }

  return true;
}


void PCLI::OnReceivedLine(Arguments & args)
{
  Commands_t::iterator cmd;
  for (cmd = m_commands.begin(); cmd != m_commands.end(); ++cmd) {
    if (cmd->IsMatch(args))
      break;
  }

  if (cmd == m_commands.end()) {
    args.GetContext() << GetUnknownCommandError() << endl;
    return;
  }

  {
    Commands_t::iterator nextCmd = cmd;
    if (++nextCmd != m_commands.end() && nextCmd->IsMatch(args)) {
      args.GetContext() << GetAmbiguousCommandError() << endl;
      return;
    }
  }

  args.Shift(cmd->m_words.GetSize());
  args.SetCommandName(cmd->m_command);
  args.m_usage = cmd->m_usage;

  if (!cmd->m_argSpec.IsEmpty()) {
    args.Parse(cmd->m_argSpec, true);
    if (!args.IsParsed()) {
      args.WriteUsage() << args.GetParseError();
      return;
    }
  }

  if (cmd->m_variable == NULL)
    cmd->m_notifier(args, 0);
  else {
    switch (cmd->m_variable->GetType()) {
      case PVarType::VarBoolean :
        OnSetBooleanCommand(args, *cmd);
        break;

      case PVarType::VarInt8:
      case PVarType::VarInt16:
      case PVarType::VarInt32:
      case PVarType::VarInt64:
      case PVarType::VarUInt8:
      case PVarType::VarUInt16:
      case PVarType::VarUInt32:
      case PVarType::VarUInt64:
        OnSetIntegerCommand(args, *cmd);
        break;

      default:
        break;
    }
  }
}


void PCLI::OnSetBooleanCommand(Arguments & args, const InternalCommand & cmd)
{
  if (args.GetCount() == 0) {
    args.GetContext() << cmd.m_varName << " = " << (cmd.m_variable->AsBoolean() ? "ON" : "OFF") << endl;
    return;
  }

  if ((args[0] *= "on") || (args[0] *= "yes") || (args[0] *= "true"))
    *cmd.m_variable = true;
  else if ((args[0] *= "off") || (args[0] *= "no") || (args[0] *= "false"))
    *cmd.m_variable = false;
  else {
    args.WriteUsage();
    return;
  }

  args.GetContext() << cmd.m_varName << " => " << (cmd.m_variable->AsBoolean() ? "ON" : "OFF") << endl;

  if (!cmd.m_notifier.IsNULL())
    cmd.m_notifier(args, cmd.m_variable->AsBoolean());
}


void PCLI::OnSetIntegerCommand(Arguments & args, const InternalCommand & cmd)
{
  if (args.GetCount() == 0) {
    args.GetContext() << cmd.m_varName << " = " << *cmd.m_variable << endl;
    return;
  }

  if (!isdigit(args[0][0])) {
    args.WriteUsage();
    return;
  }

  int64_t value = args[0].AsInt64();
  if (value < cmd.m_minimum->AsInteger64() || value > cmd.m_maximum->AsInteger64()) {
    args.WriteError() << cmd.m_varName << " outside of bounds " << *cmd.m_minimum << ".." << *cmd.m_maximum << endl;
    return;
  }

  *cmd.m_variable = value;
  args.GetContext() << cmd.m_varName << " => " << value << endl;

  if (!cmd.m_notifier.IsNULL())
    cmd.m_notifier(args, cmd.m_variable->AsInteger());
}


bool PCLI::OnLogIn(const PString & username, const PString & password)
{
  return m_username == username && m_password == password;
}


void PCLI::Broadcast(const PString & message) const
{
  for (ContextList_t::const_iterator iter = m_contextList.begin(); iter != m_contextList.end(); ++iter)
    **iter << message << endl;
  PTRACE(4, "Broadcast \"" << message << '"');
}


PCLI::InternalCommand::InternalCommand(const PNotifier & notifier,
                                       const char * help,
                                       const char * usage,
                                       const char * argSpec,
                                       const char * varName)
  : m_notifier(notifier)
  , m_help(help)
  , m_usage(usage)
  , m_argSpec(argSpec)
  , m_varName(varName)
  , m_variable(NULL)
  , m_minimum(NULL)
  , m_maximum(NULL)
{
}


PCLI::InternalCommand::InternalCommand(const InternalCommand & other)
  : m_words(other.m_words)
  , m_command(other.m_command)
  , m_notifier(other.m_notifier)
  , m_help(other.m_help)
  , m_usage(other.m_usage)
  , m_argSpec(other.m_argSpec)
  , m_varName(other.m_varName)
  , m_variable(other.m_variable != NULL ? other.m_variable->CloneAs<PVarType>() : NULL)
  , m_minimum(other.m_minimum != NULL ? other.m_minimum->CloneAs<PVarType>() : NULL)
  , m_maximum(other.m_maximum != NULL ? other.m_maximum->CloneAs<PVarType>() : NULL)
{
}


PCLI::InternalCommand::~InternalCommand()
{
  delete m_variable;
  delete m_minimum;
  delete m_maximum;
}


bool PCLI::InternalSetCommand(const char * commands, const InternalCommand & info)
{
  if (!PAssert(commands != NULL && *commands != '\0', PInvalidParameter))
    return false;

  if (!PAssert(info.m_variable != NULL || !info.m_notifier.IsNULL(), PInvalidParameter))
    return false;

  bool good = true;

  PStringArray synonymArray = PConstString(commands).Lines();
  for (PINDEX s = 0; s < synonymArray.GetSize(); ++s) {
    // Normalise command to remove any duplicate spaces, should only
    // have one as in " conf  show   members   " -> "conf show members"
    InternalCommand cmdToAdd(info);
    cmdToAdd.m_words = synonymArray[s].Tokenise(' ', false);
    for (PINDEX i = 0; i < cmdToAdd.m_words.GetSize(); ++i)
      cmdToAdd.m_command &= cmdToAdd.m_words[i];

    PArgList args(PString::Empty(), cmdToAdd.m_argSpec);
    args.SetCommandName(cmdToAdd.m_command);
    cmdToAdd.m_usage = args.Usage(cmdToAdd.m_usage, "");

    if (!m_commands.insert(cmdToAdd).second)
      good = false;
  }

  return good;
}


bool PCLI::SetCommand(const char * commands, const PNotifier & notifier, const char * help, const char * usage, const char * argSpec)
{
  return InternalSetCommand(commands, InternalCommand(notifier, help, usage, argSpec, NULL));
}


bool PCLI::SetCommand(const char * commands, bool & value, const char * varName, const char * help, const PNotifier & notifier)
{
  PStringStream adjustedHelp;
  if (help != NULL)
    adjustedHelp << help;
  else
    adjustedHelp << "Set " << varName << " on/off.";
  InternalCommand cmd(notifier, adjustedHelp, "[ \"on\" | \"off\" ]", NULL, varName);
  cmd.m_variable = new PRefVar<bool>(value);
  return InternalSetCommand(commands, cmd);
}


bool PCLI::SetCommand(const char * commands,
                      const PVarType & value,
                      const char * varName,
                      const PVarType & minValue,
                      const PVarType & maxValue,
                      const char * help,
                      const PNotifier & notifier)
{
  PStringStream adjustedHelp;
  if (help != NULL)
    adjustedHelp << help;
  else
    adjustedHelp << "Set integer " << varName;
  InternalCommand cmd(notifier, adjustedHelp, "[ <value> ]", NULL, varName);
  cmd.m_variable = value.CloneAs<PVarType>();
  cmd.m_minimum  = minValue.CloneAs<PVarType>();
  cmd.m_maximum  = maxValue.CloneAs<PVarType>();
  return InternalSetCommand(commands, cmd);
}


void PCLI::ShowHelp(Context & context, const PArgList & partial)
{
  PINDEX i;
  Commands_t::const_iterator cmd;

  PINDEX maxCommandLength = GetHelpCommand().GetLength();
  for (cmd = m_commands.begin(); cmd != m_commands.end(); ++cmd) {
    if (partial.GetCount() == 0 || cmd->IsMatch(partial, true)) {
      PINDEX len = cmd->m_command.GetLength();
      if (maxCommandLength < len)
        maxCommandLength = len;
    }
  }

  if (partial.GetCount() != 0)
    context << "\nMatching commands:\n";
  else
    context << setfill('\n') << GetHelpOnHelp().Lines() << setfill(' ');

  for (cmd = m_commands.begin(); cmd != m_commands.end(); ++cmd) {
    if (partial.GetCount() == 0 || cmd->IsMatch(partial, true)) {
      if (cmd->m_help.IsEmpty() && cmd->m_usage.IsEmpty())
        context << cmd->m_command;
      else {
        context << left << setw(maxCommandLength) << cmd->m_command << "   ";

        Commands_t::const_iterator duplicate;
        for (duplicate = m_commands.begin(); duplicate != cmd; ++duplicate) {
          if (duplicate->m_notifier.IsNULL() ? (duplicate->m_variable == cmd->m_variable)
                                             : (duplicate->m_notifier == cmd->m_notifier))
            break;
        }

        if (duplicate != cmd)
          context << "Synonym for command \"" << duplicate->m_command << '"';
        else {
          PStringArray lines;
          if (cmd->m_help.IsEmpty())
            context << GetCommandUsagePrefix(); // Earlier conditon says must have usage
          else {
            lines = cmd->m_help.Lines();
            context << lines[0];
            for (i = 1; i < lines.GetSize(); ++i)
              context << '\n' << setw(maxCommandLength+3) << ' ' << lines[i];
          }

          lines = cmd->m_usage.Lines();
          for (i = 0; i < lines.GetSize(); ++i)
            context << '\n' << setw(maxCommandLength+5) << ' ' << lines[i];
        }
      }
      context << '\n';
    }
  }

  context.flush();
}


///////////////////////////////////////////////////////////////////////////////

PCLIStandard::PCLIStandard(const char * prompt)
  : PCLI(prompt)
{
}


PCLI::Context * PCLIStandard::StartForeground()
{
  return StartContext(new PConsoleChannel(PConsoleChannel::StandardInput),
                      new PConsoleChannel(PConsoleChannel::StandardOutput),
                      true, true, false);
}


bool PCLIStandard::RunScript(PChannel * channel, bool autoDelete)
{
  PString prompt = GetPrompt();
  SetPrompt(PString::Empty());
  bool result = Run(channel, new PConsoleChannel(PConsoleChannel::StandardOutput), autoDelete, true);
  SetPrompt(prompt);
  return result;
}


///////////////////////////////////////////////////////////////////////////////

PCLISocket::PCLISocket(WORD port, const char * prompt, bool singleThreadForAll)
  : PCLI(prompt)
  , m_singleThreadForAll(singleThreadForAll)
  , m_listenSocket(port)
  , m_thread(NULL)
{
}


PCLISocket::~PCLISocket()
{
  Stop();
  delete m_thread;
}


bool PCLISocket::Start(bool runInBackground)
{
  if (!Listen())
    return false;

  if (runInBackground) {
    if (m_thread != NULL)
      return true;
    m_thread = PThread::Create(PCREATE_NOTIFIER(ThreadMain), "CLI Server");
    return m_thread != NULL;
  }

  while (m_singleThreadForAll ? HandleSingleThreadForAll() : HandleIncoming())
    GarbageCollection();
  return true;
}


void PCLISocket::Stop()
{
  m_listenSocket.Close();

  if (m_thread != NULL && PThread::Current() != m_thread) {
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
    PTRACE(2, "Cannot open PCLI socket on port " << port
           << ", error: " << m_listenSocket.GetErrorText());
    return false;
  }

  PTRACE(4, "CLI socket opened on port " << m_listenSocket.GetPort());
  return true;
}


void PCLISocket::ThreadMain(PThread &, P_INT_PTR)
{
  PTRACE(4, "Server thread started on port " << GetPort());

  while (m_singleThreadForAll ? HandleSingleThreadForAll() : HandleIncoming())
    GarbageCollection();

  PTRACE(4, "Server thread ended for port " << GetPort());
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
            PINDEX count = socket->GetLastReadCount();
            for (PINDEX i = 0; i < count; ++i) {
              if (!iterContext->second->ProcessInput(buffer[i]))
                socket->Close();
            }
          }
          else
            socket->Close();
        }
      }
    }
  }

  return m_listenSocket.IsOpen();
}


bool PCLISocket::HandleIncoming()
{
  PTCPSocket * socket = CreateSocket();
  if (socket->Accept(m_listenSocket)) {
    PTRACE(3, "Incoming connection from " << socket->GetPeerHostName());
    Context * context = CreateContext();
    if (context != NULL && context->Open(socket, true)) {
      if (m_singleThreadForAll)
        context->OnStart();
      else
        context->Start();
      AddContext(context);
      return true;
    }
  }

  PTRACE(2, "Error accepting connection: " << m_listenSocket.GetErrorText());
  delete socket;
  return false;
}


PTCPSocket * PCLISocket::CreateSocket()
{
  return new PTCPSocket();
}


///////////////////////////////////////////////////////////////////////////////

#if P_TELNET

PCLITelnet::PCLITelnet(WORD port, const char * prompt, bool singleThreadForAll)
  : PCLISocket(port, prompt, singleThreadForAll)
{
}


PTCPSocket * PCLITelnet::CreateSocket()
{
  return new PTelnetSocket();
}

#endif // P_TELNET

///////////////////////////////////////////////////////////////////////////////

#if P_CURSES==1

class PCLICursesWindow : public PCLICurses::Window
{
protected:
  WINDOW * m_outer;
  WINDOW * m_inner;

public:
  PCLICursesWindow(PCLICurses & owner, unsigned row, unsigned col, unsigned rows, unsigned cols, PCLICurses::Borders border)
    : PCLICurses::Window(owner, border)
    , m_outer(NULL)
    , m_inner(NULL)
  {
    PTRACE(5, "Constructed window " << this);
    SetPositionAndSize(rows, cols, row, col, border);
    Clear();
  }


  virtual bool FillChar(unsigned row, unsigned col, char ch, unsigned count)
  {
    while (count-- > 0) {
      if (mvwaddch(m_inner, row, col++, ch) == ERR) {
        PTRACE(2, "Write failed: errno=" << errno);
        return false;
      }
    }

    return true;
  }


  void SetPositionAndSize(unsigned rows, unsigned cols, unsigned row, unsigned col, PCLICurses::Borders border)
  {
    unsigned maxRows, maxCols;
    m_owner.GetScreenSize(maxRows, maxCols);

    if (row >= maxRows)
      row = maxRows-1;
    if (col >= maxCols)
      col = maxCols-1;

    if (row + rows > maxRows)
      rows = maxRows - row;
    if (col + cols > maxCols)
      cols = maxCols - col;

    if (m_outer != NULL)
      delwin(m_outer);
    if (m_inner != NULL)
      delwin(m_inner);

    PTRACE(4, "New window " << this << ": row=" << row << " col=" << col << " rows=" << rows << " cols=" << cols);

    m_outer = newwin(rows, cols, row, col);
    switch (m_border) {
      case PCLICurses::FullBorder :
        box(m_outer, 0, 0);
        ++row;
        ++col;
        rows -= 2;
        cols -= 2;
        break;

      case PCLICurses::BorderAbove :
        mvwhline(m_outer, 0, 0, 0, cols);
        ++row;
        --rows;
        break;

      case PCLICurses::BorderBelow :
        mvwhline(m_outer, rows-1, 0, 0, cols);
        --rows;
        break;

      case PCLICurses::BorderLeft :
        mvwvline(m_outer, 0, 0, 0, rows);
        ++col;
        --cols;
        break;

      case PCLICurses::BorderRight :
        mvwvline(m_outer, 0, cols-1, 0, rows);
        --cols;
        break;

      default :
        break;
    }
    wrefresh(m_outer);

    m_inner = newwin(rows, cols, row, col);
  }


  virtual void SetPosition(unsigned row, unsigned col)
  {
    unsigned rows, cols;
    getmaxyx(m_outer, rows, cols);
    SetPositionAndSize(rows, cols, row, col, m_border);
}


  virtual void GetPosition(unsigned & row, unsigned & col)
  {
    getbegyx(m_outer, row, col);
  }


  virtual void SetSize(unsigned rows, unsigned cols, PCLICurses::Borders border)
  {
    unsigned row, col;
    getbegyx(m_outer, row, col);
    SetPositionAndSize(rows, cols, row, col, border);
  }


  virtual void GetSize(unsigned & rows, unsigned & cols, bool includeBorder)
  {
    getmaxyx(includeBorder ? m_outer : m_inner, rows, cols);
  }


  virtual void SetCursor(unsigned row, unsigned col)
  {
    wmove(m_inner, row, col);
  }


  virtual void GetCursor(unsigned & row, unsigned & col)
  {
    getyx(m_inner, row, col);
  }


  void Refresh()
  {
    PTRACE(5, "Refresh for window " << this);
    flush();
    wrefresh(m_outer);
    wrefresh(m_inner);
  }


  virtual void Clear()
  {
    werase(m_inner);
  }


  virtual void Scroll(int n)
  {
    scrollok(m_inner, TRUE);
    wscrl(m_inner, n);
    scrollok(m_inner, FALSE);
  }
};


///////////////////////////////////////

PCLICurses::PCLICurses()
{
  if (initscr() == NULL) { // Initialise curses
    PTRACE(2, "Cannot initialise curses");
    return;
  }

  nonl();               // Don't automaticall wrap at end of line
  keypad(stdscr, TRUE); // Enable special keys (arrows, keypad etc)
  refresh();            // the wrefresh() for a window is not enough, must do this first. Weird.

  getmaxyx(stdscr, m_maxRows, m_maxCols);
  if (m_maxRows != 0 && m_maxCols != 0)
    Construct();
}


PCLICurses::~PCLICurses()
{
  m_windows.RemoveAll();
  endwin();
  PTRACE(5, "Destroyed curses");
}


///////////////////////////////////////

#elif P_CURSES==2

class PCLICursesWindow : public PCLICurses::Window
{
protected:
  HANDLE   m_hStdOut;
  unsigned m_positionRow;
  unsigned m_positionCol;
  unsigned m_sizeRows;
  unsigned m_sizeCols;
  unsigned m_cursorRow;
  unsigned m_cursorCol;

  CONSOLE_SCREEN_BUFFER_INFO m_screenBufferInfo;

public:
  PCLICursesWindow(PCLICurses & owner, unsigned row, unsigned col, unsigned rows, unsigned cols, PCLICurses::Borders border)
    : PCLICurses::Window(owner, border)
    , m_hStdOut(GetStdHandle(STD_OUTPUT_HANDLE))
    , m_positionRow(row)
    , m_positionCol(col)
    , m_sizeRows(rows)
    , m_sizeCols(cols)
    , m_cursorRow(0)
    , m_cursorCol(0)
  {
    if (!GetConsoleScreenBufferInfo(m_hStdOut, &m_screenBufferInfo)) {
      PTRACE(2, "Cannot obtain screen buffer info");
    }

    DrawBorder(false);
    Clear();
    PTRACE(5, "New window " << this << ": row=" << row << " col=" << col << " rows=" << rows << " cols=" << cols);
  }


  virtual void SetPosition(unsigned row, unsigned col)
  {
    unsigned maxRows, maxCols;
    m_owner.GetScreenSize(maxRows, maxCols);

    m_positionRow = std::min(row, maxRows-1);
    m_positionCol = std::min(col, maxCols-1);

    if (m_positionRow+m_sizeRows > maxRows)
      m_sizeRows = maxRows - m_positionRow;
    if (m_positionCol+m_sizeCols > maxCols)
      m_sizeCols = maxCols - m_positionCol;
  }


  virtual void GetPosition(unsigned & row, unsigned & col)
  {
    row = m_positionRow;
    col = m_positionCol;
  }


  virtual void SetSize(unsigned rows, unsigned cols, PCLICurses::Borders border)
  {
    unsigned maxRows, maxCols;
    m_owner.GetScreenSize(maxRows, maxCols);

    maxRows -= m_positionRow;
    maxCols -= m_positionCol;

    m_sizeRows = std::min(rows, maxRows);
    m_sizeCols = std::min(cols, maxCols);

    if (border != PCLICurses::NumBorders) {
      DrawBorder(true);
      m_border = border;
      DrawBorder(false);
    }
  }


  virtual void GetSize(unsigned & rows, unsigned & cols, bool includeBorder)
  {
    rows = m_sizeRows;
    cols = m_sizeCols;

    if (includeBorder)
      return;

    switch (m_border) {
      case PCLICurses::FullBorder :
        cols -= 2;
        rows -= 2;
        break;

      case PCLICurses::BorderAbove :
      case PCLICurses::BorderBelow :
        --rows;
        break;

      case PCLICurses::BorderLeft :
      case PCLICurses::BorderRight :
        --cols;
        break;

      default :
        break;
    }
  }


  COORD GetAbsoluteCoord(int row, int col)
  {
    COORD pos = { (SHORT)(m_positionCol+col), (SHORT)(m_positionRow+row) };
    switch (m_border) {
      case PCLICurses::FullBorder :
        ++pos.X;
        ++pos.Y;
        break;
      case PCLICurses::BorderAbove :
        ++pos.Y;
        break;
      case PCLICurses::BorderLeft :
        ++pos.X;
        break;
      default :
        break;
    }
    return pos;
  }

  virtual void SetCursor(unsigned row, unsigned col)
  {
    m_cursorRow = std::min(row, m_sizeRows);
    m_cursorCol = std::min(col, m_sizeCols);
    if (m_focus && !SetConsoleCursorPosition(m_hStdOut, GetAbsoluteCoord(m_cursorRow, m_cursorCol))) {
      PTRACE(2, "Cannot set console cursor position");
    }
  }


  virtual void GetCursor(unsigned & row, unsigned & col)
  {
    row = m_cursorRow;
    col = m_cursorCol;
  }


  virtual bool FillChar(unsigned row, unsigned col, char ch, unsigned count)
  {
    return InternalFillChar(GetAbsoluteCoord(row, col), ch, count);
  }


  bool InternalFillChar(COORD pos, char ch, unsigned count)
  {
    DWORD cCharsWritten;
    if (!FillConsoleOutputAttribute(m_hStdOut, m_screenBufferInfo.wAttributes, count, pos, &cCharsWritten )) {
      PTRACE(2, "Cannot clear console attributes");
      return false;
    }

    if (!FillConsoleOutputCharacter(m_hStdOut, ch, count, pos, &cCharsWritten)) {
      PTRACE(2, "Cannot clear console");
      return false;
    }

    return true;
  }


  void Refresh()
  {
    flush();
  }


  virtual void Clear()
  {
    unsigned rows, cols;
    GetSize(rows, cols, false);

    for (unsigned row = 0; row < rows; ++row)
      FillChar(row, 0, ' ', cols);

    m_cursorRow = m_cursorCol = 0;
  }


  virtual void Scroll(int n)
  {
    unsigned rows, cols;
    GetSize(rows, cols, false);

    COORD to = GetAbsoluteCoord(0, 0);
    SMALL_RECT rect = { to.X, to.Y, (SHORT)(to.X + cols - 1), (SHORT)(to.Y + rows - 1) };
    to.Y -= (SHORT)n;
    CHAR_INFO fill = { ' ', m_screenBufferInfo.wAttributes };
    if (ScrollConsoleScreenBuffer(m_hStdOut, &rect, &rect, to, &fill))
      return;

    PTRACE(2, "Cannot scroll console buffer");
  }


  virtual void DrawBorder(bool erase)
  {
    static const char TopLeftCorner     = '\xda';
    static const char TopRightCorner    = '\xbf';
    static const char BottomLeftCorner  = '\xc0';
    static const char BottomRightCorner = '\xd9';
    static const char HorizontalLine    = '\xc4';
    static const char VerticalLine      = '\xb3';

    COORD pos = { (SHORT)m_positionCol, (SHORT)m_positionRow };
    switch (m_border) {
      case PCLICurses::FullBorder :
        InternalFillChar(pos, erase ? ' ' : TopLeftCorner, 1);
        ++pos.X;
        InternalFillChar(pos, erase ? ' ' : HorizontalLine, m_sizeCols-2);
        pos.X += (SHORT)(m_sizeCols-2);
        InternalFillChar(pos, erase ? ' ' : TopRightCorner, 1);
        for (unsigned row = 1; row < m_sizeRows-1; ++row, ++pos.Y) {
          pos.X -= (SHORT)(m_sizeCols-1);
          InternalFillChar(pos, erase ? ' ' : VerticalLine, 1);
          pos.X += (SHORT)(m_sizeCols-1);
          InternalFillChar(pos, erase ? ' ' : VerticalLine, 1);
        }
        InternalFillChar(pos, erase ? ' ' : BottomLeftCorner, 1);
        ++pos.X;
        InternalFillChar(pos, erase ? ' ' : HorizontalLine, m_sizeCols-2);
        pos.X += (SHORT)(m_sizeCols-2);
        InternalFillChar(pos, erase ? ' ' : BottomRightCorner, 1);
        break;

      case PCLICurses::BorderAbove :
        InternalFillChar(pos, erase ? ' ' : HorizontalLine, m_sizeCols);
        break;

      case PCLICurses::BorderBelow :
        pos.Y += (SHORT)(m_sizeRows-1);
        InternalFillChar(pos, erase ? ' ' : HorizontalLine, m_sizeCols);
        break;

      case PCLICurses::BorderLeft :
        for (unsigned row = 0; row < m_sizeRows; ++row, ++pos.Y)
          InternalFillChar(pos, erase ? ' ' : VerticalLine, 1);
        break;

      case PCLICurses::BorderRight :
        pos.X += (SHORT)(m_sizeCols-1);
        for (unsigned row = 0; row < m_sizeRows; ++row, ++pos.Y)
          InternalFillChar(pos, erase ? ' ' : VerticalLine, 1);
        break;

      default :
        break;
    }
  }
};


///////////////////////////////////////

PCLICurses::PCLICurses()
{
  HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

  CONSOLE_SCREEN_BUFFER_INFO csbi; 
  if (!GetConsoleScreenBufferInfo(hStdOut, &csbi)) {
    PTRACE(2, "Cannot obtain console screen buffer");
    m_maxRows = m_maxCols = 0;
    return;
  }

  m_maxRows = (csbi.srWindow.Bottom - csbi.srWindow.Top)+1;
  m_maxCols = (csbi.srWindow.Right - csbi.srWindow.Left)+1;

  COORD size = { (SHORT)m_maxCols, (SHORT)m_maxRows };
  SetConsoleScreenBufferSize(hStdOut, size);

  // Turn off most processing.
  SetConsoleMode(hStdOut, 0);
  SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), ENABLE_MOUSE_INPUT | ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS);

  Construct();
}


PCLICurses::~PCLICurses()
{
  Stop();
  m_windows.RemoveAll();
  PTRACE(5, "Destroyed curses");
}


#endif // P_CURSES==2

///////////////////////////////////////

#if P_CURSES

void PCLICurses::Construct()
{
  m_editCodes.SetAt(m_leftCodes.GetSize(), PConsoleChannel::KeyBackSpace);
  m_leftCodes.SetAt(m_leftCodes.GetSize(), PConsoleChannel::KeyLeft);
  m_rightCodes.SetAt(m_leftCodes.GetSize(), PConsoleChannel::KeyRight);
  m_beginCodes.SetAt(m_leftCodes.GetSize(), PConsoleChannel::KeyHome);
  m_endCodes.SetAt(m_leftCodes.GetSize(), PConsoleChannel::KeyEnd);
  m_prevCmdCodes.SetAt(m_leftCodes.GetSize(), PConsoleChannel::KeyUp);
  m_nextCmdCodes.SetAt(m_leftCodes.GetSize(), PConsoleChannel::KeyDown);

  m_requireEcho = true;

  m_pageWaitPrompt = "Press a key for more ...";

  PConsoleChannel * input = new PConsoleChannel(PConsoleChannel::StandardInput);
  input->SetLocalEcho(false); // We do all this
  input->SetLineBuffered(false);

  NewWindow(0, 0, m_maxRows-2, m_maxCols, NoBorder).SetPageMode(true);
  NewWindow(m_maxRows-2, 0, 2, m_maxCols, BorderAbove).SetFocus();

  StartContext(input, &m_windows[0], true, false, false);

  PTRACE(4, "Constructed curses: maxRows=" << m_maxRows << " maxCols=" << m_maxCols);
}


class PCLICursesContext : public PCLI::Context
{
  PCLICurses & m_cli;
public:
  PCLICursesContext(PCLICurses & cli)
    : PCLI::Context(cli)
    , m_cli(cli)
  {
  }

  virtual bool WritePrompt()
  {
    PTRACE(4, "Writing prompt");

    PCLICurses::Window & wnd = m_cli[m_cli.GetWindowCount() > 1 ? 1 : 0];
    wnd.Clear();
    if (!wnd.WriteString(m_cli.GetPrompt()))
      return false;

    m_cli[0].SetPageMode();
    m_cli.Refresh();
    return true;
  }


  virtual bool EchoInput(char ch)
  {
    return m_cli[m_cli.GetWindowCount() > 1 ? 1 : 0].WriteChar(ch);
  }
};


PCLI::Context * PCLICurses::StartForeground()
{
  return m_contextList.front();
}


PCLI::Context * PCLICurses::CreateContext()
{
  return m_contextList.empty() ? new PCLICursesContext(*this) : NULL;
}


PCLICurses::Window & PCLICurses::NewWindow(unsigned row, unsigned col, unsigned rows, unsigned cols, Borders border)
{
  Window * wnd = new PCLICursesWindow(*this, row, col, rows, cols, border);
  m_windows.Append(wnd);
  return *wnd;
}


void PCLICurses::RemoveWindow(Window & wnd)
{
  if (&wnd != &m_windows[0])
    m_windows.Remove(&wnd);
}


void PCLICurses::RemoveWindow(PINDEX idx)
{
  if (idx > 0)
    m_windows.RemoveAt(0);
}


PCLICurses::Window * PCLICurses::GetFocusWindow() const
{
  for (PINDEX i = 0; i < m_windows.GetSize(); ++i) {
    if (m_windows[i].HasFocus())
      return &m_windows[i];
  }

  return NULL;
}


bool PCLICurses::WaitPage()
{
  PCLICurses::Window & wnd = m_windows[m_windows.GetSize() > 1 ? 1 : 0];
  wnd.Clear();
  if (!wnd.WriteString(GetPageWaitPrompt()))
    return false;

  m_contextList.front()->ReadChar();
  wnd.Clear();
  return true;
}


void PCLICurses::Refresh()
{
  for (PINDEX i = 0; i < m_windows.GetSize(); ++i)
    m_windows[i].Refresh();
}


///////////////////////////////////////

PCLICurses::Window::Window(PCLICurses & owner, PCLICurses::Borders border)
  : m_owner(owner)
  , m_border(border)
  , m_focus(false)
  , m_pageMode(false)
  , m_pagedRows(0)
{
}


PBoolean PCLICurses::Window::Write(const void * data, PINDEX length)
{
  lastWriteCount = 0;

  unsigned row, col;
  GetCursor(row, col);

  unsigned rows, cols;
  GetSize(rows, cols, false);

  const char * ptr = (const char *)data;
  while (length-- > 0) {
    switch (*ptr) {
      case '\x7f' :
        if (col > 0 && !FillChar(row, --col, ' ', 1))
          return false;
        break;

      case '\b' :
        if (col > 0)
          --col;
        break;

      case '\r' :
        col = 0;
        break;

      default :
        if (!FillChar(row, col, *ptr, 1))
          return false;
        if (++col < cols)
          break;
        // Do new line case as wrapped at end of line

      case '\n' :
        col = 0;
        ++row;

        if (m_pageMode && ++m_pagedRows >= rows) {
          m_pagedRows = 0;
          Refresh();
          if (!m_owner.WaitPage())
            return false;
        }
        break;
    }

    if (row >= rows) {
      Scroll();
      --row;
    }

    ++lastWriteCount;
    ++ptr;
  }

  SetCursor(row, col);

  Refresh();
  return true;
}


void PCLICurses::Window::SetPageMode(bool on)
{
  m_pageMode = on;
  m_pagedRows = 0;
}


void PCLICurses::Window::SetFocus()
{
  PTRACE(4, "SetFocus " << this);
  for (PINDEX i = 0; i < m_owner.GetWindowCount(); ++i) {
    Window * wnd = m_owner.GetWindow(i);
    wnd->m_focus = wnd == this;
  }
}


#endif // P_CURSES

#endif // P_CLI

///////////////////////////////////////////////////////////////////////////////
