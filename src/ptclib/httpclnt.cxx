/*
 * $Id: httpclnt.cxx,v 1.6 1997/03/28 04:40:46 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: httpclnt.cxx,v $
 * Revision 1.6  1997/03/28 04:40:46  robertj
 * Fixed bug in Post function doing wrong command.
 *
 * Revision 1.5  1997/03/18 22:04:03  robertj
 * Fix bug for binary POST commands.
 *
 * Revision 1.4  1996/12/21 01:26:21  robertj
 * Fixed bug in persistent connections when server closes socket during command.
 *
 * Revision 1.3  1996/12/12 09:24:44  robertj
 * Persistent connection support.
 *
 * Revision 1.2  1996/10/08 13:12:03  robertj
 * Fixed bug in HTTP/0.9 response, first 5 character not put back properly.
 *
 * Revision 1.1  1996/09/14 13:02:18  robertj
 * Initial revision
 *
 * Revision 1.37  1996/08/25 09:37:41  robertj
 * Added function to detect "local" host name.
 * Fixed printing of trailing '/' in empty URL, is distinction between with and without.
 *
 * Revision 1.36  1996/08/22 13:22:26  robertj
 * Fixed bug in print of URLs, extra @ signs.
 *
 * Revision 1.35  1996/08/19 13:42:40  robertj
 * Fixed errors in URL parsing and display.
 * Fixed "Forbidden" problem out of HTTP authorisation system.
 * Fixed authorisation so if have no user/password on basic authentication, does not require it.
 *
 * Revision 1.34  1996/07/27 04:13:47  robertj
 * Fixed use of HTTP proxy on non-persistent connections.
 *
 * Revision 1.33  1996/07/15 10:37:20  robertj
 * Improved proxy "self" detection (especially localhost).
 *
 * Revision 1.32  1996/06/28 13:20:24  robertj
 * Modified HTTPAuthority so gets PHTTPReqest (mainly for URL) passed in.
 * Moved HTTP form resource to another compilation module.
 * Fixed memory leak in POST command.
 *
 * Revision 1.31  1996/06/10 10:00:00  robertj
 * Added global function for query parameters parsing.
 *
 * Revision 1.30  1996/06/07 13:52:23  robertj
 * Added PUT to HTTP proxy FTP. Necessitating redisign of entity body processing.
 *
 * Revision 1.29  1996/06/05 12:33:04  robertj
 * Fixed bug in parsing URL with no path, is NOT absolute!
 *
 * Revision 1.28  1996/05/30 10:07:26  robertj
 * Fixed bug in version number checking of return code compatibility.
 *
 * Revision 1.27  1996/05/26 03:46:42  robertj
 * Compatibility to GNU 2.7.x
 *
 * Revision 1.26  1996/05/23 10:02:13  robertj
 * Added common function for GET and HEAD commands.
 * Fixed status codes to be the actual status code instead of sequential enum.
 * This fixed some problems with proxy pass through of status codes.
 * Fixed bug in URL parsing of username and passwords.
 *
 * Revision 1.19.1.1  1996/04/17 11:08:22  craigs
 * New version by craig pending confirmation by robert
 *
 * Revision 1.19  1996/04/05 01:46:30  robertj
 * Assured PSocket::Write always writes the number of bytes specified, no longer need write loops.
 * Added workaraound for NT Netscape Navigator bug with persistent connections.
 *
 * Revision 1.18  1996/03/31 09:05:07  robertj
 * HTTP 1.1 upgrade.
 *
 * Revision 1.17  1996/03/17 05:48:07  robertj
 * Fixed host name print out of URLs.
 * Added hit count to PHTTPResource.
 *
 * Revision 1.16  1996/03/16 05:00:26  robertj
 * Added ParseReponse() for splitting reponse line into code and info.
 * Added client side support for HTTP socket.
 * Added hooks for proxy support in HTTP socket.
 * Added translation type to TranslateString() to accommodate query variables.
 * Defaulted scheme field in URL to "http".
 * Inhibited output of port field on string conversion of URL according to scheme.
 *
 * Revision 1.15  1996/03/11 10:29:50  robertj
 * Fixed bug in help image HTML.
 *
 * Revision 1.14  1996/03/10 13:15:24  robertj
 * Redesign to make resources thread safe.
 *
 * Revision 1.13  1996/03/02 03:27:37  robertj
 * Added function to translate a string to a form suitable for inclusion in a URL.
 * Added radio button and selection boxes to HTTP form resource.
 * Fixed bug in URL parsing, losing first / if hostname specified.
 *
 * Revision 1.12  1996/02/25 11:14:24  robertj
 * Radio button support for forms.
 *
 * Revision 1.11  1996/02/25 03:10:34  robertj
 * Removed pass through HTTP resource.
 * Fixed PHTTPConfig resource to use correct name for config key.
 *
 * Revision 1.10  1996/02/19 13:48:28  robertj
 * Put multiple uses of literal strings into const variables.
 * Fixed URL parsing so that the unmangling of strings occurs correctly.
 * Moved nested classes from PHTTPForm.
 * Added overwrite option to AddResource().
 * Added get/set string to PHTTPString resource.
 *
 * Revision 1.9  1996/02/13 13:09:17  robertj
 * Added extra parameters to callback function in PHTTPResources, required
 *   by descendants to make informed decisions on data being loaded.
 *
 * Revision 1.8  1996/02/08 12:26:29  robertj
 * Redesign of resource callback mechanism.
 * Added new resource types for HTML data entry forms.
 *
 * Revision 1.7  1996/02/03 11:33:19  robertj
 * Changed RadCmd() so can distinguish between I/O error and unknown command.
 *
 * Revision 1.6  1996/02/03 11:11:49  robertj
 * Numerous bug fixes.
 * Added expiry date and ismodifiedsince support.
 *
 * Revision 1.5  1996/01/30 23:32:40  robertj
 * Added single .
 *
 * Revision 1.4  1996/01/28 14:19:09  robertj
 * Split HTML into separate source file.
 * Beginning of pass through resource type.
 * Changed PCharArray in OnLoadData to PString for convenience in mangling data.
 * Made PHTTPSpace return standard page on selection of partial path.
 *
 * Revision 1.3  1996/01/28 02:49:16  robertj
 * Further implementation.
 *
 * Revision 1.2  1996/01/26 02:24:30  robertj
 * Further implemetation.
 *
 * Revision 1.1  1996/01/23 13:04:32  robertj
 * Initial revision
 *
 */

#include <ptlib.h>
#include <sockets.h>
#include <http.h>
#include <ctype.h>


//////////////////////////////////////////////////////////////////////////////
// PHTTPClient

PHTTPClient::PHTTPClient()
{
}


int PHTTPClient::ExecuteCommand(Commands cmd,
                                const PString & url,
                                const PMIMEInfo & outMIME,
                                const PString & dataBody,
                                PMIMEInfo & replyMime)
{
  if (!WriteCommand(cmd, url, outMIME, dataBody))
    return -1;

  if (!ReadResponse(replyMime))
    return -1;

  return lastResponseCode;
}


BOOL PHTTPClient::WriteCommand(Commands cmd,
                               const PString & url,
                               const PMIMEInfo & outMIME,
                               const PString & dataBody)
{
  return PHTTP::WriteCommand(cmd, url & "HTTP/1.0") &&
         outMIME.Write(*this) &&
         Write(dataBody, dataBody.GetSize()-1);
}


BOOL PHTTPClient::ReadResponse(PMIMEInfo & replyMIME)
{
  PString http = ReadString(7);
  if (http.IsEmpty()) {
    lastResponseCode = -1;
    lastResponseInfo = GetErrorText();
    return FALSE;
  }

  UnRead(http);

  if (http.Left(5) != "HTTP/" && http != "\r\nHTTP/") {
    lastResponseCode = PHTTP::OK;
    lastResponseInfo = "HTTP/0.9";
    return TRUE;
  }

  if (http[0] == '\r' &&  http[1] == '\n')
    ReadString(2);

  if (PHTTP::ReadResponse())
    return replyMIME.Read(*this);

  return FALSE;
}


BOOL PHTTPClient::GetDocument(const PURL & url,
                              const PMIMEInfo & outMIME,
                              PMIMEInfo & replyMIME)
{
  return ExecuteCommand(GET, url.AsString(PURL::URIOnly), outMIME, PString(), replyMIME);
}


BOOL PHTTPClient::GetHeader(const PURL & url,
                            const PMIMEInfo & outMIME,
                            PMIMEInfo & replyMIME)
{
  return ExecuteCommand(HEAD, url.AsString(PURL::URIOnly), outMIME, PString(), replyMIME);
}


BOOL PHTTPClient::PostData(const PURL & url,
                           const PMIMEInfo & outMIME,
                           const PStringToString & data,
                           PMIMEInfo & replyMIME)
{
  PStringStream body;
  body << data;
  return ExecuteCommand(POST, url.AsString(PURL::URIOnly), outMIME, body, replyMIME);
}


// End Of File ///////////////////////////////////////////////////////////////
