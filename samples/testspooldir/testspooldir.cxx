//
//  test program for spool dir class
//

#include <ptlib.h>
#include <ptlib/pprocess.h>

#include <ptclib/spooldir.h>


///////////////////////////////////////////////////////

class TestSpoolDir : public PProcess
{
  PCLASSINFO(TestSpoolDir, PProcess)
  public:
    void Main();

    bool m_verbose;
};

PCREATE_PROCESS(TestSpoolDir)

void TestSpoolDir::Main()
{
  PArgList & args = GetArguments();

  args.Parse(
             "h-help."
             "v-version."
#if PTRACING
             "o-output:"             "-no-output."
             "t-trace."              "-no-trace."
#endif
  );

  m_verbose = false;
 
  if (args.HasOption('h')) {
    cout << "usage: " <<  (const char *)GetName()
         << endl
         << "  -v" << endl
#if PTRACING
         << "  -t --trace   : Enable trace, use multiple times for more detail" << endl
         << "  -o --output  : File for trace output, default is stderr" << endl
#endif
         << endl;
    return;
  }

 m_verbose = args.HasOption('v');

 if (args.GetCount() < 1) {
   PError << "error: no directory specified" << endl;
   return;
 }

 PSpoolDirectory spoolDir;
 if (!spoolDir.Open(args[0], ".tif")) {
   PError << "error: unable to open spool directory '" << args[0] << "'" << endl;
   return;
 }

#if PTRACING
  PTrace::Initialise(args.GetOptionCount('t'),
                     args.HasOption('o') ? (const char *)args.GetOptionString('o') : NULL,
         PTrace::Blocks | PTrace::Timestamp | PTrace::Thread | PTrace::FileAndLine);
#endif

  Sleep(10000000);
}

// End of testspooldir.cxx
