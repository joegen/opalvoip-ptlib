/*
 * $Id: httpform.h,v 1.1 1996/06/28 12:55:56 robertj Exp $
 *
 * Portable Windows Library
 *
 * Application Socket Class Declarations
 *
 * Copyright 1995 Equivalence
 *
 * $Log: httpform.h,v $
 * Revision 1.1  1996/06/28 12:55:56  robertj
 * Initial revision
 *
 */

#ifndef _PHTTPFORM
#define _PHTTPFORM

#ifdef __GNUC__
#pragma interface
#endif

#include <http.h>


///////////////////////////////////////////////////////////////////////////////
// PHTTPField

PDECLARE_CLASS(PHTTPField, PObject)
/* This class is the abstract base class for fields in a <A>PHTTPForm</A>
   resource type.
 */
  public:
    PHTTPField(
      const char * name,   // Name (identifier) for the field.
      const char * title,  // Title text for field (defaults to name).
      const char * help    // Help text for the field.
    );
    // Create a new field in a HTTP form.

    virtual Comparison Compare(
      const PObject & obj
    ) const;
    /* Compare the fields using the field names.

       <H2>Returns:</H2>
       Comparison of the name fields of the two fields.
     */

    const PCaselessString & GetName() const { return name; }
    /* Get the identifier name of the field.

       <H2>Returns:</H2>
       String for field name.
     */

    const PString & GetTitle() const { return title; }
    /* Get the title of the field.

       <H2>Returns:</H2>
       String for title placed next to the field.
     */

    const PString & GetHelp() const { return help; }
    /* Get the title of the field.

       <H2>Returns:</H2>
       String for title placed next to the field.
     */

    void SetHelp(
      const PString & text        // Help text.
    ) { help = text; }
    void SetHelp(
      const PString & hotLinkURL, // URL for link to help page.
      const PString & linkText    // Help text in the link.
    );
    void SetHelp(
      const PString & hotLinkURL, // URL for link to help page.
      const PString & imageURL,   // URL for image to be displayed in link.
      const PString & imageText   // Text in the link when image unavailable.
    );
    // Set the help text for the field.

    virtual void GetHTML(
      PHTML & html    // HTML to receive the field info.
    ) = 0;
    /* Convert the field to HTML for inclusion into the HTTP page.
     */

    virtual PString GetValue() const = 0;
    /* Get the value of the field.

       <H2>Returns:</H2>
       String for field value.
     */

    virtual void SetValue(
      const PString & val   // New value for the field.
    ) = 0;
    /* Set the value of the field.
     */

    virtual BOOL Validated(
      const PString & newVal, // Proposed new value for the field.
      PStringStream & msg     // Stream to take error HTML if value not valid.
    ) const;
    /* Validate the new field value before <A>SetValue()</A> if called.

       <H2>Returns:</H2>
       BOOL if the new field value is OK.
     */

    BOOL NotYetInHTML() const { return notInHTML; }

  protected:
    PCaselessString name;
    PString title;
    PString help;
    BOOL notInHTML;
};


PDECLARE_CLASS(PHTTPStringField, PHTTPField)
  public:
    PHTTPStringField(
      const char * name,
      PINDEX size,
      const char * initVal = NULL,
      const char * help = NULL
    );
    PHTTPStringField(
      const char * name,
      const char * title,
      PINDEX size,
      const char * initVal = NULL,
      const char * help = NULL
    );

    virtual void GetHTML(
      PHTML & html
    );

    virtual PString GetValue() const;

    virtual void SetValue(
      const PString & val
    );


  protected:
    PString value;
    PINDEX size;
};


PDECLARE_CLASS(PHTTPPasswordField, PHTTPStringField)
  public:
    PHTTPPasswordField(
      const char * name,
      PINDEX size,
      const char * initVal = NULL,
      const char * help = NULL
    );
    PHTTPPasswordField(
      const char * name,
      const char * title,
      PINDEX size,
      const char * initVal = NULL,
      const char * help = NULL
    );

    virtual void GetHTML(
      PHTML & html
    );
};


PDECLARE_CLASS(PHTTPIntegerField, PHTTPField)
  public:
    PHTTPIntegerField(
      const char * name,
      int low, int high,
      int initVal = 0,
      const char * units = NULL,
      const char * help = NULL
    );
    PHTTPIntegerField(
      const char * name,
      const char * title,
      int low, int high,
      int initVal = 0,
      const char * units = NULL,
      const char * help = NULL
    );

    virtual void GetHTML(
      PHTML & html
    );

    virtual PString GetValue() const;

    virtual void SetValue(
      const PString & val
    );

    virtual BOOL Validated(
      const PString & newVal,
      PStringStream & msg
    ) const;


  protected:
    int low, high, value;
    PString units;
};


PDECLARE_CLASS(PHTTPBooleanField, PHTTPField)
  public:
    PHTTPBooleanField(
      const char * name,
      BOOL initVal = FALSE,
      const char * help = NULL
    );
    PHTTPBooleanField(
      const char * name,
      const char * title,
      BOOL initVal = FALSE,
      const char * help = NULL
    );

    virtual void GetHTML(
      PHTML & html
    );

    virtual PString GetValue() const;

    virtual void SetValue(
      const PString & val
    );


  protected:
    BOOL value;
};


PDECLARE_CLASS(PHTTPRadioField, PHTTPField)
  public:
    PHTTPRadioField(
      const char * name,
      const PStringArray & valueArray,
      PINDEX initVal = 0,
      const char * help = NULL
    );
    PHTTPRadioField(
      const char * name,
      const PStringArray & valueArray,
      const PStringArray & titleArray,
      PINDEX initVal = 0,
      const char * help = NULL
    );
    PHTTPRadioField(
      const char * name,
      PINDEX count,
      const char * const * valueStrings,
      PINDEX initVal = 0,
      const char * help = NULL
    );
    PHTTPRadioField(
      const char * name,
      PINDEX count,
      const char * const * valueStrings,
      const char * const * titleStrings,
      PINDEX initVal = 0,
      const char * help = NULL
    );
    PHTTPRadioField(
      const char * name,
      const char * groupTitle,
      const PStringArray & valueArray,
      PINDEX initVal = 0,
      const char * help = NULL
    );
    PHTTPRadioField(
      const char * name,
      const char * groupTitle,
      const PStringArray & valueArray,
      const PStringArray & titleArray,
      PINDEX initVal = 0,
      const char * help = NULL
    );
    PHTTPRadioField(
      const char * name,
      const char * groupTitle,
      PINDEX count,
      const char * const * valueStrings,
      PINDEX initVal = 0,
      const char * help = NULL
    );
    PHTTPRadioField(
      const char * name,
      const char * groupTitle,
      PINDEX count,
      const char * const * valueStrings,
      const char * const * titleStrings,
      PINDEX initVal = 0,
      const char * help = NULL
    );

    virtual void GetHTML(
      PHTML & html
    );

    virtual PString GetValue() const;

    virtual void SetValue(
      const PString & val
    );


  protected:
    PStringArray values;
    PStringArray titles;
    PString value;
};


PDECLARE_CLASS(PHTTPSelectField, PHTTPField)
  public:
    PHTTPSelectField(
      const char * name,
      const PStringArray & valueArray,
      PINDEX initVal = 0,
      const char * help = NULL
    );
    PHTTPSelectField(
      const char * name,
      PINDEX count,
      const char * const * valueStrings,
      PINDEX initVal = 0,
      const char * help = NULL
    );
    PHTTPSelectField(
      const char * name,
      const char * title,
      const PStringArray & valueArray,
      PINDEX initVal = 0,
      const char * help = NULL
    );
    PHTTPSelectField(
      const char * name,
      const char * title,
      PINDEX count,
      const char * const * valueStrings,
      PINDEX initVal = 0,
      const char * help = NULL
    );

    virtual void GetHTML(
      PHTML & html
    );

    virtual PString GetValue() const;

    virtual void SetValue(
      const PString & val
    );


  protected:
    PStringArray values;
    PString value;
};


///////////////////////////////////////////////////////////////////////////////
// PHTTPForm

PDECLARE_CLASS(PHTTPForm, PHTTPString)
  public:
    PHTTPForm(
      const PURL & url
    );
    PHTTPForm(
      const PURL & url,
      const PHTTPAuthority & auth
    );
    PHTTPForm(
      const PURL & url,
      const PString & html
    );
    PHTTPForm(
      const PURL & url,
      const PString & html,
      const PHTTPAuthority & auth
    );


    virtual BOOL Post(
      PHTTPRequest & request,       // Information on this request.
      const PStringToString & data, // Variables in the POST data.
      PHTML & replyMessage          // Reply message for post.
    );


    PHTTPField * Add(
      PHTTPField * fld
    );

    enum BuildOptions {
      CompleteHTML,
      InsertIntoForm,
      InsertIntoHTML
    };

    void BuildHTML(
      const char * heading
    );
    void BuildHTML(
      const PString & heading
    );
    void BuildHTML(
      PHTML & html,
      BuildOptions option = CompleteHTML
    );


  protected:
    PLIST(FieldList, PHTTPField);
    FieldList fields;
    PStringSet fieldNames;
};


//////////////////////////////////////////////////////////////////////////////
// PHTTPConfig

PDECLARE_CLASS(PHTTPConfig, PHTTPForm)
  public:
    PHTTPConfig(
      const PURL & url,
      const PString & section
    );
    PHTTPConfig(
      const PURL & url,
      const PString & section,
      const PHTTPAuthority & auth
    );
    PHTTPConfig(
      const PURL & url,
      const PString & section,
      const PString & html
    );
    PHTTPConfig(
      const PURL & url,
      const PString & section,
      const PString & html,
      const PHTTPAuthority & auth
    );

    virtual void OnLoadedText(
      PHTTPRequest & request,    // Information on this request.
      PString & text             // Data used in reply.
    );
    virtual BOOL Post(
      PHTTPRequest & request,       // Information on this request.
      const PStringToString & data, // Variables in the POST data.
      PHTML & replyMessage          // Reply message for post.
    );


    const PString & GetConfigSection() const { return section; }
    /* Get the configuration file section that the page will alter.

       <H2>Returns:</H2>
       String for config file section.
     */

    void SetConfigSection(
      const PString & sect   // New section for the config page.
    ) { section = sect; }
    // Set the configuration file section.

    PHTTPField * AddSectionField(
      PHTTPField * sectionFld,     // Field to set as the section name
      const char * prefix = NULL,  // String to attach before the field value
      const char * suffix = NULL   // String to attach after the field value
    );
    /* Add a field that will determine the name opf the secontion into which
       the other fields are to be added as keys. The section is not created and
       and error generated if the section already exists.
     */

    void AddNewKeyFields(
      PHTTPField * keyFld,  // Field for the key to be added.
      PHTTPField * valFld   // Field for the value of the key yto be added.
    );
    /* Add fields to the HTTP form for adding a new key to the config file
       section.
     */


  protected:
    PString section;
    PString sectionPrefix;
    PString sectionSuffix;
    PHTTPField * sectionField;
    PHTTPField * keyField;
    PHTTPField * valField;

  private:
    void Construct();
};


#endif


// End Of File ///////////////////////////////////////////////////////////////
