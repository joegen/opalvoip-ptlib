%{
/*
 * $Id: asn_lex.l,v 1.2 1998/05/21 04:21:47 robertj Exp $
 *
 * Portable Windows Library
 *
 * ASN Compiler
 *
 * Copyright 1997 by Equivalence
 *
 * $Log: asn_lex.l,v $
 * Revision 1.2  1998/05/21 04:21:47  robertj
 * Implementing more of the ASN spec.
 *
 * Revision 1.1  1997/12/13 09:17:49  robertj
 * Initial revision
 *
 *
 */

#include <ptlib.h>
#include "main.h"

#include "asn_grammar.h"

#include <ctype.h>

extern FILE * yyin;

extern unsigned lineNumber;
int IdentifierTokenContext = IDENTIFIER;
int BraceTokenContext = '{';
int InMacroContext = FALSE;
int HasObjectTypeMacro = FALSE;
int InMIBContext = FALSE;

int yywrap() { return 1; }

static int TokenSelect(int context, int token1, int token2)
{
  if (context)
    return token1;
  yylval.sval = new PString(yytext);
  return token2;
}

%}

%x comment

%%

"::="         { return ASSIGNMENT; }

"ABSENT"                           { return ABSENT; }              
"ABSTRACT-SYNTAX"                  { return ABSTRACT_SYNTAX; }
"ALL"                              { return ALL; }
"APPLICATION"                      { return APPLICATION; }
"AUTOMATIC"                        { return AUTOMATIC; }
"BEGIN"                            { return BEGIN_t; }
"BIT"                              { return BIT; }
"BMPString"                        { return BMPString; }
"BOOLEAN"                          { return BOOLEAN_t; }
"BY"                               { return BY; }
"CHARACTER"                        { return CHARACTER; }
"CHOICE"                           { return CHOICE; }
"CLASS"                            { return CLASS; }
"COMPONENT"                        { return COMPONENT; }
"COMPONENTS"                       { return COMPONENTS; }
"CONSTRAINED"                      { return CONSTRAINED; }
"DEFAULT"                          { return DEFAULT; }
"DEFINITIONS"                      { return DEFINITIONS; }
"EMBEDDED"                         { return EMBEDDED; }
"END"                              { return END; }
"ENUMERATED"                       { return ENUMERATED; }
"EXCEPT"                           { return EXCEPT; }
"EXPLICIT"                         { return EXPLICIT; }
"EXPORTS"                          { return EXPORTS; }
"EXTERNAL"                         { return EXTERNAL; }
"FALSE"                            { return FALSE_t; }
"FROM"                             { return FROM; }
"GeneralString"                    { return GeneralString; }
"GraphicString"                    { return GraphicString; }
"IA5String"                        { return IA5String; }
"TYPE-IDENTIFIER"                  { return TYPE_IDENTIFIER; }
"IDENTIFIER"                       { return IDENTIFIER_t; }
"IMPLICIT"                         { return IMPLICIT; }
"IMPORTS"                          { return IMPORTS; }
"INCLUDES"                         { return INCLUDES; }
"INSTANCE"                         { return INSTANCE; }
"INTEGER"                          { return INTEGER_t; }
"INTERSECTION"                     { return INTERSECTION; }
"ISO646String"                     { return ISO646String; }
"MACRO"				   { return MACRO; }
"MAX"                              { return MAX; }
"MIN"                              { return MIN; }
"MINUS-INFINITY"                   { return MINUS_INFINITY; }
"NOTATION"			   { return NOTATION; }
"NULL"                             { return NULL_t; }
"NumericString"                    { return NumericString; }
"OBJECT"                           { return OBJECT; }
"OCTET"                            { return OCTET; }
"OF"                               { return OF_t; }
"OPTIONAL"                         { return OPTIONAL_t; }
"PDV"                              { return PDV; }
"PLUS-INFINITY"                    { return PLUS_INFINITY; }
"PRESENT"                          { return PRESENT; }
"PrintableString"                  { return PrintableString; }
"PRIVATE"                          { return PRIVATE; }
"REAL"                             { return REAL; }
"SEQUENCE"                         { return SEQUENCE; }
"SET"                              { return SET; }
"SIZE"                             { return SIZE_t; }
"STRING"                           { return STRING; }
"SYNTAX"                           { return SYNTAX; }
"T61String"                        { return T61String; }
"TAGS"                             { return TAGS; }
"TeletexString"                    { return TeletexString; }
"TRUE"                             { return TRUE_t; }
"TYPE"				   { return TYPE_t; }
"UNION"                            { return UNION; }
"UNIQUE"                           { return UNIQUE; }
"UNIVERSAL"                        { return UNIVERSAL; }
"UniversalString"                  { return UniversalString; }
"VALUE"				   { return VALUE; }
"VideotexString"                   { return VideotexString; }
"VisibleString"                    { return VisibleString; }
"WITH"                             { return WITH; }
"string"                           { return TokenSelect(InMacroContext, string_t, IDENTIFIER); }
"identifier"                       { return TokenSelect(InMacroContext, identifier_t, IDENTIFIER); }
"number"                           { return TokenSelect(InMacroContext, number_t, IDENTIFIER); }
"empty"                            { return TokenSelect(InMacroContext, empty_t, IDENTIFIER); }
"type"                             { return TokenSelect(InMacroContext, type_t, IDENTIFIER); }
"value"                            { return TokenSelect(InMacroContext, value_t, IDENTIFIER); }
"OBJECT-TYPE"			   { return TokenSelect(HasObjectTypeMacro, OBJECT_TYPE, TYPEREFERENCE); }
"TRAP-TYPE"			   { return TokenSelect(HasObjectTypeMacro, TRAP_TYPE, TYPEREFERENCE); }
"ACCESS"			   { return TokenSelect(InMIBContext, ACCESS, TYPEREFERENCE); }
"STATUS"			   { return TokenSelect(InMIBContext, STATUS, TYPEREFERENCE); }
"read-only"			   { return TokenSelect(InMIBContext, read_only_t, IDENTIFIER); }
"read-write"			   { return TokenSelect(InMIBContext, read_write_t, IDENTIFIER); }
"write-only"			   { return TokenSelect(InMIBContext, write_only_t, IDENTIFIER); }
"not-accessible"		   { return TokenSelect(InMIBContext, not_accessible_t, IDENTIFIER); }
"mandatory"			   { return TokenSelect(InMIBContext, mandatory_t, IDENTIFIER); }
"optional"			   { return TokenSelect(InMIBContext, optional_t, IDENTIFIER); }
"obsolete"			   { return TokenSelect(InMIBContext, obsolete_t, IDENTIFIER); }
"deprecated"			   { return TokenSelect(InMIBContext, deprecated_t, IDENTIFIER); }
"DESCRIPTION"			   { return TokenSelect(InMIBContext, DESCRIPTION_t, TYPEREFERENCE); }
"REFERENCE"			   { return TokenSelect(InMIBContext, REFERENCE_t, TYPEREFERENCE); }
"INDEX"				   { return TokenSelect(InMIBContext, INDEX_t, TYPEREFERENCE); }
"DEFVAL"			   { return TokenSelect(InMIBContext, DEFVAL_t, TYPEREFERENCE); }
"ENTERPRISE"			   { return TokenSelect(InMIBContext, ENTERPRISE, TYPEREFERENCE); }
"VARIABLES"			   { return TokenSelect(InMIBContext, VARIABLES, TYPEREFERENCE); }

                                                                 

"--" {
    BEGIN(comment);
  }

<comment>"--"  {
    BEGIN(INITIAL);
  }

<comment>\n  {
    lineNumber++;
    BEGIN(INITIAL);
  }

<comment>. {
}


"@"  { return '@'; }

"{"  { return BraceTokenContext; }

"}"  { return '}'; }

"["  { return '['; }

"]"  { return ']'; }

"("  { return '('; }

")"  { return ')'; }

"<"  { return '<'; }

">"  { return '>'; }

"|"  { return '|'; }

";"  { return ';'; }

"+"  { return '+'; }

"-"  { return '-'; }

"*"  { return '*'; }

"/"  { return '/'; }

","  { return ','; }

"."  { return '.'; }

":"  { return ':'; }

"="  { return '='; }

(\"[^\"]*\")(\"[^\"]*\")*  {
    PString s = yytext;
    PINDEX pos;
    while ((pos = s.Find('\n')) != P_MAX_INDEX) {
      PINDEX start = pos;
      while (start > 0 && isspace(s[start-1]))
	start--;
      while (isspace(s[pos]))
	pos++;
      s.Splice(" ", start, pos - start);
      lineNumber++;
    }
    yylval.sval = new PString(s);
    return CSTRING;
  }

[0-9][0-9]*  {
    const char * ptr = yytext;
    yylval.ival = 0;
    while (*ptr != '\0')
      yylval.ival = yylval.ival * 10 + *ptr++ - '0';
    return INTEGER;
  }

([a-z]|([a-z][a-zA-Z0-9_-]*[a-zA-Z0-9_]))  {
    yylval.sval = new PString(yytext);
    return IdentifierTokenContext;
  }

([A-Z]|([A-Z][a-zA-Z0-9_-]*[a-zA-Z0-9_]))  {	  /* See X.680 section 9.2 */
    yylval.sval = new PString(yytext);
    return TYPEREFERENCE;
  }

[ \t\r]  { }

\n {
    lineNumber++;
  }

.  { PError << StdError(Fatal) << "unknown token " << yytext << endl; }


%%
/* End PR_LEX.L */
