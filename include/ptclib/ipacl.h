/*
 * ipacl.h
 *
 * PWLib application header file for rcsd
 *
 * Copyright 1998 Equivalence Pty. Ltd.
 *
 * $Log: ipacl.h,v $
 * Revision 1.1  1999/01/31 00:59:26  robertj
 * Added IP Access Control List class to PTLib Components
 *
 */

#ifndef _IPACL_H
#define _IPACL_H


#include <ptlib/sockets.h>


class PIpAccessControlEntry : public PObject
{
/* This class is a single IP access control specification.
 */

  PCLASSINFO(PIpAccessControlEntry, PObject)

  public:
    PIpAccessControlEntry(
      PIPSocket::Address addr,
      PIPSocket::Address msk,
      BOOL allow
    );
    PIpAccessControlEntry(
      const PString & description
    );
    /* Create a new IP access control specification. See the Parse() function
       for more details on the format of the <CODE>description</CODE>
       parameter.
     */

    PIpAccessControlEntry & operator=(
      const PString & pstr
    );
    PIpAccessControlEntry & operator=(
      const char * cstr
    );
    /* Set a new IP access control specification. See the Parse() function
       for more details on the format of the <CODE>pstr</CODE> and
       <CODE>cstr</CODE> parameters.
     */

    virtual Comparison Compare(
      const PObject & obj   // Object to compare against.
    ) const;
    /* Compare the two objects and return their relative rank.

       <H2>Returns:</H2>
       <CODE>LessThan</CODE>, <CODE>EqualTo</CODE> or <CODE>GreaterThan</CODE>
       according to the relative rank of the objects.
     */

    virtual void PrintOn(
      ostream &strm   // Stream to print the object into.
    ) const;
    /* Output the contents of the object to the stream. This outputs the same
       format as the AsString() function.
     */

    virtual void ReadFrom(
      istream &strm   // Stream to read the objects contents from.
    );
    /* Input the contents of the object from the stream. This expects the
       next space delimited entry in the stream to be as described in the
       Parse() function.
     */

    PString AsString() const;
    /* Convert the specification to a string, that can be processed by the
       Parse() function.

       <H2>Returns:</H2>
       PString representation of the entry.
     */

    BOOL IsValid();
    /* Check the internal fields of the specification for validity.

       <H2>Returns:</H2>
       TRUE if entry is valid.
     */

    BOOL Parse(
      const PString & description   // Description of the specification
    );
    /* Parse the description string into this IP access control specification.
       The string may be of several forms:
          n.n.n.n         Simple IP number, this has an implicit mask of
                          255.255.255.255
          n.n.            IP with trailing dot, assumes a mask equal to the
                          number of specified octets eg 10.1. is equivalent
                          to 10.1.0.0/255.255.0.0
          n.n.n.n/b       An IP network using b bits of mask, for example
                          10.1.0.0/14 is equivalent to 10.0.1.0/255.248.0.0
          n.n.n.n/m.m.m.m An IP network using the specified mask
          hostname        A specific host name, this has an implicit mask of
                          255.255.255.255
          .domain.dom     Matches an IP number whose cannonical name (found
                          using a reverse DNS lookup) ends with the specified
                          domain.

       <H2>Returns:</H2>
       TRUE if entry is valid.
     */


    BOOL Match(
      PIPSocket::Address & address    // Address to search for
    );
    /* Check to see if the specified IP address match any of the conditions
       specifed in the Parse() function for this entry.

       <H2>Returns:</H2>
       TRUE if entry can match the address.
     */

    BOOL IsAllowed() const { return allowed; }


  protected:
    PString            domain;
    PIPSocket::Address address;
    PIPSocket::Address mask;
    BOOL               allowed;
    BOOL               hidden;
};

PSORTED_LIST(PIpAccessControlList_base, PIpAccessControlEntry);


class PIpAccessControlList : public PIpAccessControlList_base
{
/* This class is a list of IP address mask specifications used to validate if
   an address may or may not be used in a connection.

   The list may be totally internal to the application, or may use system
   wide files commonly use under Linux (hosts.allow and hosts.deny file). These
   will be used regardless of the platform.

   When a search is done using IsAllowed() function, the first entry that
   matches the specified IP address is found, and its allow flag returned. The
   list sorted so that the most specific IP number specification is first and
   the broadest onse later. The entry with the value having a mask of zero,
   that is the match all entry, is always last.
 */

  PCLASSINFO(PIpAccessControlList, PIpAccessControlList_base)

  public:
    PIpAccessControlList();
    /* Create a new, empty, access control list.
     */

    BOOL LoadHostsAccess(
      const char * daemonName = NULL    // Name of "daemon" application
    );
    /* Load the system wide files commonly use under Linux (hosts.allow and
       hosts.deny file) for IP access. See the Linux man entries on these
       files for more information. Note, these files will be loaded regardless
       of the actual platform used. The directory returned by the
       PProcess::GetOSConfigDir() function is searched for the files.

       The <CODE>daemonName</CODE> parameter is used as the search argument in
       the hosts.allow/hosts.deny file. If this is NULL then the
       PProcess::GetName() function is used.

       <H2>Returns:</H2>
       TRUE if all the entries in the file were added, if any failed then
       FALSE is returned.
     */

    BOOL Load(
      PConfig & cfg   // Configuration file to load entries from.
    );
    /* Load entries in the list from the configuration file specified. This is
       equivalent to Load(cfg, "IP Access Control List").

       <H2>Returns:</H2>
       TRUE if all the entries in the file were added, if any failed then
       FALSE is returned.
     */

    BOOL Load(
      PConfig & cfg,            // Configuration file to load entries from.
      const PString & baseName  // Base name string for each entry in file.
    );
    /* Load entries in the list from the configuration file specified, using
       the base name for the array of configuration file values. The format of
       entries in the configuration file are suitable for use with the
       PHTTPConfig classes.

       <H2>Returns:</H2>
       TRUE if all the entries in the file were added, if any failed then
       FALSE is returned.
     */

    void Save(
      PConfig & cfg   // Configuration file to save entries to.
    );
    /* Save entries in the list to the configuration file specified. This is
       equivalent to Save(cfg, "IP Access Control List").
     */

    void Save(
      PConfig & cfg,            // Configuration file to save entries to.
      const PString & baseName  // Base name string for each entry in file.
    );
    /* Save entries in the list to the configuration file specified, using
       the base name for the array of configuration file values. The format of
       entries in the configuration file are suitable for use with the
       PHTTPConfig classes.
     */

    BOOL Add(
      const PString & description   // Description of the IP match parameters
    );
    BOOL Add(
      PIPSocket::Address address,   // IP network address
      PIPSocket::Address mask,      // Mask for IP network
      BOOL allow                    // Flag for if network is allowed or not
    );
    /* Add the specified entry into the list. See the PIpAccessControlEntry
       class for more details on the format of the <CODE>description</CODE>
       field.

       <H2>Returns:</H2>
       TRUE if the entries was successfully added.
     */

    BOOL Remove(
      const PString & description   // Description of the IP match parameters
    );
    BOOL Remove(
      PIPSocket::Address address,   // IP network address
      PIPSocket::Address mask       // Mask for IP network
    );
    /* Remove the specified entry into the list. See the PIpAccessControlEntry
       class for more details on the format of the <CODE>description</CODE>
       field.

       <H2>Returns:</H2>
       TRUE if the entries was successfully removed.
     */


    BOOL IsAllowed(
      PTCPSocket & socket           // Socket to test
    );
    BOOL IsAllowed(
      PIPSocket::Address address    // IP Address to test
    );
    /* Test the address/connection for if it is allowed within this access
       control list. If the <CODE>socket</CODE> form is used the peer address
       of the connection is tested.

       <H2>Returns:</H2>
       TRUE if the remote host address is allowed.
     */

  private:
    BOOL InternalLoadHostsAccess(const PString & daemon, const char * file, BOOL allow);
    BOOL InternalAddEntry(PIpAccessControlEntry & entry);
    BOOL InternalRemoveEntry(PIpAccessControlEntry & entry);
};


#endif  // _IPACL_H


// End of File ///////////////////////////////////////////////////////////////
