// MergeSym.cxx

#include <ptlib.h>

PDECLARE_CLASS(Symbol, PCaselessString)
  public:
    Symbol(const PString & sym, const PString & cpp, PINDEX ord = 0, BOOL ext = FALSE)
      : PCaselessString(sym), unmangled(cpp) { ordinal = ord; external = ext; }

    void SetOrdinal(PINDEX ord) { ordinal = ord; }
    BOOL IsExternal() const { return external; }

    void PrintOn(ostream & s) const
      { s << "    " << theArray << " @" << ordinal << " NONAME ;" << unmangled << '\n'; }

  private:
    PString unmangled;
    PINDEX ordinal;
    BOOL external;
};

PSORTED_LIST(SortedSymbolList, Symbol);


PDECLARE_CLASS(MergeSym, PProcess)
  public:
    void Main();
};

PCREATE_PROCESS(MergeSym);


void MergeSym::Main()
{
  PArgList & args = GetArguments();
  args.Parse("vx:");

  PFilePath lib_filename, def_filename;

  switch (args.GetCount()) {
    case 2 :
      def_filename = args[1];
      lib_filename = args[0];
      break;

    case 1 :
      lib_filename = def_filename = args[0];
      def_filename.SetType(".def");
      break;

    default :
      PError << "usage: MergeSym [ -v ] [ -x deffile[.def] ] libfile[.lib] [ deffile[.def] ]";
      return;
  }

  if (lib_filename.GetType().IsEmpty())
    lib_filename.SetType(".lib");

  if (!PFile::Exists(lib_filename)) {
    PError << "MergeSym: library file " << lib_filename << " does not exist.\n";
    return;
  }

  if (def_filename.GetType().IsEmpty())
    def_filename.SetType(".def");

  SortedSymbolList def_symbols;

  if (args.HasOption('x')) {
    PFilePath ext_filename = args.GetOptionString('x');
    if (ext_filename.GetType().IsEmpty())
      ext_filename.SetType(".def");

    if (args.HasOption('v'))
      PError << "Reading external symbols..." << flush;
    PTextFile ext;
    if (ext.Open(ext_filename, PFile::ReadOnly)) {
      BOOL prefix = TRUE;
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
          def_symbols.Append(new Symbol(line(start, end-1), "", 0, TRUE));
          if (args.HasOption('v') && def_symbols.GetSize()%100 == 0)
            PError << '.' << flush;
        }
      }
      if (args.HasOption('v'))
        PError << '\n' << def_symbols.GetSize() << " symbols read.\n";
    }
  }

  PStringList def_file_lines;
  PINDEX max_ordinal = 0;
  PINDEX removed = 0;

  PTextFile def;
  if (def.Open(def_filename, PFile::ReadOnly)) {
    if (args.HasOption('v'))
      PError << "Reading existing ordinals..." << flush;
    BOOL prefix = TRUE;
    while (!def.eof()) {
      PCaselessString line;
      def >> line;
      if (prefix) {
        def_file_lines.AppendString(line);
        if (line.Find("EXPORTS") != P_MAX_INDEX)
          prefix = FALSE;
      }
      else {
        PINDEX start = 0;
        while (isspace(line[start]))
          start++;
        PINDEX end = start;
        while (line[end] != '\0' && !isspace(line[end]))
          end++;
        PINDEX ordpos = line.Find('@', end);
        if (ordpos != P_MAX_INDEX) {
          PINDEX ordinal = line.Mid(ordpos+1).AsInteger();
          if (ordinal > max_ordinal)
            max_ordinal = ordinal;
          PINDEX unmanglepos = line.Find(';', ordpos);
          if (unmanglepos != P_MAX_INDEX)
            unmanglepos++;
          def_symbols.Append(new Symbol(line(start, end-1), line.Mid(unmanglepos), ordinal));
          removed++;
          if (args.HasOption('v') && def_symbols.GetSize()%100 == 0)
            PError << '.' << flush;
        }
      }
    }
    def.Close();
    if (args.HasOption('v'))
      PError << '\n' << removed << " symbols read.\n";
  }
  else {
    def_file_lines.AppendString("LIBRARY " + def_filename.GetTitle());
    def_file_lines.AppendString("EXPORTS");
  }

  if (args.HasOption('v'))
    PError << "Reading library symbols..." << flush;
  PINDEX linecount = 0;
  SortedSymbolList lib_symbols;
  PPipeChannel pipe("dumpbin /symbols " + lib_filename, PPipeChannel::ReadOnly);
  while (!pipe.eof()) {
    char line[500];
    pipe.getline(line, sizeof(line));
    char * namepos = strchr(line, '|');
    if (namepos != NULL) {
      *namepos = '\0';
      while (*++namepos == ' ');
      if (strstr(line, " UNDEF ") == NULL &&
          strstr(line, " External ") != NULL &&
          strstr(namepos, "deleting destructor") == NULL) {
        int namelen = strcspn(namepos, "\r\n\t ");
        PString name(namepos, namelen);
        if (lib_symbols.GetValuesIndex(name) == P_MAX_INDEX) {
          const char * unmangled = strchr(namepos+namelen, '(');
          if (unmangled == NULL)
            unmangled = name;
          else {
            unmangled++;
            char * endunmangle = strrchr(unmangled, ')');
            if (endunmangle != NULL)
              *endunmangle = '\0';
          }
          lib_symbols.Append(new Symbol(name, unmangled));
        }
      }
    }
    if (args.HasOption('v') && linecount%500 == 0)
      PError << '.' << flush;
    linecount++;
  }

  if (args.HasOption('v'))
    PError << '\n' << lib_symbols.GetSize() << " symbols read.\n"
              "Writing .DEF file..." << flush;
  if (def.Open(def_filename, PFile::WriteOnly)) {
    PINDEX i;
    for (i = 0; i < def_file_lines.GetSize(); i++)
      def << def_file_lines[i] << '\n';
    PINDEX added = 0;
    for (i = 0; i < def_symbols.GetSize(); i++) {
      if (lib_symbols.GetValuesIndex(def_symbols[i]) != P_MAX_INDEX) {
        if (!def_symbols[i].IsExternal()) {
          def << def_symbols[i];
          removed--;
        }
      }
      if (args.HasOption('v') && i%100 == 0)
        PError << '.' << flush;
    }
    for (i = 0; i < lib_symbols.GetSize(); i++) {
      if (def_symbols.GetValuesIndex(lib_symbols[i]) == P_MAX_INDEX) {
        lib_symbols[i].SetOrdinal(++max_ordinal);
        def << lib_symbols[i];
        added++;
      }
      if (args.HasOption('v') && i%100 == 0)
        PError << '.' << flush;
    }
    PError << "\nSymbols merged: " << added << " added, " << removed << " removed.\n";
  }
  else
    PError << "Could not create file " << def_filename << ':' << def.GetErrorText() << endl;
}


// End MergeSym.cxx
