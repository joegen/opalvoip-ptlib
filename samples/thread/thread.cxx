/*
 * thread.cxx
 *
 * Sample program to test PWLib threads.
 *
 * Portable Windows Library
 *
 * Copyright (c) 2001 Roger Hardiman
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
 * The Initial Developer of the Original Code is Roger Hardiman
 *
 * $Log: thread.cxx,v $
 * Revision 1.2  2001/09/27 08:22:48  rogerh
 * Doing a flush on cout does not work on Mac OS X. So you do not see any
 * results until the program stops. So replace the printing of the numbers with
 * good old printf and fflush.
 *
 * Revision 1.1  2001/09/21 09:18:28  rogerh
 * Add a thread test program which demonstrates thread, suspend and resume.
 *
 *
 */

/*
 * This sample program tests threads is PWLib. It creates two threads,
 * one which display the number '1' and one which displays the number '2'.
 * It also demonstrates starting a thread with Resume() and also using
 * Suspend() and Resume() to suspend a running thread.
 */

#include <ptlib.h>

/*
 * Thread #1 loops forever, displaying the number 1
 */
class MyThread1 : public PThread
{
  PCLASSINFO(MyThread1, PThread);
  public:
    MyThread1() : PThread(1000,AutoDeleteThread)
    {
      Resume(); // start running this thread as soon as the thread is created.
    }

    void Main() {
      while (1) {
        // Display the number 1, then sleep for a short time
        printf("1 ");
        fflush(stdout);
	usleep(10000);
      };
    };
};


/*
 * Thread #2 loops forever, displaying the number 2
 */
class MyThread2 : public PThread
{
  PCLASSINFO(MyThread2, PThread);
  public:
    MyThread2() : PThread(1000,AutoDeleteThread)
    {
    // This thread will not start automatically. We must call
    // Resume() after creating the thread.
    }

    void Main() {
      while (1) {
        // Display the number 2, then sleep for a short time
        printf("2 ");
        fflush(stdout);
	usleep(10000);
      };
    };
};


/*
 * The main program class
 */
class ThreadTest : public PProcess
{
  PCLASSINFO(ThreadTest, PProcess)
  public:
    void Main();
};

PCREATE_PROCESS(ThreadTest);

// The main program
void ThreadTest::Main()
{
  cout << "Thread Test Program" << endl;
  cout << "This program will display the following:" << endl;
  cout << "             3 seconds of 1 1 1 1 1..." << endl;
  cout << " followed by 3 seconds of 1 2 1 2 1 2 1 2 1 2..." << endl;
  cout << " followed by 3 seconds of 2 2 2 2 2..." << endl;
  cout << " followed by 3 seconds of 1 2 1 2 1 2 1 2 1 2..." << endl;
  cout << endl;
  cout << "It tests thread creation, suspend and resume functions." << endl;
  cout << endl;

  // Create the threads
  PThread * mythread1;
  PThread * mythread2;

  mythread1 = new MyThread1();
  mythread2 = new MyThread2();


  // Thread 1 should now be running, as there is a Resume() function
  // in the thread constructor.
  // Thread 2 should be suspended.
  // Sleep for three seconds. Only thread 1 will be running.
  // Display will show "1 1 1 1 1 1 1..."
  sleep(3);


  // Start the second thread.
  // Both threads should be running
  // Sleep for 3 seconds, allowing the threads to run.
  // Display will show "1 2 1 2 1 2 1 2 1 2..."
  mythread2->Resume();
  sleep(3);


  // Suspend thread 1.
  // Sleep for 3 seconds. Only thread 2 should be running.
  // Display will show "2 2 2 2 2 2 2..."
  mythread1->Suspend();
  sleep(3);


  // Resume thread 1.
  // Sleep for 3 seconds. Both threads should be running.
  // Display will show "1 2 1 2 1 2 1 2 1 2..."
  mythread1->Resume();
  sleep(3);
}

