/*
 * script.h
 *
 * Abstract class for interface to external script languages
 *
 * Portable Tools Library
 *
 * Copyright (C) 2010 by Post Increment
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
 * The Original Code is Portable Tools Library.
 *
 * The Initial Developer of the Original Code is Post Increment
 *
 * Contributor(s): Craig Southeren
 *                 Robert Jongbloed
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#ifndef PTLIB_SCRIPT_H
#define PTLIB_SCRIPT_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptlib/pfactory.h>
#include <ptclib/vartype.h>


//////////////////////////////////////////////////////////////

/**A wrapper around a scripting language instance.
 */
class PScriptLanguage : public PObject
{
  PCLASSINFO(PScriptLanguage, PObject)
  public:
    static PScriptLanguage * Create(const PString & language)
    {
      return PFactory<PScriptLanguage>::CreateInstance(language);
    }
    static PStringArray GetLanguages()
    {
      return PFactory<PScriptLanguage>::GetKeyList();
    }

  /**@name Construction */
  //@{
    /**Create a context in which to execute a script.
     */
    PScriptLanguage();

    /// Destroy the script context.
    ~PScriptLanguage();
  //@}

  /**@name Scripting functions */
  //@{
    /// Get the name of this scripting language
    virtual PString GetLanguageName() const = 0;

    /**Load a script from a file.
      */
    virtual bool LoadFile(
      const PFilePath & filename  ///< Name of script file to load
    ) = 0;

    /** Load script text.
      */
    virtual bool LoadText(
      const PString & text  ///< Script text to load.
    ) = 0;

    /**Load script from a file (if exists) or assume is the actual script.
      */
    virtual bool Load(
      const PString & script  ///< Name of script file or script itself to load
    );

    /**Run the script.
       If \p script is NULL or empty then the currently laoded script is
       executed. If \p script is an existing file, then that will be loaded
       and executed. All other cases the string is loaded as direct script
       text and executed.
      */
    virtual bool Run(
      const char * script = NULL
    ) = 0;

    /**Create a composite structure.
       The exact semantics is language dependant. For Lua this is a table.

       See class description for how \p name is parsed.
      */
    virtual bool CreateComposite(
      const PString & name   ///< Name of new composite structure
    ) = 0;

    /**Get a variable in the script 
       See class description for how \p name is parsed.
      */
    virtual bool GetVar(
      const PString & name,  ///< Name of global
      PVarType & var
    ) = 0;

    /**Set a variable in the script 
       See class description for how \p name is parsed.
      */
    virtual bool SetVar(
      const PString & name, ///< Name of global
      const PVarType & var
    ) = 0;

    /**Get a variable in the script as a string value.
       See class description for how \p name is parsed.
      */
    virtual bool GetBoolean(
      const PString & name  ///< Name of global
    ) = 0;

    /**Set a variable in the script as a string value.
       See class description for how \p name is parsed.
      */
    virtual bool SetBoolean(
      const PString & name, ///< Name of global
      bool value            ///< New value
    ) = 0;

    /**Get a variable in the script as an integer value.
       See class description for how \p name is parsed.
      */
    virtual int GetInteger(
      const PString & name  ///< Name of global
    ) = 0;

    /**Set a variable in the script as an integer value.
       See class description for how \p name is parsed.
      */
    virtual bool SetInteger(
      const PString & name, ///< Name of global
      int value             ///< New value
    ) = 0;

    /**Get a variable in the script as a number value.
       See class description for how \p name is parsed.
      */
    virtual double GetNumber(
      const PString & name  ///< Name of global
    ) = 0;

    /**Set a variable in the script as a number value.
       See class description for how \p name is parsed.
      */
    virtual bool SetNumber(
      const PString & name, ///< Name of global
      double value          ///< New value
    ) = 0;

    /**Get a variable in the script as a string value.
       See class description for how \p name is parsed.
      */
    virtual PString GetString(
      const PString & name  ///< Name of global
    ) = 0;

    /**Set a variable in the script as a string value.
       See class description for how \p name is parsed.
      */
    virtual bool SetString(
      const PString & name, ///< Name of global
      const char * value    ///< New value
    ) = 0;

    /**Release a variable name.
       Note the exact semantics is language dependant. It generally applies
       to global variables as most languages have automatic garbage collection
       for other variable types.
      */
    virtual bool ReleaseVariable(
      const PString & name    ///< Name of table to delete
    ) = 0;

    /// Individual Parameter in ParamVector.
    /// Vector of parameters as used by Signature structure.
    struct ParamVector : public vector<PVarType>
    {
      ParamVector(size_t sz = 0) : vector<PVarType>(sz) { }
      void Push(void * state);
      void Pop(void * state);
    };

    /// Signature of Lua function and callback.
    struct Signature {
      ParamVector m_arguments;
      ParamVector m_results;
    };

    /**Call a specific function in the script.
       The \p sigString indicates the types of the arguments and return values
       for the function. The types available are:
         'b' for boolean,
         'i' for integer,
         'n' for a number (double float)
         's' for string (const char * or char *)
         'p' for user defined (void *)

       A '>' separates arguments from return values. The same letters are used
       for the tpes, but a pointer to the variable is supplied in the argument
       list, as for scanf. Note there can be multiple return values.

       if 's' is used as a return value, then the caller is expected to delete
       the returned string pointer as it is allocated on the heap.

       If \p sigString is NULL or empty then a void parameterless function is
       called.

       The second form with \p signature alows for the caller to adaptively
       respond to different return types.

       See class description for how \p name is parsed.

       @returns false if function does not exist.
      */
    virtual bool Call(
      const PString & name,           ///< Name of function to execute.
      const char * sigString = NULL,  ///< Signature of arguments following
      ...
    ) = 0;
    virtual bool Call(
      const PString & name,       ///< Name of function to execute.
      Signature & signature ///< Signature of arguments following
    ) = 0;

    typedef PNotifierTemplate<Signature &>  FunctionNotifier;
    #define PDECLARE_ScriptFunctionNotifier(cls, fn) PDECLARE_NOTIFIER2(PScriptLanguage, cls, fn, PScriptLanguage::Signature &)

    /**Set a notifier as a script callable function.
       See class description for how \p name is parsed.
      */
    virtual bool SetFunction(
      const PString & name,         ///< Name of function script can call
      const FunctionNotifier & func ///< Notifier excuted
    ) = 0;
  //@}

  /**@name member variables */
  //@{
    /// Rerturn true if script is successfully loaded.
    __inline bool IsLoaded() const { return m_loaded; }

    /// Get the last error text for an operation.
    virtual int GetLastErrorCode() const { return m_lastErrorCode; }

    /// Get the last error text for an operation.
    virtual const PString & GetLastErrorText() const { return m_lastErrorText; }
  //@}

  protected:
    /**Set m_lastErrorCode and m_lastErrorText members, with mutex.
      */
    virtual void OnError(int code, const PString & str);

    virtual bool InternalSetFunction(const PString & name, const FunctionNotifier & func);
    virtual void InternalRemoveFunction(const PString & prefix);

    bool m_loaded;
    int m_lastErrorCode;
    PString m_lastErrorText;
    typedef map<PString, FunctionNotifier> FunctionMap;
    FunctionMap m_functions;

    PMutex m_mutex;
};


#if P_LUA
  PFACTORY_LOAD(PLua);
#endif
#if P_V8
  PFACTORY_LOAD(PJavaScript);
#endif


#endif  // PTLIB_SCRIPT_H

