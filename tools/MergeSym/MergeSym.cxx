/*
 * MergeSym.cxx
 *
 * Symbol merging utility for Windows DLL's.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
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
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Contributor(s): ______________________________________.
 */

#include <ptlib.h>
#include <ptlib/pipechan.h>
#include <ptlib/pprocess.h>
#include <set>

void unsetenv(const char *);

// -x ../../include/ptlib/msos/ptlib.ignore "..\..\lib\ptlibsd.lib" "..\..\include\ptlib\msos\ptlibs_2010.dtf" "..\..\lib\Win32\debug\$(TargetName).def"

class SymbolInfo
{
  public:
    SymbolInfo()
      : m_ordinal(0)
      , m_external(true)
      , m_noName(true)
    { }

    void Set(const PString & cpp, PINDEX ord, bool ext, bool nonam, const PString & synonym)
    {
      m_unmangled = cpp;
      m_ordinal = ord;
      m_external = ext;
      m_noName = nonam;
      m_synonym = synonym;
    }

    void SetOrdinal(PINDEX ord) { m_ordinal = ord; }
    unsigned GetOrdinal() const { return m_ordinal; }
    bool IsExternal() const { return m_external; }
    bool NoName() const { return m_noName; }
    void KeepName() { m_noName = false; }
    void Print(std::ostream & strm, const PString & name)
    {
      strm << "    ";
      if (!m_synonym.IsEmpty())
        strm << m_synonym << '=';
      strm << name;
      if (m_ordinal > 0)
        strm << " @" << m_ordinal;
      if (m_noName)
        strm << " NONAME";
      if (!m_unmangled.IsEmpty() && name != m_unmangled)
        strm << "\n    ;" << m_unmangled;
      strm << '\n';
    }

  private:
    PString m_unmangled;
    PINDEX  m_ordinal;
    bool    m_external;
    bool    m_noName;
    PString m_synonym;
};

typedef std::map<PCaselessString, SymbolInfo> SortedSymbolList;


std::ostream & operator<<(std::ostream & strm, const SortedSymbolList::iterator & it)
{
  it->second.Print(strm, it->first);
  return strm;
}


class WildcardNames : public list<PCaselessString>
{
public:
  bool Matches(const PCaselessString & str) const
  {
    for (const_iterator it = begin(); it != end(); ++it) {
      if (str.Find(*it) != P_MAX_INDEX)
        return true;
    }
    return false;
  }
};



PDECLARE_CLASS(MergeSym, PProcess)
  public:
    MergeSym();
    void Main();
};

PCREATE_PROCESS(MergeSym);


MergeSym::MergeSym()
  : PProcess("Equivalence", "MergeSym", 1, 12, ReleaseCode, 1, false, true)
{
}


void MergeSym::Main()
{
  cout << GetName() << " version " << GetVersion(true)
       << " on " << GetOSClass() << ' ' << GetOSName()
       << " by " << GetManufacturer() << endl;

  PArgList & args = GetArguments();
  args.Parse("v. Version\n"
             "s. Symbol file (output from dumpbin)\n"
             "d: Dumpbin executable (default \"dumpbin\")\n"
             "x: File of excluded symbols\n"
             "I: Search directory for excluded symbol files\n");

  PFilePath lib_filename, def_filename, out_filename;

  switch (args.GetCount()) {
    case 3 :
      out_filename = args[2];
      def_filename = args[1];
      lib_filename = args[0];
      break;

    case 2 :
      def_filename = out_filename = args[1];
      lib_filename = args[0];
      break;

    case 1 :
      lib_filename = def_filename = args[0];
      def_filename.SetType(".def");
      out_filename = def_filename;
      break;

    default :
      args.Usage(PError, "libfile[.lib] [ deffile[.def] [ outfile[.def] ] ]");
      SetTerminationValue(1);
      return;
  }

  if (lib_filename.GetType().IsEmpty())
    lib_filename.SetType(".lib");

  if (!PFile::Exists(lib_filename)) {
    PError << "MergeSym: library file " << lib_filename << " does not exist.\n";
    SetTerminationValue(1);
    return;
  }

  if (def_filename.GetType().IsEmpty())
    def_filename.SetType(".def");

  if (out_filename.GetType().IsEmpty())
    out_filename.SetType(".def");

  SortedSymbolList def_symbols;
  std::vector<bool> ordinals_used(65536);
  WildcardNames wildcards;

  const char * const DefaultWildcards[] = {
    "$$F",
    "??_C@_",
    "?__LINE__Var@",
    "_CT?",
    "__real@",
    "__xmm@",
    "__m2mep@?",
    "__mep@?",
    "__t2m@?",
    "___@",
    "__@@_PchSym_@",
    "_TI2?AVbad_cast@std"
  };
  for (PINDEX i = 0; i < PARRAYSIZE(DefaultWildcards); ++i)
    wildcards.push_back(DefaultWildcards[i]);

  if (args.HasOption('x')) {
    PStringArray include_path;
    if (args.HasOption('I')) {
      PString includes = args.GetOptionString('I');
      if (includes.Find(';') == P_MAX_INDEX)
        include_path = includes.Tokenise(',', false);
      else
        include_path = includes.Tokenise(';', false);
    }
    include_path.InsertAt(0, new PString());
    PStringArray file_list = args.GetOptionString('x').Lines();
    for (PINDEX ext_index = 0; ext_index < file_list.GetSize(); ext_index++) {
      PString base_ext_filename = file_list[ext_index];
      PString ext_filename = base_ext_filename;
      if (PFilePath(ext_filename).GetType().IsEmpty())
        ext_filename += ".def";

      PINDEX previous_def_symbols_size = def_symbols.size();
      PINDEX inc_index = 0;
      for (inc_index = 0; inc_index < include_path.GetSize(); inc_index++) {
        PString trial_filename;
        if (PFilePath::IsAbsolutePath(ext_filename))
          trial_filename = ext_filename;
        else
          trial_filename = PDirectory(include_path[inc_index]) + ext_filename;
        if (args.HasOption('v'))
          cout << "\nTrying " << trial_filename << " ..." << flush;
        PTextFile ext;
        if (ext.Open(trial_filename, PFile::ReadOnly)) {
          if (args.HasOption('v'))
            cout << "\nReading external symbols from " << ext.GetFilePath() << " ..." << flush;
          bool prefix = true;
          while (!ext.eof()) {
            PCaselessString line;
            ext >> line;
            if (prefix)
              prefix = line.Find("EXPORTS") == P_MAX_INDEX;
            else {
              PINDEX start = 0;
              while (isspace(line[start]))
                start++;
              PINDEX end = start;
              while (line[end] != '\0' && !isspace(line[end]))
                end++;
              PString symName = line(start, end-1);
              if (symName.Find('*') != P_MAX_INDEX)
                wildcards.push_back(symName.Replace("*", "", true));
              else
                def_symbols[symName]; // Create default
              if (args.HasOption('v') && def_symbols.size()%100 == 0)
                cout << '.' << flush;
            }
          }
          break;
        }
      }
      if (inc_index >= include_path.GetSize())
        PError << "MergeSym: external symbol file \"" << base_ext_filename << "\" not found.\n";
      if (args.HasOption('v'))
        cout << '\n' << (def_symbols.size() - previous_def_symbols_size)
             << " symbols read." << endl;
    }
  }

  PStringList def_file_lines;
  PINDEX next_ordinal = 0;
  PINDEX removed = 0;

  PTextFile def;
  if (def.Open(def_filename, PFile::ReadOnly)) {
    if (args.HasOption('v'))
      cout << "Reading existing ordinals..." << flush;
    bool prefix = true;
    while (!def.eof()) {
      PCaselessString line;
      def >> line;
      if (prefix) {
        def_file_lines.AppendString(line);
        if (line.Find("EXPORTS") != P_MAX_INDEX)
          prefix = false;
      }
      else {
        PINDEX start = 0;
        while (isspace(line[start]))
          start++;
        if (line[start] != ';') {
          PINDEX end = start;
          while (line[end] != '\0' && !isspace(line[end]))
            end++;
          PINDEX ordpos = line.Find('@', end);
          if (ordpos != P_MAX_INDEX) {
            PINDEX ordinal = line.Mid(ordpos+1).AsInteger();
            ordinals_used[ordinal] = true;
            if (ordinal > next_ordinal)
              next_ordinal = ordinal;
            bool noname = line.Find("NONAME", ordpos) != P_MAX_INDEX;
            PCaselessString synonym, symbol;
            line(start, end-1).Split('=', synonym, symbol, PString::SplitDefaultToAfter|PString::SplitTrim);
            if (def_symbols.find(symbol) == def_symbols.end())
              def_symbols[symbol].Set(PString::Empty(), ordinal, false, noname, synonym);
            removed++;
            if (args.HasOption('v') && def_symbols.size()%100 == 0)
              cout << '.' << flush;
          }
        }
      }
    }
    def.Close();
    if (args.HasOption('v'))
      cout << '\n' << removed << " symbols read." << endl;
  }
  else {
    def_file_lines.AppendString("LIBRARY " + def_filename.GetTitle());
    def_file_lines.AppendString("EXPORTS");
  }

  if (args.HasOption('v'))
    cout << "\nReading library symbols..." << flush;

  unsetenv("VS_UNICODE_OUTPUT");

  PINDEX linecount = 0;
  PString dumpbin = args.GetOptionString('d', "dumpbin");
  PPipeChannel pipe(PSTRSTRM(dumpbin << " /symbols /directives \"" << lib_filename << '"'), PPipeChannel::ReadOnly);
  if (!pipe.IsOpen()) {
    PError << "\nMergeSym: could not run \"" << dumpbin << "\".\n";
    SetTerminationValue(2);
    return;
  }

  PTextFile symfile;
  if (args.HasOption('s')) {
    PFilePath sym_filename = out_filename;
    sym_filename.SetType(".sym");
    if (!symfile.Open(sym_filename, PFile::WriteOnly))
      cerr << "Could not open symbol file " << sym_filename << endl;
  }

  std::set<PString> explicitExports;
  SortedSymbolList lib_symbols;

  PSimpleTimer duration;
  while (!pipe.eof()) {
    PString line;
    pipe >> line;
    symfile << line << '\n';

    PINDEX namepos = line.Find('|');
    if (namepos != P_MAX_INDEX &&
        line.Find(" UNDEF ") == P_MAX_INDEX &&
        line.Find(" External ") != P_MAX_INDEX &&
        line.Find("deleting destructor") == P_MAX_INDEX) {

      while (line[++namepos] == ' ')
        ;
      PINDEX nameend = line.FindOneOf("\r\n\t ", namepos);
      PString name = line(namepos, nameend-1);
      if (name.FindOneOf("'\"=") == P_MAX_INDEX && lib_symbols.find(name) == lib_symbols.end()) {
        bool add = true;
        PString unmangled;

        PINDEX unmangledpos = line.Find('(', nameend);
        if (unmangledpos != P_MAX_INDEX) {
          unmangled = line(unmangledpos+1, line.FindLast(')')-1);
          if (unmangled.FindOneOf("%^") != P_MAX_INDEX)
            add = false;
          else {
            static const char * const ConventionNames[] = {
              "__cdecl", "__clrcall", "__stdcall", "__fastcall", "__thiscall", "__vectorcall"
            };
            PINDEX convention;
            for (PINDEX i = 0; i < PARRAYSIZE(ConventionNames); ++i) {
              if ((convention = unmangled.Find(ConventionNames[i])) != P_MAX_INDEX)
                break;
            }
            PINDEX colons = unmangled.Find("::", convention);
            PINDEX paren = unmangled.Find('(', convention);
            if (colons < paren && colons > 4 && unmangled.NumCompare(" std", 4, colons-4) == EqualTo)
              add = false;
          }
        }
        else {
          if (name[0] != '_' && !isalnum(name[0]))
              add = false;
        }

        if (add)
          lib_symbols[name].Set(unmangled,
                                0,
                                wildcards.Matches(name),
                                explicitExports.find(name) == explicitExports.end(),
                                PString::Empty());
      }
    }
    else if ((namepos = line.Find("/EXPORT:")) != P_MAX_INDEX) {
      PString name = line.Mid(namepos+8);
      if (args.HasOption('v'))
        cout << "Explicit export \"" << name << '"' << endl;
      explicitExports.insert(name);
      SortedSymbolList::iterator it = def_symbols.find(name);
      if (it != def_symbols.end())
        it->second.KeepName();
    }

    if (args.HasOption('v') && linecount%500 == 0)
      cout << '.' << flush;
    linecount++;
  }

  if (args.HasOption('v'))
    cout << '\n' << lib_symbols.size() << " symbols read in " << duration.GetElapsed() << " seconds.\n"
            "Sorting symbols... " << flush;

  SortedSymbolList::iterator it;
  for (it = def_symbols.begin(); it != def_symbols.end(); ++it) {
    if (lib_symbols.find(it->first) != lib_symbols.end() && !it->second.IsExternal())
      removed--;
    else
      ordinals_used[it->second.GetOrdinal()] = false;
  }

  PINDEX added = 0;
  for (it = lib_symbols.begin(); it != lib_symbols.end(); ++it) {
    if (def_symbols.find(it->first) == def_symbols.end()) {
      if (++next_ordinal > 65535)
        next_ordinal = 1;
      while (ordinals_used[next_ordinal]) {
        if (++next_ordinal > 65535) {
          cerr << "Catastrophe! More than 65535 exported symbols in DLL!" << endl;
          SetTerminationValue(1);
          return;
        }
      }
      it->second.SetOrdinal(next_ordinal);
      ordinals_used[next_ordinal] = true;
      added++;
    }
  }

  if (added == 0 && removed == 0) {
    cout << "\nNo changes to symbols.\n";
    PFile::Copy(def_filename, out_filename, true);
    return;
  }

  SortedSymbolList merged_symbols;
  PINDEX i;

  for (i = 0, it = def_symbols.begin(); it != def_symbols.end(); ++it, ++i) {
    if (lib_symbols.find(it->first) != lib_symbols.end() && !it->second.IsExternal())
      merged_symbols.insert(*it);
    if (args.HasOption('v') && i%100 == 0)
      cout << '.' << flush;
  }
  for (i = 0, it = lib_symbols.begin(); it != lib_symbols.end(); ++it, ++i) {
    if (def_symbols.find(it->first) == def_symbols.end() && !it->second.IsExternal())
      merged_symbols.insert(*it);
    if (args.HasOption('v') && i%100 == 0)
      cout << '.' << flush;
  }

  cout << "\nSymbols merged: " << added << " added, "
       << removed << " removed, "
       << merged_symbols.size() << " total.\n";

  if (def_filename == out_filename)
    return;

  if (args.HasOption('v'))
    cout << "Writing " << out_filename << " ..." << flush;

  // If file is read/only, set it to read/write
  PFileInfo info;
  if (PFile::GetInfo(out_filename, info)) {
    if ((info.permissions&PFileInfo::UserWrite) == 0) {
      PFile::SetPermissions(out_filename, info.permissions|PFileInfo::UserWrite);
      cout << "Setting \"" << out_filename << "\" to read/write mode." << flush;
      PFile::Copy(out_filename, out_filename+".original");
    }
  }

  if (!def.Open(out_filename, PFile::WriteOnly)) {
    cerr << "Could not create file " << out_filename << ':' << def.GetErrorText() << endl;
    SetTerminationValue(1);
    return;
  }

  for (PStringList::iterator is = def_file_lines.begin(); is != def_file_lines.end(); ++is)
    def << *is << '\n';
  for (it = merged_symbols.begin(); it != merged_symbols.end(); ++it, ++i) {
    if (!it->second.NoName())
      def << it;
  }
  for (it = merged_symbols.begin(); it != merged_symbols.end(); ++it, ++i) {
    if (it->second.NoName())
      def << it;
  }

  if (args.HasOption('v'))
    cout << merged_symbols.size() << " symbols written." << endl;
} 
// End MergeSym.cxx

 void  unsetenv(const char *name) 
 { 
     char       *envstr; 
  
     if (getenv(name) == NULL) 
         return;                 /* no work */ 
  
     /* 
      * The technique embodied here works if libc follows the Single Unix Spec 
      * and actually uses the storage passed to putenv() to hold the environ 
      * entry.  When we clobber the entry in the second step we are ensuring 
      * that we zap the actual environ member.  However, there are some libc 
      * implementations (notably recent BSDs) that do not obey SUS but copy the 
      * presented string.  This method fails on such platforms.  Hopefully all 
      * such platforms have unsetenv() and thus won't be using this hack. 
      * 
      * Note that repeatedly setting and unsetting a var using this code will 
      * leak memory. 
      */ 
  
     envstr = (char *) malloc(strlen(name) + 2); 
     if (!envstr)                /* not much we can do if no memory */ 
         return; 
  
     /* Override the existing setting by forcibly defining the var */ 
     sprintf(envstr, "%s=", name); 
     _putenv(envstr); 
  
     /* Now we can clobber the variable definition this way: */ 
     strcpy(envstr, "="); 
  
     /* 
      * This last putenv cleans up if we have multiple zero-length names as a 
      * result of unsetting multiple things. 
      */ 
     _putenv(envstr); 
 }

