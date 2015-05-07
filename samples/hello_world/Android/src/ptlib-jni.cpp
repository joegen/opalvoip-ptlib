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
#include <ptlib/pluginmgr.h>

#include <jni.h>

class Pjstring
{
  JNIEnv     * m_jni;
  jstring      m_jstr;
  const char * m_cstr;

public:
  Pjstring(JNIEnv* jni, jstring str)
    : m_jni(jni)
    , m_jstr(str)
    , m_cstr(jni->GetStringUTFChars(str, 0))
  {
  }

  ~Pjstring()
  {
    Release();
  }

  void Release()
  {
    if (m_cstr != NULL) {
      m_jni->ReleaseStringUTFChars(m_jstr, m_cstr);
      m_cstr = NULL;
    }
  }

  Pjstring & operator=(jstring str)
  {
    Release();
    m_cstr = m_jni->GetStringUTFChars(m_jstr = str, 0);
  }

  operator const char *() const { return m_cstr; }
  operator PString() const { return m_cstr; }
};


extern "C" {

void 
Java_org_opalvoip_ptlib_hello_PTLibHello_initialisePTLib(JNIEnv* env,
                                                jobject thiz,
                                                jstring cfgDir,
                                                jstring tmpDir,
                                                jstring pluginDir)
{
  PLibraryProcess * process = new PLibraryProcess("Vox Lucida", "PTLibHello", 1,  0, PProcess::AlphaCode, 0, true);

  Pjstring tmp(env, tmpDir);
  static PConstString const logFile("/ptlib.log");
  PTRACE_INITIALISE(5, tmp + logFile);

  tmp = cfgDir;
  process->SetConfigurationPath(tmp);

  tmp = pluginDir;
  PPluginManager::AddPluginDirs(tmp);

  process->Startup();
}


jstring
Java_org_opalvoip_ptlib_hello_PTLibHello_getAppVersion( JNIEnv* env,
                                                  jobject thiz )
{
    return env->NewStringUTF(PProcess::Current().GetVersion());
}

jstring
Java_org_opalvoip_ptlib_hello_PTLibHello_getLibVersion( JNIEnv* env,
                                                  jobject thiz )
{
    return env->NewStringUTF(PProcess::GetLibVersion());
}

};
