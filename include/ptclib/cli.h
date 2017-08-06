/*
 * cli.h
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
 */

#ifndef PTLIB_CLI_H
#define PTLIB_CLI_H

#include <ptlib.h>

#if P_CLI

#include <ptlib/sockets.h>
#include <ptclib/vartype.h>

#include <list>


/** Command Line Interpreter class.
    This class contains a set of commands, which are executed via a PNotifier,
    when entered via a PChannel.

    The system supports multiple simultaneous interpreter which may access the
    same command set. For example several telnet sessions.

    Note that the various command interpreters could be operating in different
    threads, so care should be taken for sybchronisation issues on the object
    being acted upon via the PNotifiers.
  */
class PCLI : public PObject
{
    PCLASSINFO(PCLI, PObject);
  public:
    class Context;

    /**Context for command line interpreter.
      */
    class Context : public PIndirectChannel
    {
      public:
      /**@name Construction */
      //@{
        /**Construct new command line interpreter context.
          */
        Context(
          PCLI & cli
        );

        /**Destroy command line interpreter context.
           This will close and wait for threads to exit.
          */
        virtual ~Context();
      //@}

      /**@name Overrides from PChannel */
      //@{
        /**Low level write to the channel. This function will block until the
           requested number of characters are written or the write timeout is
           reached. The GetLastWriteCount() function returns the actual number
           of bytes written.

           This translate new line character ('\n') to be the text string in
           PCLI::m_newLine.

           The GetErrorCode() function should be consulted after Write() returns
           false to determine what caused the failure.

           @return
           true if at least len bytes were written to the channel.
         */
        virtual PBoolean Write(
          const void * buf, ///< Pointer to a block of memory to write.
          PINDEX len        ///< Number of bytes to write.
        );
      //@}

      /**@name Operations */
      //@{
        /**Start a command line interpreter thread.
          */
        bool Start();

        /**Stop command line interpreter context.
           This will close the channel and wait for threads to exit.
          */
        void Stop();

        /**Run command line interpreter context.
           This will execute OnStart() then ReadAndProcessInput() until it
           returns false, then OnStop().
          */
        bool Run();

        /**Call back frunction for when context is started.
           This is usually called from within a background thread.

           The default behaviour displays the prompt.
          */
        virtual void OnStart();

        /**Callback for when context is stopping.
           This is usually called from within a background thread.

           The default behaviour does nothing.
          */
        virtual void OnStop();

        /**Write prompt (depending on state) to channel.
          */
        virtual bool WritePrompt();

        /**Read a character from the attached channel an process.
           If the character was successfully read then ProcessInput() is called.
          */
        virtual bool ReadAndProcessInput();

        /**Process a character read from the channel.
           Returns false if have error and processing is to cease.
          */
        virtual bool ProcessInput(int ch);

        /**Echo character from input.
           This is only called if m_cli.GetRequireEcho() returns true.
           It should also handle a DEL ('\x7f') character as erase character
           to the left of the cursor. Often this is "\b \b".
          */
        virtual bool EchoInput(char ch);

        /**Call back for a command line was completed and ENTER pressed.
           The default behaviour processes the line into a PArgList and deals
           with the command history and help.

           Control is then passed on to PCLI::OnReceivedLine().
          */
        virtual void OnCompletedLine();

        /** Get the terminal size from the operating system.
          */
        virtual bool GetTerminalSize(
          unsigned & rows,
          unsigned & columns
        );

        /// Handle Broadcast
        virtual void Broadcast(
          const PString & message  ///< Message to broadcase
        );
      //@}

      /**@name Member access */
      //@{
        /**Get the CLI.
          */
        PCLI & GetCLI() const { return m_cli; }

        /**Indicate is currently processing a command.
          */
        bool IsProcessingCommand() const { return m_state == e_ProcessingCommand; }
      //@}

      protected:
        PDECLARE_NOTIFIER(PThread, Context, ThreadMain);
        bool InternalMoveCursor(bool left, PINDEX count);
        bool InternalEchoCommandLine(PINDEX echoPosition, PINDEX moveLeftCount);
        bool InternalMoveHistoryCommand(int direction);
        bool InternalWrite(const char * str, PINDEX len, PINDEX & written);

        PCLI      & m_cli;
        PString     m_commandLine;
        PINDEX      m_editPosition;
        bool        m_ignoreNextEOL;
        PStringList m_commandHistory;
        PINDEX      m_historyPosition;
        PThread   * m_thread;
        int         m_pagedLines;

        enum State {
          e_Username,
          e_Password,
          e_CommandEntry,
          e_ProcessingCommand
        } m_state;
        PString m_enteredUsername;
    };

    /**This class is an enhancement to PArgList to add context.
      */
    class Arguments : public PArgList
    {
      public:
      /**@name Construction */
      //@{
        Arguments(
          Context & context,
          const PString & rawLine
        );
      //@}

      /**@name Operations */
      //@{
        /**Write to the CLI output channel the usage for the current command.
          */
        Context & WriteUsage();

        /**Write an error to the CLI output channel.
          */
        Context & WriteError(
          const PString & error = PString::Empty()  /// Error message
        );
      //@}

      /**@name Member access */
      //@{
        /**Get the CLI context supplying the command line arguments.
          */
        Context & GetContext() const { return m_context; }
      //@}

      protected:
        Context & m_context;
        PString   m_usage;

      friend class PCLI;
    };


  /**@name Construction */
  //@{
    /** Contracut a new command line interpreter.
      */
    PCLI(
      const char * prompt = NULL
    );

    /**Destroy the command line interpreter. This will call Stop() to assure
       everything is cleaned up before exiting.
      */
    virtual ~PCLI();
  //@}

  /**@name Operations */
  //@{
    /**Start a command line interpreter.
       If runInBackground is true the all the command line interpreter contexts
       that have been added will have their background threads started.

       If runInBackground is false, then there must only be one context added
       and that context is continuously read until it is stopped, it's channel
       is closed or returns end of file.
      */
    virtual bool Start(
      bool runInBackground = true   ///< Spawn a thread to read and interpret commands
    );

    /**Stop and clean up command line interpreters.
       All the running contexts threads will be stopped, closing the channels
       and memory cleaned up.
      */
    virtual void Stop();

    /**Start a command line interpreter context.
       If \p runInBackground is true then the context is immediately started in
       a thread. If false then the context is set up and will await a call to
       the Start() function.
      */
    Context * StartContext(
      PChannel * channel,           ///< Channel to read/write
      bool autoDelete = true,       ///< Automatically delete channel on exit
      bool runInBackground = true   ///< Spawn a thread to read and interpret commands
    ) { return StartContext(channel, channel, autoDelete, autoDelete, runInBackground); }
    Context * StartContext(
      PChannel * readChannel,      ///< Channel to be used for both read operations.
      PChannel * writeChannel,     ///< Channel to be used for both write operations.
      bool autoDeleteRead = true,  ///< Automatically delete the read channel
      bool autoDeleteWrite = true, ///< Automatically delete the write channel
      bool runInBackground = true   ///< Spawn a thread to read and interpret commands
    );

    /** Start default foreground context.
        Default action returns NULL.
      */
    virtual Context * StartForeground();

    /**Run a command line interpreter context.
       This will run the interpreter synchonously, continuously reading from
       the channel supplied until the context is stopped or the channel is
       closed or returns end of file.
      */
    bool Run(
      PChannel & channel            ///< Channel to read/write
    ) { return Run(&channel, &channel, false, false); }
    bool Run(
      PChannel * channel,           ///< Channel to read/write
      bool autoDelete = true        ///< Automatically delete channel on exit
    ) { return Run(channel, channel, autoDelete, autoDelete); }
    bool Run(
      PChannel * readChannel,      ///< Channel to be used for both read operations.
      PChannel * writeChannel,     ///< Channel to be used for both write operations.
      bool autoDeleteRead = true,  ///< Automatically delete the read channel
      bool autoDeleteWrite = true  ///< Automatically delete the write channel
    );

    /** Run a context.
       This will run the interpreter synchonously, continuously reading from
       the channel supplied until the context is stopped or the channel is
       closed or returns end of file.
    */
    bool RunContext(
      Context * context
    ) { return context != NULL && context->Run(); }

    /**Create a new context.
       Users may use this to create derived classes for their own use.
      */
    virtual Context * CreateContext();

    /**Add a command line interpreter context to the system.
       If context is NULL then CreateContext() is called to create one.
      */
    virtual Context * AddContext(
      Context * context = NULL    ///< New context to add to the system.
    );

    /**Remove the command line interpreter context.
       The context thread is stopped, the channel closed and memory deleted.
      */
    virtual void RemoveContext(
      Context * context   ///< Context to remove
    );

    /**Remove any closed command line interpreter contexts.
      */
    virtual void GarbageCollection();

    /**Received a completed command line.
       The completed command line is parsed into arguments by the PArgList
       class, and passed to this function.

       The default behaviour searches the list of registered commands for a
       PNotifier to execute.
      */
    virtual void OnReceivedLine(
      Arguments & line
    );

    /**Received a login name/pasword to be verified.
       Note that the m_username or m_password field must be non-empty for a
       log in sequence to occur, even if this function is overridden and the
       memnbers not actually used for validation.

       If the m_username is an empty string, but m_password is not, then it
       the username not prompted for, just the password is required. The
       reverse is also poassible and only a username entry required.

       Default returns true if parameters are equal to m_username, m_password
       respectively.
      */
    virtual bool OnLogIn(
      const PString & username,
      const PString & password
    );

    /**Set a string to all command line interpreter contexts.
      */
    void Broadcast(
      const PString & message   ///< Message to broadcast.
    ) const;

    /**Register a new command to be interpreted.
       Note the command may be a series of synonyms of the same command
       separated by the '\n' character.

       The command may also contain spaces which separates sub-commands, e.g.
       "list users".

       Returns false if one of the command synonyms was a dupicate of an
               existing command.
      */
    bool SetCommand(
      const char * command,       ///< Command(s) to register
      const PNotifier & notifier, ///< Callback to execute when command interpreted
      const char * help = NULL,   ///< Help text on command (what it does)
      const char * usage = NULL,  ///< Usage text on command (syntax/options)
      const char * argSpec = NULL ///< Argument specification to pass to PArgList
    );

    /**Register a command setting a boolean variable.
    */
    bool SetCommand(
      const char * command,       ///< Command(s) to register
      bool & value,               ///< Boolean value to change
      const char * name,          ///< Name of variable
      const char * help = NULL,   ///< Help text on command (what it does)
      const PNotifier & notifier = PNotifier() ///< Callback to execute when value changed
    );

    /**Register a command setting an integer variable.
    */
    bool SetCommand(
      const char * command,       ///< Command(s) to register
      const PVarType & value,     ///< Integer value to change
      const char * name,          ///< Name of variable
      const PVarType & minValue,  ///< Minimum value for integer
      const PVarType & maxValue,  ///< Maximum value for integer
      const char * help = NULL,   ///< Help text on command (what it does)
      const PNotifier & notifier = PNotifier() ///< Callback to execute when value changed
    );

    template <typename TYPE>
    bool SetCommand(
      const char * command,        ///< Command(s) to register
      TYPE & value,                ///< Integer value to change
      const char * name,           ///< Name of variable
      TYPE minValue,               ///< Minimum value for integer
      TYPE maxValue = INT_MAX,     ///< Maximum value for integer
      const char * help = NULL,    ///< Help text on command (what it does)
      const PNotifier & notifier = PNotifier() ///< Callback to execute when value changed
    )
    {
      return SetCommand(command, PRefVar<TYPE>(value), name, minValue, maxValue, help, notifier);
    }

    /**Show help for registered commands to the context.
      */
    virtual void ShowHelp(
      Context & context,        ///< Context to output help to.
      const PArgList & partial  ///< Partial command line to limit help
    );
    //@}

  /**@name Member access */
  //@{
    /**Get new line string output at the end of every line.
       Default is "\n".
      */
    const PString & GetNewLine() const { return m_newLine; }

    /**Set new line string output at the end of every line.
       Default is "\n".
      */
    void SetNewLine(const PString & newLine);

    /**Get flag for echo is required for entered characters.
       Default is false.
      */
    bool GetRequireEcho() const { return m_requireEcho; }

    /**Set flag for echo is required for entered characters.
       Default is false.
      */
    void SetRequireEcho(bool requireEcho) { m_requireEcho = requireEcho; }

    /**Get codes used for editing (backspace/delete) command line.
       Default is {'\b','\x7f'}.
      */
    const PIntArray & GetEditCodes() const { return m_editCodes; }

    /**Set codes used for editing (backspace/delete) command line.
       Default is {'\b','\x7f'}.
      */
    void SetEditCodes(const PIntArray & editCodes) { m_editCodes = editCodes; }

    /**Get codes used for erasing whole command line.
       Default is ^U {'\x15'}.
      */
    const PIntArray & GetEraseCodes() const { return m_eraseCodes; }

    /**Set codes used for erasing whole command line.
       Default is ^U {'\x15'}.
      */
    void SetEraseCodes(const PIntArray & eraseCodes) { m_eraseCodes = eraseCodes; }

    /**Get codes used for moving left in command line.
       Default is ^L {'\x0c'}.
      */
    const PIntArray & GetLeftCodes() const { return m_leftCodes; }

    /**Set codes used for moving left in command line.
       Default is ^L {'\x0c'}.
      */
    void SetLeftCodes(const PIntArray & leftCodes) { m_leftCodes = leftCodes; }

    /**Get codes used for moving right in command line.
       Default is ^R {'\x12'}.
      */
    const PIntArray & GetRightCodes() const { return m_rightCodes; }

    /**Set codes used for moving right in command line.
       Default is ^R {'\x12'}.
      */
    void SetRightCodes(const PIntArray & rightCodes) { m_rightCodes = rightCodes; }

    /**Get codes used for moving to beginning of command line.
       Default is ^B.
      */
    const PIntArray & GetBeginCodes() const { return m_beginCodes; }

    /**Set codes used for moving to beginning of command line.
       Default is ^B.
      */
    void SetBeginCodes(const PIntArray & beginCodes) { m_beginCodes = beginCodes; }

    /**Get codes used for moving to end of command line.
       Default is ^E.
      */
    const PIntArray & GetEndCodes() const { return m_endCodes; }

    /**Set codes used for moving to end of command line.
       Default is ^E.
      */
    void SetEndCodes(const PIntArray & endCodes) { m_endCodes = endCodes; }

    /**Get codes used for getting previous command in history.
       Default is ^P.
      */
    const PIntArray & GetPrevCmdCodes() const { return m_prevCmdCodes; }

    /**Set codes used for setting previous command in history.
       Default is ^P.
      */
    void SetPrevCmdCodes(const PIntArray & prevCmdCodes) { m_prevCmdCodes = prevCmdCodes; }

    /**Get codes used for getting next command in history.
       Default is ^N.
      */
    const PIntArray & GetNextCmdCodes() const { return m_nextCmdCodes; }

    /**Set codes used for setting next command in history.
       Default is ^N.
      */
    void SetNextCmdCodes(const PIntArray & nextCmdCodes) { m_nextCmdCodes = nextCmdCodes; }

    /**Set codes used for auto-fill of keywords in command line.
       Default is tab {'\t'}.
      */
    void SetAutoFillCodes(const PIntArray & autoFillCodes) { m_autoFillCodes = autoFillCodes; }

    /**Get codes used for auto-fill of keywords in command line.
       Default is tab {'\t'}.
      */
    const PIntArray & GetAutoFillCodes() const { return m_autoFillCodes; }

    /**Get prompt used for command line interpreter.
       Default is "CLI> ".
      */
    const PString & GetPrompt() const { return m_prompt; }

    /**Set prompt used for command line interpreter.
       Default is "CLI> ".
      */
    void SetPrompt(const PString & prompt) { m_prompt = prompt; }

    /**Get prompt used for login (if enabled).
       Default is "Username: ".
      */
    const PString & GetUsernamePrompt() const { return m_usernamePrompt; }

    /**Set prompt used for login (if enabled).
       Default is "Username: ".
      */
    void SetUsernamePrompt(const PString & prompt) { m_usernamePrompt = prompt; }

    /**Get prompt used for password (if enabled).
       Default is "Password: ".
      */
    const PString & GetPasswordPrompt() const { return m_passwordPrompt; }

    /**Set prompt used for password (if enabled).
       Default is "Password: ".
      */
    void SetPasswordPrompt(const PString & prompt) { m_passwordPrompt = prompt; }

    /**Get username for log in validation.
       Default is empty string, disabling entry of username/password.
      */
    const PString & GetUsername() const { return m_username; }

    /**Set username for log in validation.
       Default is empty string, disabling entry of username/password.
      */
    void SetUsername(const PString & username) { m_username = username; }

    /**Get password for log in validation.
       Default is empty string, disabling entry of password.
      */
    const PString & GetPassword() const { return m_password; }

    /**Set password for log in validation.
       Default is empty string, disabling entry of password.
      */
    void SetPassword(const PString & password) { m_password = password; }

    /**Get command to be used for comment lines.
       Default is "#\n;\n//".
      */
    const PCaselessString & GetCommentCommand() const { return m_commentCommand; }

    /**Set command to be used for comment lines.
       Default is "#\n;\n//".
      */
    void SetCommentCommand(const PCaselessString & commentCommand) { m_commentCommand = commentCommand; }

    /**Get command to be used to exit session.
       Default is "exit\nquit".
      */
    const PCaselessString & GetExitCommand() const { return m_exitCommand; }

    /**Set command to be used to exit session.
       Default is "exit\nquit".
      */
    void SetExitCommand(const PCaselessString & exitCommand) { m_exitCommand = exitCommand; }

    /**Get command to be used to display help.
       Default is "?\nhelp".
      */
    const PCaselessString & GetHelpCommand() const { return m_helpCommand; }

    /**Set command to be used to display help.
       Default is "?\nhelp".
      */
    void SetHelpCommand(const PCaselessString & helpCommand) { m_helpCommand = helpCommand; }

    /**Get help on help.
       This string is output before the individual help is output for each command.
       Default describes ?, !, !n, !! followed by "Command available are:".
      */
    const PString & GetHelpOnHelp() const { return m_helpOnHelp; }

    /**Set help on help.
       This string is output before the individual help is output for each command.
       Default describes ?, !, !n, !! followed by "Command available are:".
      */
    void SetHelpOnHelp(const PCaselessString & helpOnHelp) { m_helpOnHelp = helpOnHelp; }

    /**Get the command to be used to repeat the last executed command.
       Default is "!!".
      */
    const PCaselessString & GetRepeatCommand() const { return m_repeatCommand; }

    /**Set the command to be used to repeat the last executed command.
       Default is "!!".
      */
    void SetRepeatCommand(const PCaselessString & repeatCommand) { m_repeatCommand = repeatCommand; }

    /**Get command that will list/execute command history.
       Default is "!".
      */
    const PCaselessString & GetHistoryCommand() const { return m_historyCommand; }

    /**Set command that will list/execute command history.
       Default is "!".
      */
    void SetHistoryCommand(const PCaselessString & historyCommand) { m_historyCommand = historyCommand; }

    /**Get error message for if there is no history.
       Default is "No command history".
      */
    const PString & GetNoHistoryError() const { return m_noHistoryError; }

    /**Set error message for if there is no history.
       Default is "No command history".
      */
    void SetNoHistoryError(const PString & noHistoryError) { m_noHistoryError = noHistoryError; }

    /**Get usage prefix for if Arguments::WriteUsage() called.
       Default is "Usage: ".
      */
    const PString & GetCommandUsagePrefix() const { return m_commandUsagePrefix; }

    /**Set usage prefix for if Arguments::WriteUsage() called.
       Default is "Usage: ".
      */
    void SetCommandUsagePrefix(const PString & commandUsagePrefix) { m_commandUsagePrefix = commandUsagePrefix; }

    /**Get error prefix for if Arguments::WriteError() called.
       Default is ": error: ", always prefixed by command name.
      */
    const PString & GetCommandErrorPrefix() const { return m_commandErrorPrefix; }

    /**Set error prefix for if Arguments::WriteError() called.
       Default is ": error: ", always prefixed by command name.
      */
    void SetCommandErrorPrefix(const PString & commandErrorPrefix) { m_commandErrorPrefix = commandErrorPrefix; }

    /**Get error message for if unknown command is entered.
       Default is "Unknown command".
      */
    const PString & GetUnknownCommandError() const { return m_unknownCommandError; }

    /**Set error message for if unknown command is entered.
       Default is "Unknown command".
      */
    void SetUnknownCommandError(const PString & unknownCommandError) { m_unknownCommandError = unknownCommandError; }

    /**Get error message for if ambiguous command is entered.
       Default is "Ambiguous command".
      */
    const PString & GetAmbiguousCommandError() const { return m_ambiguousCommandError; }

    /**Set error message for if ambiguous command is entered.
       Default is "Ambiguous command".
      */
    void SetAmbiguousCommandError(const PString & ambiguousCommandError) { m_ambiguousCommandError = ambiguousCommandError; }

    /**Get command that will read a script file.
       Default is "<\nread".
      */
    const PCaselessString & GetScriptCommand() const { return m_scriptCommand; }

    /**Set command that will read a script file.
       Default is "<\nread".
      */
    void SetScriptCommand(const PCaselessString & scriptCommand) { m_scriptCommand = scriptCommand; }

    /**Get error message for if script file doe not exist.
       Default is "Script file could not be found".
      */
    const PString & GetNoScriptError() const { return m_noScriptError; }

    /**Set error message for if script file doe not exist.
       Default is "Script file could not be found".
      */
    void SetNoScriptError(const PString & noScriptError) { m_noScriptError = noScriptError; }

    /**Get number of lines to emimt before pausing and prompting for input.
       Zero indicates disabled, -1 indicates the value is determined from the operating
       system, and if unavailable, disabled.
      */
    int GetPagerLines() const { return m_pagerLines; }

    /**Set number of lines to emimt before pausing and prompting for input.
       Zero indicates disabled, -1 indicates the value is determined from the operating
       system, and if unavailable, disabled.
      */
    void SetPagerLines(int lines) { m_pagerLines = lines; }

    /**Get prompt used for waiting on a page of display output.
       Default is "Press a key for more ...".
      */
    const PString & GetPageWaitPrompt() const { return m_pagerWaitPrompt; }

    /**Set prompt used for waiting on a page of display output.
       Default is "Press a key for more ...".
      */
    void SetPageWaitPrompt(const PString & prompt) { m_pagerWaitPrompt = prompt; }

    /**Get name of command to adjust pager lines.
       Default is "set page".
      */
    const PString & GetPagerCommand() const { return m_pagerCommand; }

    /**Set name of command to adjust pager lines.
       Default is "set page".
      */
    void SetPagerCommand(const PString & cmd) { m_pagerCommand = cmd; }
  //@}


  protected:
    PString         m_newLine;
    bool            m_requireEcho;
    PIntArray       m_editCodes;
    PIntArray       m_eraseCodes;
    PIntArray       m_leftCodes;
    PIntArray       m_rightCodes;
    PIntArray       m_beginCodes;
    PIntArray       m_endCodes;
    PIntArray       m_prevCmdCodes;
    PIntArray       m_nextCmdCodes;
    PIntArray       m_autoFillCodes;
    PString         m_prompt;
    PString         m_usernamePrompt;
    PString         m_passwordPrompt;
    PString         m_username;
    PString         m_password;
    PCaselessString m_commentCommand;
    PCaselessString m_exitCommand;
    PCaselessString m_helpCommand;
    PString         m_helpOnHelp;
    PCaselessString m_repeatCommand;
    PCaselessString m_historyCommand;
    PString         m_noHistoryError;
    PString         m_commandUsagePrefix;
    PString         m_commandErrorPrefix;
    PString         m_unknownCommandError;
    PString         m_ambiguousCommandError;
    PCaselessString m_scriptCommand;
    PString         m_noScriptError;
    int             m_pagerLines;
    PString         m_pagerWaitPrompt;
    PCaselessString m_pagerCommand;

    virtual bool InternalPageWait(Context & context);

    struct InternalCommand
    {
      InternalCommand(const PNotifier & notifier, const char * help, const char * usage, const char * argSpec, const char * varName);
      InternalCommand(const InternalCommand & other);
      ~InternalCommand();
      bool IsMatch(const PArgList & args, bool partial = false) const;
      bool operator<(const InternalCommand & other) const;

      PStringArray m_words;
      PString      m_command;
      PNotifier    m_notifier;
      PString      m_help;
      PString      m_usage;
      PString      m_argSpec;
      PString      m_varName;
      PVarType   * m_variable;
      PVarType   * m_minimum;
      PVarType   * m_maximum;
    };
    typedef std::set<InternalCommand> Commands_t;
    Commands_t m_commands;
    bool InternalSetCommand(const char * commands, const InternalCommand & info);

    virtual void OnSetBooleanCommand(Arguments & args, const InternalCommand & cmd);
    virtual void OnSetIntegerCommand(Arguments & args, const InternalCommand & cmd);

    typedef std::list<Context *> ContextList_t;
    ContextList_t m_contextList;
    PMutex        m_contextMutex;
};


/**Command Line Interpreter over standard input/output.
  */
class PCLIStandard : public PCLI
{
  public:
  /**@name Construction */
  //@{
    /**Create new command line interpreter for standard I/O.
      */
    PCLIStandard(
      const char * prompt = NULL
    );
  //@}

  /**@name Overrides from PCLI */
  //@{
    /** Start default foreground context.
        Default behaviour returns a context using stdin/stdout.
      */
    virtual Context * StartForeground();
  //@}

  /**@name Operations */
  //@{
    /**Run a script file, output going to stdout.
       This will also suppress output of the prompt.
      */
    bool RunScript(
      PChannel & channel            ///< Channel to read from
    ) { return RunScript(&channel, false); }
    bool RunScript(
      PChannel * channel,           ///< Channel to read from
      bool autoDelete = true        ///< Automatically delete channel on exit
    );
  //@}
};


/**Command Line Interpreter over TCP sockets.
   This class allows for access and automatic creation of command line
   interpreter contexts from incoming TCP connections on a listening port.
  */
class PCLISocket : public PCLI
{
  public:
  /**@name Construction */
  //@{
    PCLISocket(
      WORD port = 0,
      const char * prompt = NULL,
      bool singleThreadForAll = false
    );
    ~PCLISocket();
  //@}

  /**@name Overrides from PCLI */
  //@{
    /**Start a command line interpreter.
       This will start listening for incoming TCP connections and
       create/dispatch contexts to handle them.
      */
    virtual bool Start(
      bool runInBackground = true   ///< Spawn a thread to read and interpret commands
    );

    /**Stop and clean up command line interpreters.
       All the running contexts threads will be stopped, closing the channels
       and memory cleaned up.

       The listening socket is also closed and the listener dispatch thread
       shut down.
      */
    virtual void Stop();

    /**Add a command line interpreter context to the system.
       If context is NULL then CreateContext() is called to create one.
      */
    virtual Context * AddContext(
      Context * context = NULL
    );

    /**Remove the command line interpreter context.
       The context thread is stopped, the channel closed and memory deleted.
      */
    virtual void RemoveContext(
      Context * context
    );
  //@}

  /**@name Operations */
  //@{
    /**Start listening socket.
      */
    bool Listen(
      WORD port = 0
    );

    /**Get the port we are listing on.
      */
    WORD GetPort() const { return m_listenSocket.GetPort(); }
  //@}

  protected:
    PDECLARE_NOTIFIER(PThread, PCLISocket, ThreadMain);
    bool HandleSingleThreadForAll();
    bool HandleIncoming();
    virtual PTCPSocket * CreateSocket();

    bool m_singleThreadForAll;

    PTCPSocket m_listenSocket;
    PThread  * m_thread;

    typedef std::map<PSocket *, Context *> ContextMap_t;
    ContextMap_t m_contextBySocket;
};


#if P_TELNET
/**Command Line Interpreter over Telnet sockets.
   This class allows for access and automatic creation of command line
   interpreter contexts from incoming Telnet connections on a listening port.
  */
class PCLITelnet : public PCLISocket
{
  public:
  /**@name Construction */
  //@{
    PCLITelnet(
      WORD port = 0,
      const char * prompt = NULL,
      bool singleThreadForAll = false
    );
  //@}

  protected:
    virtual PTCPSocket * CreateSocket();
};
#endif // P_TELNET


#if P_CURSES
/**Command Line Interpreter within a text mode windowing environment.
   This class allows creation of a command line interpreter which is in a text
   mode windowing system, allowing output to happen in a cursor addressable
   manner.
  */
class PCLICurses : public PCLI
{
  public:
  /**@name Construction */
  //@{
    PCLICurses();
    ~PCLICurses();
  //@}

  /**@name Overrides from PCLI */
  //@{
    /** Start default foreground context.
        Default behaviour returns a context using stdin/stdout.
      */
    virtual Context * StartForeground();

    /**Create a new context.
       Users may use this to create derived classes for their own use.
      */
    virtual Context * CreateContext();
  //@}

    void GetScreenSize(
      unsigned & rows,
      unsigned & cols
    ) { rows = m_maxRows; cols = m_maxCols; }

    P_DECLARE_ENUM(Borders,
      NoBorder,
      FullBorder,
      BorderAbove,
      BorderBelow,
      BorderLeft,
      BorderRight
    );

    class Window : public PChannel
    {
    protected:
      PCLICurses & m_owner;
      Borders      m_border;
      bool         m_focus;

      Window(PCLICurses & owner, Borders border);

    public:
      // Overrides from PChannel
      virtual PString GetName() const { return "CursesWindow"; }
      virtual PBoolean Write(const void * data, PINDEX length);

      // New functions
      virtual bool FillChar(
        unsigned row,
        unsigned col,
        char ch,
        unsigned count
      ) = 0;

      virtual void SetPosition(
        unsigned row,
        unsigned col
      ) = 0;
      virtual void GetPosition(
        unsigned & row,
        unsigned & col
      ) = 0;

      virtual void SetSize(
        unsigned rows,
        unsigned cols,
        Borders border = NumBorders
      ) = 0;
      virtual void GetSize(
        unsigned & rows,
        unsigned & cols,
        bool includeBorder
      ) = 0;

      virtual void SetCursor(
        unsigned row,
        unsigned col
      ) = 0;
      virtual void GetCursor(
        unsigned & row,
        unsigned & col
      ) = 0;

      virtual void Refresh() = 0;
      virtual void Clear() = 0;
      virtual void Scroll(int n = 1) = 0;

      virtual void SetFocus();
      bool HasFocus() const { return m_focus; }
    };

    /** Create a new window.
        Note dimensions include the border.
      */
    Window & NewWindow(
      unsigned row,
      unsigned col,
      unsigned rows,
      unsigned cols,
      Borders border = NoBorder
    );

    void RemoveWindow(
      Window & wnd
    );

    void RemoveWindow(
      PINDEX idx
    );

    PINDEX GetWindowCount() const { return m_windows.GetSize(); }

    Window * GetWindow(
      PINDEX idx
    ) { return dynamic_cast<Window *>(m_windows.GetAt(idx)); }

    Window & operator[](PINDEX idx) const { return m_windows[idx]; }

    Window * GetFocusWindow() const;

    virtual void Refresh();

  protected:
    void Construct();

    virtual bool InternalPageWait(Context & context);

    PArray<Window> m_windows;
    unsigned       m_maxRows;
    unsigned       m_maxCols;
};
#endif // P_CURSES

#endif // P_CLI

#endif // PTLIB_CLI_H


// End Of File ///////////////////////////////////////////////////////////////
