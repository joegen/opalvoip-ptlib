/*
 * $Id: pstring.h,v 1.6 1995/01/15 04:50:20 robertj Exp $
 *
 * Portable Windows Library
 *
 * Container Classes Interface Declarations
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 *
 * $Log: pstring.h,v $
 * Revision 1.6  1995/01/15 04:50:20  robertj
 * Added inlines on friend functions, required by GNU compiler.
 *
 * Revision 1.5  1995/01/10  11:43:41  robertj
 * Removed PString parameter in stdarg function for GNU C++ compatibility.
 *
 * Revision 1.4  1995/01/09  12:33:44  robertj
 * Removed unnecesary return value from I/O functions.
 * Changed function names due to Mac port.
 *
 * Revision 1.3  1994/12/21  11:53:21  robertj
 * Documentation and variable normalisation.
 *
 * Revision 1.2  1994/12/12  13:13:13  robertj
 * Fixed bugs in PString mods just made.
 *
 * Revision 1.1  1994/12/12  09:59:37  robertj
 * Initial revision
 *
 */

#ifdef __GNUC__
#pragma interface
#endif


///////////////////////////////////////////////////////////////////////////////
// PString class

class PStringArray;

#ifdef PHAS_UNICODE
#define PSTRING_ANCESTOR_CLASS PWordArray
#else
#define PSTRING_ANCESTOR_CLASS PCharArray
#endif

PDECLARE_CLASS(PString, PSTRING_ANCESTOR_CLASS)
/* The character string class. It supports a wealth of additional functions
   for string processing and conversion. Operators are provided so that
   strings can virtually be treated as a basic type.

   The $B$PSTRING_ANCESTOR_CLASS$B$ is dependent on whether UNICODE support is
   selected. The $U$entire library and application$U$ must be compiled with or
   without UNICODE or undefined results will occur. PSTRING_ANCESTOR_CLASS
   macro is normally set to PCharArray.

   Note that the array is a '\0' terminated string as in C strings. Thus the
   memory allocated, and the length of the string may be different values.
 */

  public:
    PString();
    /* Construct an empty string. This will have one character in it which is
       the '\0' character.
     */

    PString(
      const PString & str  // String to create new reference to.
    );
    /* Create a new reference to the specified string. The string memory is not
       copied, only the pointer to the data.
     */

    PString(
      const char * cstr // Standard '\0' terminated C string.
    );
    /* Create a string from the C string array. This is most commonly used with
       a literal string, eg "hello". A new memory block is allocated of a size
       sufficient to take the length of the string and its terminating
       '\0' character.

       If UNICODE is used then each char from the char pointer is mapped to a
       single UNICODE character.
     */

    PString(
      const char * cstr,  // Pointer to a string of characters.
      PINDEX len          // Length of the string in bytes.
    );
    /* Create a string from the char array. A new memory block is allocated of
       a size equal to $B$len$B$ plus one which is sufficient to take the
       string and a terminating '\0' character.

       If UNICODE is used then each char from the char pointer is mapped to a
       single UNICODE character.

       Note that this function will allow a string with embedded '\0'
       characters to be created, but most of the functions here will be unable
       to access characters beyond the first '\0'. Furthermore, if the
       $B$MakeMinimumSize()$B$ function is called, all data beyond that first
       '\0' character will be lost.
     */

    PString(char c);
    /* Create a string from the single character. This is most commonly used
       as a type conversion constructor when a literal character, eg 'A' is
       used in a string expression. A new memory block is allocated of two
       characters to take the char and its terminating '\0' character.

       If UNICODE is used then the char is mapped to a single UNICODE
       character.
     */

    enum ConversionType {
      Pascal,   // Data is a length byte followed by characters.
      Basic,    // Data is two length bytes followed by characters.
      Literal,  // Data is C language style string with \ escape codes.
      Signed,   // Convert a signed integer to a string.
      Unsigned, // Convert an unsigned integer to a string.
      Decimal,  // Convert a real number to a string in decimal format.
      Exponent, // Convert a real number to a string in exponent format.
      Printf,   // Formatted output, sprintf() style function.
      NumConversionTypes
    };
    /* Type of conversion to make in the conversion constructors.
     */

    PString(
      ConversionType type,  // Type of data source for conversion.
      const char * str,     // String to convert.
      ...                   // Extra parameters for $B$sprintf()$B$ call.
    );
    PString(
      ConversionType type,  // Type of data source for conversion.
      long value,           // Integer value to convert.
      unsigned base = 10    // Number base to use for the integer conversion.
    );
    PString(
      ConversionType type,  // Type of data source for conversion.
      double value,         // Floating point value to convert.
      unsigned places       // Number of decimals in real number output.
    );
    /* Contruct a new string converting from the spcified data source into
       a string array.
     */


  // Overrides from class PObject
    virtual PObject * Clone() const;
    /* Make a complete duplicate of the string. Note that the data in the
       array of characters is duplicated as well and the new object is a
       unique reference to that data.
     */

    virtual Comparison Compare(
      const PObject & obj   // Other PString to compare against.
    ) const;
    /* Get the relative rank of the two strings. The system standard function,
       eg strcmp(), is used.

       Returns: comparison of the two objects, $B$EqualTo$B$ for same,
                $B$LessThan$B$ for $B$obj$B$ logically less than the object
                and $B$GreaterThan$B$ for $B$obj$B$ logically greater than
                the object.
     */

    virtual void PrintOn(
      ostream & strm  // I/O stream to output to.
    ) const;
    /* Output the string to the specified stream.

       Returns: stream that the string was output to.
     */

    virtual void ReadFrom(
      istream & strm  // I/O stream to input from.
    );
    /* Input the string from the specified stream. This will read all
       characters until a end of line is reached. The end of line itself is
       $U$not$U$ placed in the string, however it $U$is$U$ removed from the
       stream.

       Returns: stream that the string was input from.
     */

    virtual PINDEX HashFunction() const;
    /* Calculate a hash value for use in sets and dictionaries.
    
       The hash function for strings will produce a value based on the sum of
       the first three characters of the string. This is a fairly basic
       function and make no assumptions about the string contents. A user may
       descend from PString and override the hash function if they can take
       advantage of the types of strings being used, eg if all strings start
       with the letter 'A' followed by 'B or 'C' then the current hash function
       will not perform very well.

       Returns: hash value for string.
     */


  // Overrides from class PContainer
    virtual BOOL IsEmpty() const;
    /* Determine if the string is empty. This is semantically slightly
       different from the usual container $B$IsEmpty()$B$ function. It does not
       test for $B$GetSize()$B$ equal to zero, it tests for $B$GetLength()$B$
       equal to zero.

       Returns: TRUE if no non-null characters in string.
     */


  // New functions for class
    BOOL MakeMinimumSize();
    /* Set the actual memory block array size to the minimum required to hold
       the current string contents.
       
       Note that this function will break the current instance from multiple
       references to the string. A new string buffer is allocated and the data
       from the old string buffer copied to it.

       Returns: TRUE if new memory block successfully allocated.
     */

    PINDEX GetLength() const;
    /* Determine the length of the null terminated string. This is different
       from $B$GetSize()$B$ which returns the amount of memory allocated to the
       string. This is often, though no necessarily, one larger than the
       length of the string.
       
       Returns: length of the null terminated string.
     */

    PString & operator=(
      const PString & str  // New string to assign.
    );
    /* Assign the string to the current object. The current instance then
       becomes another reference to the same string in the $B$str$B$ parameter.
       
       Returns: reference to the current PString object.
     */

    PString & operator=(
      const char * cstr  // C string to assign.
    );
    /* Assign the C string to the current object. The current instance then
       becomes a unique reference to a copy of the $B$cstr$B$ parameter. The
       $B$cstr$B$ parameter is typically a literal string, eg:

          $F$myStr = "fred";$F$
       
       Returns: reference to the current PString object.
     */

    PString operator+(
      const PString & str   // String to concatenate.
    ) const;
    /* Concatenate two strings to produce a third. The original strings are
       not modified, an entirely new unique reference to a string is created.
       
       Returns: new string with concatenation of the object and parameter.
     */

    PString operator+(
      const char * cstr  // C string to concatenate.
    ) const;
    /* Concatenate a C string to a PString to produce a third. The original
       string is not modified, an entirely new unique reference to a string
       is created. The $B$cstr$B$ parameter is typically a literal string, eg:

          $F$myStr = aStr + "fred";$F$

       Returns: new string with concatenation of the object and parameter.
     */

    PString operator+(
      char c   // Character to concatenate.
    ) const;
    /* Concatenate a single character to a PString to produce a third. The
       original string is not modified, an entirely new unique reference to a
       string is created. The $B$c$B$ parameter is typically a literal, eg:

          $F$myStr = aStr + '!';$F$

       Returns: new string with concatenation of the object and parameter.
     */

    PINLINE friend PString operator+(
      const char * cstr,    // C string to be concatenated to.
      const PString & str   // String to concatenate.
    );
    /* Concatenate a PString to a C string to produce a third. The original
       string is not modified, an entirely new unique reference to a string
       is created. The $B$cstr$B$ parameter is typically a literal string, eg:

          $F$myStr = "fred" + aStr;$F$

       Returns: new string with concatenation of the object and parameter.
     */

    PINLINE friend PString operator+(
      char  c,              // Character to be concatenated to.
      const PString & str   // String to concatenate.
    );
    /* Concatenate a PString to a single character to produce a third. The
       original string is not modified, an entirely new unique reference to a
       string is created. The $B$c$B$ parameter is typically a literal, eg:

          $F$myStr = '!' + aStr;$F$

       Returns: new string with concatenation of the object and parameter.
     */

    PString & operator+=(
      const PString & str   // String to concatenate.
    );
    /* Concatenate a string to another string, modifiying that string.

       Returns: reference to string that was concatenated to.
     */

    PString & operator+=(
      const char * cstr  // C string to concatenate.
    );
    /* Concatenate a C string to a PString, modifiying that string. The
       $B$cstr$B$ parameter is typically a literal string, eg:

          $F$myStr += "fred";$F$

       Returns: reference to string that was concatenated to.
     */


    BOOL operator==(
      const PObject & str  // PString object to compare against.
    ) const;
    /* Compare two strings using the $B$Compare()$B$ function. This is
       identical to the $H$PObject class function but is necessary due to
       other overloaded versions.

       Returns: TRUE if equal.
     */

    BOOL operator!=(
      const PObject & str  // PString object to compare against.
    ) const;
    /* Compare two strings using the $B$Compare()$B$ function. This is
       identical to the $H$PObject class function but is necessary due to
       other overloaded versions.

       Returns: TRUE if not equal.
     */

    BOOL operator<(
      const PObject & str  // PString object to compare against.
    ) const;
    /* Compare two strings using the $B$Compare()$B$ function. This is
       identical to the $H$PObject class function but is necessary due to
       other overloaded versions.

       Returns: TRUE if less than.
     */

    BOOL operator>(
      const PObject & str  // PString object to compare against.
    ) const;
    /* Compare two strings using the $B$Compare()$B$ function. This is
       identical to the $H$PObject class function but is necessary due to
       other overloaded versions.

       Returns: TRUE if greater than.
     */

    BOOL operator<=(
      const PObject & str  // PString object to compare against.
    ) const;
    /* Compare two strings using the $B$Compare()$B$ function. This is
       identical to the $H$PObject class function but is necessary due to
       other overloaded versions.

       Returns: TRUE if less than or equal.
     */

    BOOL operator>=(
      const PObject & str  // PString object to compare against.
    ) const;
    /* Compare two strings using the $B$Compare()$B$ function. This is
       identical to the $H$PObject class function but is necessary due to
       other overloaded versions.

       Returns: TRUE if greater than or equal.
     */


    BOOL operator==(
      const char * cstr  // C string to compare against.
    ) const;
    /* Compare a PString to a C string using the $B$Compare()$B$ function. The
       $B$cstr$B$ parameter is typically a literal string, eg:

          $F$if (myStr == "fred")$F$

       Returns: TRUE if equal.
     */

    BOOL operator!=(
      const char * cstr  // C string to compare against.
    ) const;
    /* Compare a PString to a C string using the $B$Compare()$B$ function. The
       $B$cstr$B$ parameter is typically a literal string, eg:

          $F$if (myStr != "fred")$F$

       Returns: TRUE if not equal.
     */

    BOOL operator<(
      const char * cstr  // C string to compare against.
    ) const;
    /* Compare a PString to a C string using the $B$Compare()$B$ function. The
       $B$cstr$B$ parameter is typically a literal string, eg:

          $F$if (myStr < "fred")$F$

       Returns: TRUE if less than.
     */

    BOOL operator>(
      const char * cstr  // C string to compare against.
    ) const;
    /* Compare a PString to a C string using the $B$Compare()$B$ function. The
       $B$cstr$B$ parameter is typically a literal string, eg:

          $F$if (myStr > "fred")$F$

       Returns: TRUE if greater than.
     */

    BOOL operator<=(
      const char * cstr  // C string to compare against.
    ) const;
    /* Compare a PString to a C string using the $B$Compare()$B$ function. The
       $B$cstr$B$ parameter is typically a literal string, eg:

          $F$if (myStr <= "fred")$F$

       Returns: TRUE if less than or equal.
     */

    BOOL operator>=(
      const char * cstr  // C string to compare against.
    ) const;
    /* Compare a PString to a C string using the $B$Compare()$B$ function. The
       $B$cstr$B$ parameter is typically a literal string, eg:

          $F$if (myStr >= "fred")$F$

       Returns: TRUE if greater than or equal.
     */


    virtual PINDEX Find(
      char ch,              // Character to search for in string.
      PINDEX offset = 0     // Offset into string to begin search.
    ) const;
    virtual PINDEX Find(
      const PString & str,  // String to search for in string.
      PINDEX offset = 0     // Offset into string to begin search.
    ) const;
    virtual PINDEX Find(
      const char * cstr,    // C string to search for in string.
      PINDEX offset = 0     // Offset into string to begin search.
    ) const;
    /* Locate the position within the string of the character or substring. The
       search will begin at the character offset provided.
       
       If $B$offset$B$ is beyond the length of the string, then the function
       will always return P_MAX_INDEX.
       
       The matching will be for identical character or string. If a search
       ignoring case is required then the string should be converted to a
       $H$PCaselessString before the search is made.

       Returns: position of character or substring in the string, or
                P_MAX_INDEX if the character or substring is not in the string.
     */

    virtual PINDEX FindLast(
      char ch,                     // Character to search for in string.
      PINDEX offset = P_MAX_INDEX  // Offset into string to begin search.
    ) const;
    virtual PINDEX FindLast(
      const PString & str,         // String to search for in string.
      PINDEX offset = P_MAX_INDEX  // Offset into string to begin search.
    ) const;
    virtual PINDEX FindLast(
      const char * cstr,           // C string to search for in string.
      PINDEX offset = P_MAX_INDEX  // Offset into string to begin search.
    ) const;
    /* Locate the position within the string of the last matching character or
       substring. The search will begin at the character offset provided,
       moving backward through the string.

       If $B$offset$B$ is beyond the length of the string, then the search
       begins at the end of the string. If $B$offset$B$ is zero then the
       function always returns P_MAX_INDEX.

       The matching will be for identical character or string. If a search
       ignoring case is required then the string should be converted to a
       $H$PCaselessString before the search is made.

       Returns: position of character or substring in the string, or
                P_MAX_INDEX if the character or substring is not in the string.
     */

    virtual PINDEX FindOneOf(
      const PString & set,  // String of characters to search for in string.
      PINDEX offset = 0     // Offset into string to begin search.
    ) const;
    virtual PINDEX FindOneOf(
      const char * cset,    // C string of characters to search for in string.
      PINDEX offset = 0     // Offset into string to begin search.
    ) const;
    /* Locate the position within the string of one of the characters in the
       set. The search will begin at the character offset provided.

       If $B$offset$B$ is beyond the length of the string, then the function
       will always return P_MAX_INDEX.
       
       The matching will be for identical character or string. If a search
       ignoring case is required then the string should be converted to a
       $H$PCaselessString before the search is made.

       Returns: position of character in the string, or P_MAX_INDEX if no
                characters from the set are in the string.
     */


    void Insert(
      const PString & str,  // Substring to insert.
      PINDEX pos            // Position in string to insert the substring.
    );
    void Insert(
      const char * cstr,    // Substring to insert.
      PINDEX pos            // Position in string to insert the substring.
    );
    /* Insert the substring into the string.

       Note that this function will break the current instance from multiple
       references to the string. A new string buffer is allocated and the data
       from the old string buffer copied to it.
     */

    void Delete(
      PINDEX start,   // Position in string to remove.
      PINDEX len      // Number of characters to delete.
    );
    /* Remove the substring from the string.

       Note that this function will break the current instance from multiple
       references to the string. A new string buffer is allocated and the data
       from the old string buffer copied to it.
     */


    PString operator()(
      PINDEX start,  // Starting position of the substring.
      PINDEX end     // Ending position of the substring.
    ) const;
    /* Extract a portion of the string into a new string. The original string
       is not changed and a new unique reference to a string is returned.
       
       The substring is returned inclusive of the characters at the $B$start$B$
       and $B$end$B$ positions.
       
       If the $B$end$B$ position is greater than the length of the string then
       all characters from the $B$start$B$ up to the end of the string are
       returned.

       If $B$start$B$ is greater than the length of the string or $B$end$B$ is
       before $B$start$B$ then an empty string is returned.

       Returns: substring of the source string.
     */

    PString Left(
      PINDEX len   // Number of characters to extract.
    ) const;
    /* Extract a portion of the string into a new string. The original string
       is not changed and a new unique reference to a string is returned.
       
       A substring from the beginning of the string for the number of
       characters specified is extracted.
       
       If $B$len$B$ is greater than the length of the string then all
       characters to the end of the string are returned.

       If $B$len$B$ is zero then an empty string is returned.

       Returns: substring of the source string.
     */

    PString Right(
      PINDEX len   // Number of characters to extract.
    ) const;
    /* Extract a portion of the string into a new string. The original string
       is not changed and a new unique reference to a string is returned.
       
       A substring from the end of the string for the number of characters
       specified is extracted.
       
       If $B$len$B$ is greater than the length of the string then all
       characters to the beginning of the string are returned.

       If $B$len$B$ is zero then an empty string is returned.

       Returns: substring of the source string.
     */

    PString Mid(
      PINDEX start,             // Starting position of the substring.
      PINDEX len = P_MAX_INDEX  // Number of characters to extract.
    ) const;
    /* Extract a portion of the string into a new string. The original string
       is not changed and a new unique reference to a string is returned.
       
       A substring from the $B$start$B$ position for the number of characters
       specified is extracted.
       
       If $B$len$B$ is greater than the length of the string from the
       $B$start$B$ position then all characters to the end of the string are
       returned.

       If $B$start$B$ is greater than the length of the string or $B$len$B$ is
       zero then an empty string is returned.

       Returns: substring of the source string.
     */


    PString LeftTrim() const;
    /* Create a string consisting of all characters from the source string
       except all spaces at the beginning of the string. The original string
       is not changed and a new unique reference to a string is returned.
       
       Returns: string with leading spaces removed.
     */

    PString RightTrim() const;
    /* Create a string consisting of all characters from the source string
       except all spaces at the end of the string. The original string is not
       changed and a new unique reference to a string is returned.
       
       Returns: string with trailing spaces removed.
     */

    PString Trim() const;
    /* Create a string consisting of all characters from the source string
       except all spaces at the beginning and end of the string. The original
       string is not changed and a new unique reference to a string is
       returned.
       
       Returns: string with leading and trailing spaces removed.
     */


    PString ToLower() const;
    /* Create a string consisting of all characters from the source string
       with all upper case letters converted to lower case. The original
       string is not changed and a new unique reference to a string is
       returned.
       
       Returns: string with upper case converted to lower case.
     */

    PString ToUpper() const;
    /* Create a string consisting of all characters from the source string
       with all lower case letters converted to upper case. The original
       string is not changed and a new unique reference to a string is
       returned.
       
       Returns: string with lower case converted to upper case.
     */


    PString & sprintf(
      const char * cfmt,    // C string for output format.
      ...                   // Extra parameters for $B$sprintf()$B$ call.
    );
    /* Concatenate a formatted output to the string. This is identical to the
       standard C library $B$sprintf()$B$ function, but appends its output
       to the string.
       
       This function makes the assumption that there is less the 1000
       characters of formatted output. The function will assert if this occurs.

       Note that this function will break the current instance from multiple
       references to the string. A new string buffer is allocated and the data
       from the old string buffer copied to it.
       
       Returns: reference to the current string object.
     */

    friend PString psprintf(
      const char * cfmt,    // C string for output format.
      ...                   // Extra parameters for $B$sprintf()$B$ call.
    );
    /* Produce formatted output as a string. This is identical to the standard
       C library $B$sprintf()$B$ function, but sends its output to a PString.

       This function makes the assumption that there is less the 1000
       characters of formatted output. The function will assert if this occurs.

       Note that this function will break the current instance from multiple
       references to the string. A new string buffer is allocated and the data
       from the old string buffer copied to it.
       
       Returns: reference to the current string object.
     */

    PString & vsprintf(
      const char * cfmt,    // C string for output format.
      va_list args          // Extra parameters for $B$sprintf()$B$ call.
    );
    PString & vsprintf(
      const PString & fmt,  // String for output format.
      va_list args          // Extra parameters for $B$sprintf()$B$ call.
    );
    /* Concatenate a formatted output to the string. This is identical to the
       standard C library $B$vsprintf()$B$ function, but appends its output
       to the string.

       This function makes the assumption that there is less the 1000
       characters of formatted output. The function will assert if this occurs.

       Note that this function will break the current instance from multiple
       references to the string. A new string buffer is allocated and the data
       from the old string buffer copied to it.
       
       Returns: reference to the current string object.
     */

    friend PString pvsprintf(
      const char * cfmt,    // C string for output format.
      va_list args          // Extra parameters for $B$sprintf()$B$ call.
    );
    PINLINE friend PString pvsprintf(
      const PString & fmt,  // String for output format.
      va_list args          // Extra parameters for $B$sprintf()$B$ call.
    );
    /* Produce formatted output as a string. This is identical to the standard
       C library $B$vsprintf()$B$ function, but sends its output to a PString.

       This function makes the assumption that there is less the 1000
       characters of formatted output. The function will assert if this occurs.

       Note that this function will break the current instance from multiple
       references to the string. A new string buffer is allocated and the data
       from the old string buffer copied to it.
       
       Returns: reference to the current string object.
     */


    long AsInteger(
      unsigned base = 10    // Number base to convert the string in.
    ) const;
    /* Convert the string to an integer value using the specified number base.
       All characters up to the first illegal character for the number base are
       converted. Case is not significant for bases greater than 10.

       The number base may only be from 2 to 36 and the function will assert
       if it is not in this range.

       This function uses the standard C library $B$strtol()$B$ function.

       Returns: integer value for the string.
     */

    double AsReal() const;
    /* Convert the string to a floating point number. This number may be in
       decimal or exponential form. All characters up to the first illegal
       character for a floting point number are converted.

       This function uses the standard C library $B$strtod()$B$ function.

       Returns: floating point value for the string.
     */
     

    PStringArray Tokenise(
      const PString & separators,
        // A string for the set of separator characters that delimit tokens.
      BOOL onePerSeparator = TRUE
        // Flag for if there are empty tokens between consecutive separators.
    ) const;
    PStringArray Tokenise(
      const char * cseparators,
        // A C string for the set of separator characters that delimit tokens.
      BOOL onePerSeparator = TRUE
        // Flag for if there are empty tokens between consecutive separators.
    ) const;
    /* Split the string into an array of substrings delimited by characters
       from the specified set.
       
       There are two options for the tokenisation, the first is where the
       $B$onePerSeparator$B$ is TRUE. This form will produce a token for each
       delimiter found in the set. Thus the string ",two,three,,five" would be
       split into 5 substrings; "", "two", "three", "" and "five".
       
       The second form where $B$onePerSeparator$B$ is FALSE is used where
       consecutive delimiters do not constitute a empty token. In this case
       the string "  a list  of words  " would be split into 4 substrings;
       "a", "list", "of" and "words".

       There is an important distinction when there are delimiters at the
       beginning or end of the source string. In the first case there will be
       empty strings at the end of the array and in the second the delimiters
       are ignored.

       Returns: an array of substring for each token in the string.
     */

    PStringArray Lines() const;
    /* Split the string into individual lines. The line delimiters may be a
       carriage return ('\r'), a line feed ('\n') or a carriage return and
       line feed pair ("\r\n"). A line feed and carriage return pair ("\n\r")
       would yield a blank line. between the characters.

       The $B$Tokenise()$B$ function should not be used to split a string into
       lines as a "\r\n" pair consitutes a single line ending. The
       $B$Tokenise()$B$ function would produce a blank line in between them.

       Returns: string array with a substring for each line in the string.
     */


    PBYTEArray ToPascal() const;
    /* Convert a standard null terminated string to a "pascal" style string.
       This consists of a songle byte for the length of the string and then
       the string characters following it.
       
       This function will assert if the string is greater than 255 characters
       in length.

       Returns: byte array containing the "pascal" style string.
     */

    PString ToLiteral() const;
    /* Convert the string to C literal string format. This will convert non
       printable characters to the \nnn form or for standard control characters
       such as line feed, to \n form. Any '"' characters are also escaped with
       a \ character and the entire string is enclosed in '"' characters.
       
       Returns: string converted to a C language literal form.
     */

#ifndef PHAS_UNICODE
    operator const unsigned char *() const;
    /* Get the internal buffer as a pointer to unsigned characters. The
       standard "operator const char *" function is provided by the
       $H$PCharArray ancestor class.

       Returns: pointer to character buffer.
     */
#endif


  protected:
    virtual Comparison InternalCompare(
      const char * cstr   // C string to compare against.
    ) const;
    /* Internal function to compare the current string value against the
       specified C string. This is a wrapper araound the standard C libary
       function $B$strcmp()$B$.

       Returns: relative rank of the two strings.
     */

    PString(int dummy, const PString * str);
};


//////////////////////////////////////////////////////////////////////////////

PDECLARE_CLASS(PCaselessString, PString)
/* This class is a variation of a string that ignores case. Thus in all
   standard comparison ("==", "<" etc) and search ($B$Find()$B$ etc) functions
   the case of the characters and strings is ignored.
   
   The characters in the string still maintain their case. Only the comparison
   oeprations are affected. So printing etc will still display the string as
   entered.
 */
 
  public:
    PCaselessString();
    /* Create a new, empty, caseless string.
     */

    PCaselessString(
      const char * cstr   // C string to initialise the caseless string from.
    );
    /* Create a new caseless string, initialising it to the characters in the
       C string provided.
     */

    PCaselessString(
      const PString & str  // String to initialise the caseless string from.
    );
    /* Create a caseless string, with a reference to the characters in the
       normal $H$PString provided. A PCaselessString may also be provided to
       this constructor.
     */


    PCaselessString & operator=(const PString & str);
    /* Set the current instance to reference the same string as the $B$str$B$
       parameter. The previous reference is decremented and if no more
       references to the string are present, the string buffer is released. A
       PCaselessString may also be provided to this operator.
     */


  // Overrides from class PString
    virtual PINDEX Find(
      char ch,              // Character to search for in string.
      PINDEX offset = 0     // Offset into string to begin search.
    ) const;
    virtual PINDEX Find(
      const PString & str,  // String to search for in string.
      PINDEX offset = 0     // Offset into string to begin search.
    ) const;
    virtual PINDEX Find(
      const char * cstr,    // C string to search for in string.
      PINDEX offset = 0     // Offset into string to begin search.
    ) const;
    /* Locate the position within the string of the character or substring. The
       search will begin at the character offset provided. The case of either
       string or character is ignored.
       
       If $B$offset$B$ is beyond the length of the string, then the function
       will always return P_MAX_INDEX.
       
       Returns: position of character or substring in the string, or
                P_MAX_INDEX if the character or substring is not in the string.
     */

    virtual PINDEX FindLast(
      char ch,                     // Character to search for in string.
      PINDEX offset = P_MAX_INDEX  // Offset into string to begin search.
    ) const;
    virtual PINDEX FindLast(
      const PString & str,         // String to search for in string.
      PINDEX offset = P_MAX_INDEX  // Offset into string to begin search.
    ) const;
    virtual PINDEX FindLast(
      const char * cstr,           // C string to search for in string.
      PINDEX offset = P_MAX_INDEX  // Offset into string to begin search.
    ) const;
    /* Locate the position within the string of the last matching character or
       substring. The search will begin at the character offset provided,
       moving backward through the string. The case of either string or
       character is ignored.

       If $B$offset$B$ is beyond the length of the string, then the search
       begins at the end of the string. If $B$offset$B$ is zero then the
       function always returns P_MAX_INDEX.

       Returns: position of character or substring in the string, or
                P_MAX_INDEX if the character or substring is not in the string.
     */

    virtual PINDEX FindOneOf(
      const PString & set,  // String of characters to search for in string.
      PINDEX offset = 0     // Offset into string to begin search.
    ) const;
    virtual PINDEX FindOneOf(
      const char * cset,    // C string of characters to search for in string.
      PINDEX offset = 0     // Offset into string to begin search.
    ) const;
    /* Locate the position within the string of one of the characters in the
       set. The search will begin at the character offset provided. The case of
       either string or character set is ignored.

       If $B$offset$B$ is beyond the length of the string, then the function
       will always return P_MAX_INDEX.
       
       Returns: position of character in the string, or P_MAX_INDEX if no
                characters from the set are in the string.
     */


  protected:
    virtual Comparison InternalCompare(
      const char * cstr   // C string to compare against.
    ) const;
    /* Internal function to compare the current string value against the
       specified C string. This is a wrapper araound the standard C libary
       function $B$stricmp()$B$ or $B$strcasecmp()$B$.

       Returns: relative rank of the two strings.
     */

    PCaselessString(int dummy, const PCaselessString * str);
};


//////////////////////////////////////////////////////////////////////////////

class PStringStream;

PCLASS PStringStream : public PString, public iostream {
/* This class is a standard C++ stream class descendent for reading or writing
   streamed data to or from a $H$PString class.
   
   All of the standard stream I/O operators, manipulators etc will operate on
   the PString class.
 */

  PCLASSINFO(PStringStream, PString)

  public:
    PStringStream();
    /* Create a new, empty, string stream. Data may be output to this stream,
       but attempts to input from it will return end of file.
     */

    PStringStream(
      const PString & str   // Initial value for string stream.
    );
    /* Create a new string stream and initialise it to the provided value. The
       string stream references the same string buffer as the $B$str$B$
       parameter until any output to the string stream is attempted. The
       reference is then broken and the instance of the string stream becomes
       a unique reference to a string buffer.
     */

    PStringStream(
      const char * cstr   // Initial value for the string stream.
    );
    /* Create a new string stream and initialise it with the provided value.
       The stream may be read or written from. Writes will append to the end of
       the string.
     */

    PStringStream & operator=(
      const PString & str  // New string to assign.
    );
    /* Assign the string to the current object. The current instance then
       becomes another reference to the same string in the $B$str$B$ parameter.
       
       This will reset the read pointer for input to the beginning of the
       string. Also, any data output to the string up until the asasignement
       will be lost.

       Returns: reference to the current PStringStream object.
     */

    PStringStream & operator=(
      const char * cstr  // C string to assign.
    );
    /* Assign the C string to the string stream. The current instance then
       becomes a unique reference to a copy of the $B$cstr$B$ parameter. The
       $B$cstr$B$ parameter is typically a literal string, eg:

          $F$myStr = "fred";$F$

       This will reset the read pointer for input to the beginning of the
       string. Also, any data output to the string up until the asasignement
       will be lost.

       Returns: reference to the current PStringStream object.
     */


    virtual ~PStringStream();
    // Destroy the string stream, deleting the stream buffer


  private:
    PStringStream(int, const PStringStream &) { }
    PStringStream & operator=(const PStringStream &) { return *this; }

    PCLASS Buffer : public PObject, public streambuf {
      PCLASSINFO(Buffer, PObject)
    
      public:
        Buffer(PStringStream * str);
        Buffer(const Buffer & sbuf);
        Buffer & operator=(const Buffer & sbuf);
        virtual int overflow(int=EOF);
        virtual int underflow();
        virtual int sync();
        virtual streampos seekoff(streamoff, ios::seek_dir, int);
        PStringStream * string;
    };
};


PDECLARE_ARRAY(PStringArray, PString)
/*PDECLARE_CLASS(PStringArray, PArray)
   This is an array collection class of $H$PString objects. It has all the
   usual functions for a collection, with the object types set to $H$PString
   pointers.
   
   In addition some addition functions are added that take a const $H$PString
   reference instead of a pointer as most standard collection functions do.
   This is more convenient for when string expressions are used as parameters
   to function in the collection.

   See the $H$PAbstractArray and $H$PArray classes and $H$PDECLARE_ARRAY macro
   for more information.
 */

  public:
    PStringArray(PINDEX count, char **strarr);
    PINDEX GetStringsIndex(const PString & str) const;
};


PDECLARE_LIST(PStringList, PString)
/*PDECLARE_CLASS(PStringList, PList)
   This is a list collection class of $H$PString objects. It has all the
   usual functions for a collection, with the object types set to $H$PString
   pointers.
   
   In addition some addition functions are added that take a const $H$PString
   reference instead of a pointer as most standard collection functions do.
   This is more convenient for when string expressions are used as parameters
   to function in the collection.

   See the $H$PAbstractList and $H$PList classes and $H$PDECLARE_LIST macro
   for more information.
 */

  public:
    PINDEX AppendString(const PString & str);
    PINDEX InsertString(const PString & before, const PString & str);
    PINDEX GetStringsIndex(const PString & str) const;
};


PDECLARE_SORTED_LIST(PSortedStringList, PString)
/*PDECLARE_CLASS(PSortedStringList, PSortedList)
   This is a sorted list collection class of $H$PString objects. It has all the
   usual functions for a collection, with the object types set to $H$PString
   pointers.
   
   In addition some addition functions are added that take a const $H$PString
   reference instead of a pointer as most standard collection functions do.
   This is more convenient for when string expressions are used as parameters
   to function in the collection.

   See the $H$PAbstractSortedList and $H$PSortedList classes and
   $H$PDECLARE_SORTEDLIST macro for more information.
 */

  public:
    PINDEX AppendString(const PString & str);
    PINDEX InsertString(const PString & before, const PString & str);
    PINDEX GetStringsIndex(const PString & str) const;
};


PDECLARE_SET(PStringSet, PString, TRUE)
/*PDECLARE_CLASS(PStringSet, PSet)
   This is a set collection class of $H$PString objects. It has all the
   usual functions for a collection, with the object types set to $H$PString
   pointers.

   In addition some addition functions are added that take a const $H$PString
   reference instead of a pointer as most standard collection functions do.
   This is more convenient for when string expressions are used as parameters
   to function in the collection.

   Unlike the normal sets, this will delete the PStrings removed from it. This
   complements the automatic creation of new PString objects when literals or
   expressions are used.

   See the $H$PAbstractSet and $H$PSet classes and $H$PDECLARE_SET macro for
   more information.
 */

  public:
    void Include(const PString & key);
    void Exclude(const PString & key);
};


#ifdef PHAS_TEMPLATES

template <class K>
PDECLARE_CLASS(PStringDictionary, PAbstractDictionary)
/* This template class maps the PAbstractDictionary to a specific key type and
   a $H$PString data type. The functions in this class primarily do all the
   appropriate casting of types.

   Note that if templates are not used the $H$PDECLARE_STRING_DICTIONARY macro
   will simulate the template instantiation.
 */

  public:
    PStringDictionary()
      : PAbstractDictionary() { }
    /* Create a new, empty, dictionary.

       Note that by default, objects placed into the dictionary will be
       deleted when removed or when all references to the dictionary are
       destroyed.
     */

    virtual PObject * Clone() const
      { return PNEW PStringDictionary(0, this); }
    /* Make a complete duplicate of the dictionary. Note that all objects in
       the array are also cloned, so this will make a complete copy of the
       dictionary.
     */

    const PString & operator[](const K & key) const
      { return (const PString &)GetRefAt(key); }
    /* Get the string contained in the dictionary at the $B$key$B$ position.
       The hash table is used to locate the data quickly via the hash function
       privided by the key.

       The last key/data pair is remembered by the class so that subseqent
       access is very fast.

       Returns: reference to the object indexed by the key.
     */

    virtual PString * GetAt(
      const K & key   // Key for position in dictionary to get object.
    ) const { return (PString *)PAbstractDictionary::GetAt(key); }
    /* Get the object at the specified key position. If the key was not in the
       collection then NULL is returned.

       Returns: pointer to object at the specified key.
     */

    virtual BOOL SetDataAt(
      PINDEX index,        // Ordinal index in the dictionary.
      const PString & str  // New string value to put into the dictionary.
    ) { return PAbstractDictionary::SetDataAt(index, PNEW PString(str)); }
    /* Set the data at the specified ordinal index position in the dictionary.

       The ordinal position in the dictionary is determined by the hash values
       of the keys and the order of insertion.

       Returns: TRUE if the new object could be placed into the dictionary.
     */

    virtual BOOL SetAt(
      const K & key,       // Key for position in dictionary to add object.
      const PString & str  // New string value to put into the dictionary.
    ) { return PAbstractDictionary::SetAt(index, PNEW PString(str)); }
    /* Add a new object to the collection. If the objects value is already in
       the dictionary then the object is overrides the previous value. If the
       AllowDeleteObjects option is set then the old object is also deleted.

       The object is placed in the an ordinal position dependent on the keys
       hash function. Subsequent searches use the has function to speed access
       to the data item.

       Returns: TRUE if the object was successfully added.
     */

    const K & GetKeyAt(PINDEX index) const
      { return (const K &)AbstractGetKeyAt(index); }
    /* Get the key in the dictionary at the ordinal index position.
    
       The ordinal position in the dictionary is determined by the hash values
       of the keys and the order of insertion.

       The last key/data pair is remembered by the class so that subseqent
       access is very fast.

       Returns: reference to key at the index position.
     */

    PString & GetDataAt(PINDEX index) const
      { return (PString &)AbstractGetDataAt(index); }
    /* Get the data in the dictionary at the ordinal index position.
    
       The ordinal position in the dictionary is determined by the hash values
       of the keys and the order of insertion.

       The last key/data pair is remembered by the class so that subseqent
       access is very fast.

       Returns: reference to data at the index position.
     */


  protected:
    PDictionary(int dummy, const cls * c)
      : PAbstractDictionary(dummy, c) { }
};


/*$MACRO PDECLARE_STRING_DICTIONARY(cls, K)
   This macro is used to declare a descendent of PAbstractList class,
   customised for a particular key type $B$K$B$ and data object type
   $H$PString.

   If the compilation is using templates then this macro produces a descendent
   of the $H$PStringDictionary template class. If templates are not being used
   then the macro defines a set of inline functions to do all casting of types.
   The resultant classes have an identical set of functions in either case.

   See the $H$PStringDictionary and $H$PAbstractDictionary classes for more
   information.
 */
#define PDECLARE_ORDINAL_DICTIONARY(cls, K) \
  PDECLARE_CLASS(cls, POrdinalDictionary<K>)
  protected: \
    cls(int dummy, const cls * c) \
      : POrdinalDictionary<K>(dummy, c) { } \
  public: \
    cls() \
      : PDictionary<K>() { } \
    virtual PObject * Clone() const \
      { return PNEW cls(0, this); } \


/*$MACRO PSTRING_DICTIONARY(cls, K, D)
   This macro is used to declare a descendent of PAbstractDictionary class,
   customised for a particular key type $B$K$B$ and data object type
   $H$PString. This macro closes the class declaration off so no additional
   members can be added.

   If the compilation is using templates then this macro produces a typedef
   of the $H$PStringDictionary template class.

   See the $H$PStringDictionary class and $H$PDECLARE_STRING_DICTIONARY macro
   for more information.
 */
#define PORDINAL_DICTIONARY(cls, K) typedef POrdinalDictionary<K> cls


#else // PHAS_TEMPLATES


#define PDECLARE_STRING_DICTIONARY(cls, K) \
  PDECLARE_CLASS(cls, PAbstractDictionary) \
  protected: \
    inline cls(int dummy, const cls * c) \
      : PAbstractDictionary(dummy, c) { } \
  public: \
    inline cls() \
      : PAbstractDictionary() { } \
    inline virtual PObject * Clone() const \
      { return PNEW cls(0, this); } \
    inline PString & operator[](const K & key) const \
      { return (PString &)GetRefAt(key); } \
    inline virtual PString * GetAt(const K & key) const \
      { return (PString *)PAbstractDictionary::GetAt(key); } \
    inline virtual BOOL SetDataAt(PINDEX index, const PString str) \
     {return PAbstractDictionary::SetDataAt(index,PNEW PString(str));} \
    inline virtual BOOL SetAt(const K & key, const PString & str) \
      { return PAbstractDictionary::SetAt(key, PNEW PString(str)); } \
    inline const K & GetKeyAt(PINDEX index) const \
      { return (const K &)AbstractGetKeyAt(index); } \
    inline PString & GetDataAt(PINDEX index) const \
      { return (PString &)AbstractGetDataAt(index); } \

#define PSTRING_DICTIONARY(cls, K) PDECLARE_STRING_DICTIONARY(cls, K) }


#endif // PHAS_TEMPLATES


PSTRING_DICTIONARY(POrdinalStringDictionary, POrdinalKey);
/*PDECLARE_CLASS(POrdinalStringDictionary, PStringDictionary)
   This is a dictionary collection class of $H$PString objects, keyed by an
   ordinal value. It has all the usual functions for a collection, with the
   object types set to $H$PString pointers. The class could be considered like
   a sparse array of strings.

   In addition some addition functions are added that take a const $H$PString
   reference instead of a pointer as most standard collection functions do.
   This is more convenient for when string expressions are used as parameters
   to function in the collection.

   See the $H$PAbstractDictionary and $H$PStringDictionary classes and
   $H$PDECLARE_DICTIONARY and $H$PDECLARE_STRING_DICTIONARY macros for more
   information.
 */

PORDINAL_DICTIONARY(PStringOrdinalDictionary, PString);
/*PDECLARE_CLASS(PStringOrdinalDictionary, POrdinalDictionary)
   This is a dictionary collection class of ordinals (via the $H$POrdinalKey
   class) keyed by $H$PString objects. It has all the usual functions for a
   collection, with the object types set to $H$POrdinalKey pointers.

   In addition some addition functions are added that take a const
   $H$POrdinalKey reference ir a simple PINDEX instead of a pointer as most
   standard collection functions do. This is more convenient for when integer
   expressions are used as parameters to function in the collection.

   See the $H$PAbstractDicionary and $H$POrdinalDictionary classes and
   $H$PDECLARE_ORDINAL_DICTIONARY macro for more information.
 */


// End Of File ///////////////////////////////////////////////////////////////
