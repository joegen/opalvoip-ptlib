/*
 * $Id: indchan.h,v 1.1 1996/09/14 13:00:56 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: indchan.h,v $
 * Revision 1.1  1996/09/14 13:00:56  robertj
 * Initial revision
 *
 */

#ifndef _PINDIRECTCHANNEL
#define _PINDIRECTCHANNEL

#ifdef __GNUC__
#pragma interface
#endif


PDECLARE_CLASS(PIndirectChannel, PChannel)
/* This is a channel that operates indirectly through another channel(s). This
   allows for a protocol to operate through a "channel" mechanism and for its
   low level byte exchange (Read and Write) to operate via a completely
   different channel, eg TCP or Serial port etc.
 */

  public:
    PIndirectChannel();
    /* Create a new indirect channel without any channels to redirect to. If
       an attempt to read or write is made before Open() is called the the
       functions will assert.
     */

  ~PIndirectChannel();
  // Close the indirect channel, deleting read/write channels if desired.


  // Overrides from class PObject
    Comparison Compare(
      const PObject & obj   // Another indirect channel to compare against.
    ) const;
    /* Determine if the two objects refer to the same indirect channel. This
       actually compares the channel pointers.

       <H2>Returns:</H2>
       EqualTo if channel pointer identical.
     */


  // Overrides from class PChannel
    virtual PString GetName() const;
    /* Get the name of the channel. This is a combination of the channel
       pointers names (or simply the channel pointers name if the read and
       write channels are the same) or empty string if both null.
    
       <H2>Returns:</H2>
       string for the channel names.
     */

    virtual BOOL Close();
    /* Close the channel. This will detach itself from the read and write
       channels and delete both of them if they are auto delete.

       <H2>Returns:</H2>
       TRUE if the channel is closed.
     */

    virtual BOOL IsOpen() const;
    /* Determine if the channel is currently open and read and write operations
       can be executed on it. For example, in the <A>PFile</A> class it returns
       if the file is currently open.

       <H2>Returns:</H2>
       TRUE if the channel is open.
     */

    virtual BOOL Read(
      void * buf,   // Pointer to a block of memory to receive the read bytes.
      PINDEX len    // Maximum number of bytes to read into the buffer.
    );
    /* Low level read from the channel. This function may block until the
       requested number of characters were read or the read timeout was
       reached. The GetLastReadCount() function returns the actual number
       of bytes read.

       This will use the <CODE>readChannel</CODE> pointer to actually do the
       read. If <CODE>readChannel</CODE> is null the this asserts.

       The GetErrorCode() function should be consulted after Read() returns
       FALSE to determine what caused the failure.

       <H2>Returns:</H2>
       TRUE indicates that at least one character was read from the channel.
       FALSE means no bytes were read due to timeout or some other I/O error.
     */

    virtual BOOL Write(
      const void * buf, // Pointer to a block of memory to write.
      PINDEX len        // Number of bytes to write.
    );
    /* Low level write to the channel. This function will block until the
       requested number of characters are written or the write timeout is
       reached. The GetLastWriteCount() function returns the actual number
       of bytes written.

       This will use the <CODE>writeChannel</CODE> pointer to actually do the
       write. If <CODE>writeChannel</CODE> is null the this asserts.

       The GetErrorCode() function should be consulted after Write() returns
       FALSE to determine what caused the failure.

       <H2>Returns:</H2>
       TRUE if at least len bytes were written to the channel.
     */

    virtual BOOL Shutdown(
      ShutdownValue option
    );
    /* Close one or both of the data streams associated with a channel.

       The behavour here is to pass the shutdown on to its read and write
       channels.

       <H2>Returns:</H2>
       TRUE if the shutdown was successfully performed.
     */


    virtual PChannel * GetBaseReadChannel() const;
    /* This function returns the eventual base channel for reading of a series
       of indirect channels provided by descendents of <A>PIndirectChannel</A>.

       The behaviour for this function is to return "this".
       
       <H2>Returns:</H2>
       Pointer to base I/O channel for the indirect channel.
     */

    virtual PChannel * GetBaseWriteChannel() const;
    /* This function returns the eventual base channel for writing of a series
       of indirect channels provided by descendents of <A>PIndirectChannel</A>.

       The behaviour for this function is to return "this".
       
       <H2>Returns:</H2>
       Pointer to base I/O channel for the indirect channel.
     */


  // New member functions
    BOOL Open(
      PChannel & channel
        // Channel to be used for both read and write operations.
    );
    BOOL Open(
      PChannel * channel,
        // Channel to be used for both read and write operations.
      BOOL autoDelete = TRUE   // Automatically delete the channel
    );
    BOOL Open(
      PChannel * readChannel,
        // Channel to be used for both read operations.
      PChannel * writeChannel,
        // Channel to be used for both write operations.
      BOOL autoDeleteRead = TRUE,  // Automatically delete the read channel
      BOOL autoDeleteWrite = TRUE  // Automatically delete the write channel
    );
    /* Set the channel for both read and write operations. This then checks
       that they are open and then calls the OnOpen() virtual function. If
       it in turn returns TRUE then the Open() function returns success.

       <H2>Returns:</H2>
       TRUE if both channels are set, open and OnOpen() returns TRUE.
     */

    PChannel * GetReadChannel() const;
    /* Get the channel used for read operations.
    
       <H2>Returns:</H2>
       pointer to the read channel.
     */

    BOOL SetReadChannel(
      PChannel * channel,
        // Channel to be used for both read operations.
      BOOL autoDelete = TRUE   // Automatically delete the channel
    );
    /* Set the channel for read operations.

       <H2>Returns:</H2>
       Returns TRUE if both channels are set and are both open.
     */

    PChannel * GetWriteChannel() const;
    /* Get the channel used for write operations.
    
       <H2>Returns:</H2>
       pointer to the write channel.
     */

    BOOL SetWriteChannel(
      PChannel * channel,
        // Channel to be used for both write operations.
      BOOL autoDelete = TRUE   // Automatically delete the channel
    );
    /* Set the channel for read operations.

       <H2>Returns:</H2>
       Returns TRUE if both channels are set and are both open.
    */


  protected:
    virtual BOOL OnOpen();
    /* This callback is executed when the Open() function is called with
       open channels. It may be used by descendent channels to do any
       handshaking required by the protocol that channel embodies.

       The default behaviour is to simply return TRUE.

       <H2>Returns:</H2>
       Returns TRUE if the protocol handshaking is successful.
     */


    // Member variables
    PChannel * readChannel;
    // Channel for read operations.

    BOOL readAutoDelete;
    // Automatically delete read channel on destruction.

    PChannel * writeChannel;
    // Channel for write operations.

    BOOL writeAutoDelete;
    // Automatically delete write channel on destruction.
};


#endif // _PINDIRECTCHANNEL
