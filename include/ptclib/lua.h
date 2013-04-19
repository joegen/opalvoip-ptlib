/*
 * lua.h
 *
 * Interface library for Lua interpreter
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

#ifndef PTLIB_LUA_H
#define PTLIB_LUA_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptlib.h>

#if P_LUA

#include <ptclib/script.h>
#include <ptclib/vartype.h>


struct lua_State;


//////////////////////////////////////////////////////////////

/**A wrapper around a Lua scripting language instance.

   The \p name indicates the variable name of the form
     var[.var[.var ...]]
   where var is the usual alphanumeric+underscore string for identifier names
   as used in C and many other languages. The dots separate Lua tables and
   fields of that table. Tables may be nested.
 */
class PLua : public PScriptLanguage
{
    PCLASSINFO(PLua, PScriptLanguage)
  public:
  /**@name Construction */
  //@{
    /**Create a context in which to execute a Lua script.
     */
    PLua();

    /// Destroy the Lua script context.
    ~PLua();
  //@}

  /**@name Path addition functions */
  //@{
    /**Load a Lua script from a file.
      */
    virtual bool LoadFile(
      const PFilePath & filename  ///< Name of script file to load
    );

    /** Load a Lua script text.
      */
    virtual bool LoadText(
      const PString & text  ///< Script text to load.
    );

    /**Run the script.
       If \p script is NULL or empty then the currently laoded script is
       executed. If \p script is an existing file, then that will be loaded
       and executed. All other cases the string is laoded as direct script
       text and executed.
      */
    virtual bool Run(
      const char * script = NULL
    );

    /**Create a table with optional metatable.
       If the metatable does not exist it is created.

       See class description for how \p name is parsed.
      */
    bool CreateTable(
      const PString & name,   ///< Name of new table
      const PString & metatable = PString::Empty() ///< Metatable to attach
    );

    /**Delete table.
       Note this only applies to metatables and global tables. Tables
       contained within other tables are garbage collected when the
       enclosing table no longer reference it.
      */
    bool DeleteTable(
      const PString & name,   ///< Name of table to delete
      bool metaTable = false  ///< Table it a metatable
    );


    /**Create a composite structure.
       For Lua this creates a table.

       See class description for how \p name is parsed.
      */
    virtual bool CreateComposite(
      const PString & name   ///< Name of new table
    );

    /**Get a variable in the script 
       See class description for how \p name is parsed.
      */
    virtual bool GetVar(
      const PString & name,  ///< Name of global
      PVarType & var
    );

    /**Set a variable in the script 
       See class description for how \p name is parsed.
      */
    virtual bool SetVar(
      const PString & name, ///< Name of global
      const PVarType & var
    );

    /**Get a variable in the script as a string value.
       See class description for how \p name is parsed.
      */
    bool GetBoolean(
      const PString & name  ///< Name of global
    );

    /**Set a variable in the script as a string value.
       See class description for how \p name is parsed.
      */
    bool SetBoolean(
      const PString & name, ///< Name of global
      bool value            ///< New value
    );

    /**Get a variable in the script as an integer value.
       See class description for how \p name is parsed.
      */
    int GetInteger(
      const PString & name  ///< Name of global
    );

    /**Set a variable in the script as an integer value.
       See class description for how \p name is parsed.
      */
    bool SetInteger(
      const PString & name, ///< Name of global
      int value             ///< New value
    );

    /**Get a variable in the script as a number value.
       See class description for how \p name is parsed.
      */
    double GetNumber(
      const PString & name  ///< Name of global
    );

    /**Set a variable in the script as a number value.
       See class description for how \p name is parsed.
      */
    bool SetNumber(
      const PString & name, ///< Name of global
      double value          ///< New value
    );

    /**Get a variable in the script as a string value.
       See class description for how \p name is parsed.
      */
    PString GetString(
      const PString & name  ///< Name of global
    );

    /**Set a variable in the script as a string value.
       See class description for how \p name is parsed.
      */
    bool SetString(
      const PString & name, ///< Name of global
      const char * value    ///< New value
    );

    /**Release a variable name.
       Note this only applies to metatables and global tables/variables.
       Variables contained within other tables are garbage collected when the
       enclosing table no longer reference it.
      */
    virtual bool ReleaseVariable(
      const PString & name    ///< Name of table to delete
    );

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
    bool Call(
      const PString & name,           ///< Name of function to execute.
      const char * sigString = NULL,  ///< Signature of arguments following
      ...
    );
    bool Call(
      const PString & name,       ///< Name of function to execute.
      Signature & signature ///< Signature of arguments following
    );


    #define PDECLARE_LuaFunctionNotifier(cls, fn) PDECLARE_NOTIFIER2(PLua, cls, fn, PScriptLanguage::Signature &)

    /**Set a notifier as a Lua callable function.
       See class description for how \p name is parsed.
      */
    bool SetFunction(
      const PString & name,         ///< Name of function Lua script can call
      const FunctionNotifier & func ///< Notifier excuted
    );
  //@}

  protected:
    /**Check for an error and set m_lastErrorText to error text.
      */
    virtual bool OnLuaError(int code, const PString & str = PString::Empty(), int pop = 0);

    bool ParseVariableName(const PString & name, PStringArray & vars);
    bool InternalGetVariable(const PString & name);
    bool InternalSetVariable(const PString & name);
    static int InternalCallback(lua_State * state);
    int InternalCallback();

    lua_State * m_lua;
};


#endif // P_LUA

#endif  // PTLIB_LUA_H

