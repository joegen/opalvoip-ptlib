/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <ptlib.h>
#include <ptlib/pprocess.h>

#include <jni.h>

class JniProcess : public PLibraryProcess {
public:
   JniProcess()
      : PLibraryProcess("Vox Lucida", "PTLibHello", 1,  0, AlphaCode, 0)
   {
   }
} myProcess;


extern "C" {

jstring
Java_com_ptlib_hello_PTLibHello_getVersion( JNIEnv* env,
                                                  jobject thiz )
{
    return env->NewStringUTF(myProcess.GetVersion());
}

};
