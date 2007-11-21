/*
 * custom.h
 *
 * PWLib application header file for xmlrpcsrvr
 *
 * Customisable application configurationfor OEMs.
 *
 * Copyright 2002 Equivalence
 *
 * $Id$
 */

#include <ptclib/httpsvc.h>

enum {
  SkName, SkCompany, SkEMail,
  NumSecuredKeys
};


extern PHTTPServiceProcess::Info ProductInfo;


// End of File ///////////////////////////////////////////////////////////////
