/*
 * svcproc.h
 *
 * Service Process (daemon) class.
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
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 * $Id$
 */


///////////////////////////////////////////////////////////////////////////////
// PServiceProcess

#undef PCREATE_PROCESS
#define PCREATE_PROCESS(cls) \
  extern "C" int PASCAL WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int) \
    { PProcess::PreInitialise(__argc, __argv, _environ); \
      static cls instance; \
      return instance._main(hInst); \
    }


  public:
    virtual const char * GetServiceDependencies() const;
      // Get a set of null terminated strings terminated with double null.

    virtual BOOL IsServiceProcess() const;

  protected:
    PCaselessString systemLogFileName;
    BOOL debugHidden; /// Flag to indicate service is run in simulation mode without showing the control window 

  private:
    static void __stdcall StaticMainEntry(DWORD argc, LPTSTR * argv);
    /* Internal function called from the Service Manager. This simply calls the
       <A>MainEntry()</A> function on the PServiceProcess instance.
    */

    void MainEntry(DWORD argc, LPTSTR * argv);
    /* Internal function function that takes care of actually starting the
       service, informing the service controller at each step along the way.
       After launching the worker thread, it waits on the event that the worker
       thread will signal at its termination.
    */

    static void StaticThreadEntry(void *);
    /* Internal function called to begin the work of the service process. This
       essentially just calls the <A>Main()</A> function on the
       PServiceProcess instance.
    */

    void ThreadEntry();
    /* Internal function function that starts the worker thread for the
       service.
    */

    static void __stdcall StaticControlEntry(DWORD code);
    /* This function is called by the Service Controller whenever someone calls
       ControlService in reference to our service.
     */

    void ControlEntry(DWORD code);
    /* This function is called by the Service Controller whenever someone calls
       ControlService in reference to our service.
     */

    static void Control_C(int);
    /* This function is called on a SIGINTR (Control-C) signal for use in
       debug mode.
     */

    BOOL ReportStatus(
      DWORD dwCurrentState,
      DWORD dwWin32ExitCode = NO_ERROR,
      DWORD dwCheckPoint = 0,
      DWORD dwWaitHint = 0
    );
    /* This function is called by the Main() and Control() functions to update the
       service's status to the service control manager.
     */


    BOOL ProcessCommand(const char * cmd);
    // Process command line argument for controlling the service.

    BOOL CreateControlWindow(BOOL createDebugWindow);
    static LPARAM WINAPI StaticWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LPARAM WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void DebugOutput(const char * out);

    BOOL                  isWin95;
    SERVICE_STATUS        status;
    SERVICE_STATUS_HANDLE statusHandle;
    HANDLE                startedEvent;
    HANDLE                terminationEvent;
    HWND                  controlWindow;
    HWND                  debugWindow;

  friend void PAssertFunc(const char *);

// End Of File ///////////////////////////////////////////////////////////////
