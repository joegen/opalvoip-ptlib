%{

/*
 * $Id: asn_grammar.y,v 1.1 1997/12/13 09:17:47 robertj Exp $
 *
 * ASN Grammar
 *
 * Copyright 1997 by Equivalence Pty. Ltd.
 *
 * $Log: asn_grammar.y,v $
 * Revision 1.1  1997/12/13 09:17:47  robertj
 * Initial revision
 *
 */

#include <ptlib.h>
#include "main.h"

extern int yylex();

extern int IdentifierTokenContext;
extern int InMacroContext;
extern int HasObjectTypeMacro;
extern int InMIBContext;

int UnnamedFieldCount = 1;


%}

%token IDENTIFIER
%token BIT_IDENTIFIER
%token OID_IDENTIFIER
%token TYPEREFERENCE
%token INTEGER

%token BSTRING
%token CSTRING
%token HSTRING

%token ABSENT           
%token ABSTRACT_SYNTAX  
%token ALL              
%token APPLICATION      
%token ASSIGNMENT
%token AUTOMATIC        
%token BEGIN_t
%token BIT              
%token BMPString
%token BOOLEAN_t
%token BY
%token CHARACTER        
%token CHOICE           
%token CLASS            
%token COMPONENT        
%token COMPONENTS       
%token CONSTRAINED      
%token DEFAULT          
%token DEFINITIONS      
%token EMBEDDED         
%token END
%token ENUMERATED       
%token EXCEPT           
%token EXPLICIT         
%token EXPORTS          
%token EXTERNAL         
%token FALSE_t           
%token FROM             
%token GeneralString    
%token GraphicString    
%token IA5String        
%token TYPE_IDENTIFIER 
%token IDENTIFIER_t
%token IMPLICIT         
%token IMPORTS          
%token INCLUDES         
%token INSTANCE         
%token INTEGER_t        
%token INTERSECTION     
%token ISO646String
%token MACRO
%token MAX              
%token MIN              
%token MINUS_INFINITY
%token NOTATION
%token NULL_t
%token NumericString    
%token OBJECT           
%token OCTET            
%token OF_t              
%token OPTIONAL_t
%token PDV              
%token PLUS_INFINITY    
%token PRESENT          
%token PrintableString  
%token PRIVATE          
%token REAL             
%token SEQUENCE         
%token SET              
%token SIZE_t            
%token STRING           
%token SYNTAX           
%token T61String        
%token TAGS             
%token TeletexString    
%token TRUE_t
%token TYPE_t
%token UNION            
%token UNIQUE           
%token UNIVERSAL        
%token UniversalString  
%token VideotexString   
%token VisibleString    
%token GeneralizedTime
%token UTCTime
%token VALUE
%token WITH
%token string_t
%token identifier_t
%token number_t
%token empty_t
%token type_t
%token value_t
%token OBJECT_TYPE
%token TRAP_TYPE
%token ACCESS
%token STATUS
%token read_only_t
%token read_write_t
%token write_only_t
%token not_accessible_t
%token mandatory_t
%token optional_t
%token obsolete_t
%token deprecated_t
%token DESCRIPTION_t
%token REFERENCE_t
%token INDEX_t
%token DEFVAL_t
%token ENTERPRISE
%token VARIABLES

%token ObjectDescriptor_t


%type <ival> INTEGER Class ClassNumber TagDefault SignedNumber
%type <ival> ObjectTypeAccess ObjectTypeStatus

%type <sval> IDENTIFIER BIT_IDENTIFIER OID_IDENTIFIER TYPEREFERENCE
%type <sval> BSTRING CSTRING HSTRING
%type <sval> Symbol Reference GlobalModuleReference CharsDefn
%type <sval> DefinedValue_OID ExternalValueReference ExternalTypeReference
%type <sval> NumberForm NameAndNumberForm ObjIdComponent
%type <sval> DefinitiveNameAndNumberForm DefinitiveObjIdComponent
%type <sval> MibIndexType MibDescrPart MibReferPart

%type <slst> DefinitiveIdentifier DefinitiveObjIdComponentList SymbolList
%type <slst> ObjIdComponentList BitIdentifierList CharSyms CharacterStringList
%type <slst> MibIndexTypes MibIndexPart

%type <tval> Type DefinedType UsefulType SelectionType CharacterStringType 
%type <tval> ConstrainedType BitStringType BooleanType ChoiceType BuiltinType
%type <tval> EmbeddedPDVType EnumeratedType Enumerations ExternalType IntegerType
%type <tval> NullType ObjectIdentifierType  OctetStringType 
%type <tval> RealType SequenceType SequenceOfType SetType SetOfType TaggedType
%type <tval> RestrictedCharacterStringType UnrestrictedCharacterStringType
%type <tval> TypeWithConstraint NamedType ComponentType ComponentTypeLists
%type <tval> AlternativeTypeLists ContainedSubtype

%type <tlst> ComponentTypeList AlternativeTypeList

%type <vval> Value DefinedValue ReferencedValue BuiltinValue BitStringValue BooleanValue 
%type <vval> CharacterStringValue ChoiceValue NamedValue RestrictedCharacterStringValue
%type <vval> NullValue ObjectIdentifierValue RealValue NumericRealValue SpecialRealValue 
%type <vval> AssignedIdentifier ExceptionIdentification ExceptionSpec
%type <vval> LowerEndpoint LowerEndValue UpperEndpoint UpperEndValue
%type <vval> MibDefValPart

%type <vlst> ComponentValueList SequenceValue SetValue MibVarPart MibVarTypes

%type <nval> NamedNumber EnumerationItem NamedBit

%type <nlst> NamedNumberList Enumeration NamedBitList

%type <tagv> Tag

%type <cons> Constraint SizeConstraint ConstraintSpec ElementSetSpecs
%type <cons> ComponentConstraint NamedConstraint InnerTypeConstraints

%type <clst> TypeConstraints MultipleTypeConstraints

%type <elst> ElementSetSpec Unions Intersections

%type <elmt> IntersectionElements Elements Exclusions SubtypeElements
%type <elmt> ValueRange PermittedAlphabet

%union {
  PInt64	     ival;
  PString	   * sval;
  PStringList	   * slst;
  TypeBase	   * tval;
  TypesList	   * tlst;
  ValueBase	   * vval;
  ValuesList       * vlst;
  NamedNumber	   * nval;
  NamedNumberList  * nlst;
  Constraint	   * cons;
  ConstraintList   * clst;
  ValueElementList * elst;
  ValueElementBase * elmt;
  struct {
    Tag::Type tagClass;
    unsigned tagNumber;
  } tagv;
}

%%

ModuleDefinition
  : TYPEREFERENCE DefinitiveIdentifier DEFINITIONS TagDefault ASSIGNMENT BEGIN_t
      {
	Module = new ModuleDefinition($1, $2, (Tag::Mode)$4);
      }
    ModuleBody END
  ;

DefinitiveIdentifier
  : '{' DefinitiveObjIdComponentList '}'
	{
	  $$ = $2;
	}
  | /* empty */
	{
	  $$ = new PStringList;
	}
  ;

DefinitiveObjIdComponentList
  : DefinitiveObjIdComponent      
      {
	$$ = new PStringList;
	$$->Append($1);
      }
  | DefinitiveObjIdComponent DefinitiveObjIdComponentList
      {
	$2->InsertAt(0, $1);
	$$ = $2;
      }
  ;

DefinitiveObjIdComponent
  : IDENTIFIER	
  | INTEGER
      {
	$$ = new PString(PString::Unsigned, (int)$1);
      }
  | DefinitiveNameAndNumberForm
  ;

DefinitiveNameAndNumberForm
  : IDENTIFIER '(' INTEGER ')'
      {
	delete $1;
	$$ = new PString(PString::Unsigned, (int)$3);
      }
  ;

TagDefault
  : EXPLICIT TAGS
      {
	$$ = Tag::Explicit;
      }
  | IMPLICIT TAGS 
      {
	$$ = Tag::Implicit;
      }
  | AUTOMATIC TAGS 
      {
	$$ = Tag::Automatic;
      }
  | /* empty */
      {
	$$ = Tag::Explicit;
      }
  ;


/*************************************/

ModuleBody
  : Exports Imports AssignmentList
  | /* empty */
  ;

Exports
  : EXPORTS SymbolsExported ';'
  | /* empty */
  ;

SymbolsExported
  : SymbolList
      {
	Module->SetExports($1);
      }
  | /* empty */
      {
	Module->SetExportAll();
      }
  ;

Imports
  : IMPORTS SymbolsFromModuleList ';' 
  | /* empty */
  ;

SymbolsFromModuleList
  : SymbolsFromModule
  | SymbolsFromModuleList SymbolsFromModule
  ;

SymbolsFromModule
  : SymbolList FROM GlobalModuleReference
      {
	if (!HasObjectTypeMacro) {
	  HasObjectTypeMacro = $1->GetValuesIndex(PString("OBJECT-TYPE")) != P_MAX_INDEX;
	  if (HasObjectTypeMacro)
	    PError << "Info: including OBJECT-TYPE macro" << endl;
	}
	Module->AddImport(new ImportModule($3, $1));
      }
  ;

GlobalModuleReference
  : TYPEREFERENCE /*modulereference*/ AssignedIdentifier
      {
	delete $2;
      }
  ;

AssignedIdentifier
  : /* empty */
      {
	$$ = NULL;
      }
/*!!!! 
  | DefinedValue
*/ 
  | ObjectIdentifierValue
  ;

SymbolList
  : Symbol
      {
	$$ = new PStringList;
	$$->Append($1);
      }
  | Symbol ',' SymbolList
      {
	$3->Append($1);
	$$ = $3;
      }
  ;

Symbol
  : Reference
/*| ParameterizedReference	/* only required for X.683 */
  ;

Reference
  : TYPEREFERENCE
  | IDENTIFIER 
/*| objectclassreference      /* not defined in X.680 */
/*| objectreference	      /* not defined in X.680 */
/*| objectsetreference	      /* not defined in X.680 */
  ;


/*************************************/

AssignmentList: Assignment 
              | AssignmentList Assignment
  ;

Assignment
  : TypeAssignment
  | ValueAssignment
  | ValueSetTypeAssignment 
  | MacroDefinition
  | ObjectTypeDefinition
  | TrapTypeDefinition
/*| ObjectClassAssignment	/* ITU-T Rec. X.681 | ISO/IEC 8824-2, subclause 9.1 */
/*| ObjectAssignment		/* ITU-T Rec. X.681 | ISO/IEC 8824-2, subclause 11.1 */
/*| ObjectSetAssignment		/* ITU-T Rec. X.681 | ISO/IEC 8824-2, subclause 12.1 */
/*| ParameterizedAssignment	/* ITU-T Rec. X.683 | ISO/IEC 8824-4, subclause 8.1 */
  ;

ExternalTypeReference
  : TYPEREFERENCE /*modulereference*/ '.' TYPEREFERENCE
      {
	*$1 += *$3;
	delete $3;
      }
  ;

ExternalValueReference
  : TYPEREFERENCE /*modulereference*/ '.' IDENTIFIER
      {
	*$1 += *$3;
	delete $3;
      }
  ;

DefinedType
  : ExternalTypeReference 
      {
	$$ = new DefinedType($1);
      }
  | TYPEREFERENCE 
      {
	$$ = new DefinedType($1);
      }
/*| ParameterizedType		  /* ITU-T Rec. X.683 | ISO/IEC 8824-4 */
/*| ParameterizedValueSetType	  /* ITU-T Rec. X.683 | ISO/IEC 8824-4 */
  ;

DefinedValue
  : ExternalValueReference
      {
	$$ = new DefinedValue($1);
      }
  | IDENTIFIER
      {
	$$ = new DefinedValue($1);
      }
/*| ParameterizedValue		  /* specified in ITU-T Rec. X.683 | ISO/IEC 8824-4 */
  ;

DefinedValue_OID
  : ExternalValueReference
  | OID_IDENTIFIER
  ;


/*
 The following is for documentation references only (X.680 section 12.1)

AbsoluteReference
  : '@' GlobalModuleReference '.' ItemSpec
  ;

ItemSpec
  : TYPEREFERENCE 
  |  ItemSpec '.' ComponentId
  ;

ComponentId
  : IDENTIFIER
  | INTEGER
  | '*'
  ;
*/


TypeAssignment
  : TYPEREFERENCE ASSIGNMENT Type
      {
	$3->SetName($1);
	Module->AddType($3);
      }
  ;

ValueAssignment 
  : IDENTIFIER Type
      {
	IdentifierTokenContext = $2->GetIdentifierTokenContext();
      }
    ASSIGNMENT Value
      {
	$5->SetValueName($1);
	Module->AddValue($5);
	IdentifierTokenContext = IDENTIFIER;
      }
  ;

ValueSetTypeAssignment
  : TYPEREFERENCE Type
      {
	$2->SetName($1);
	Module->AddType($2);
	IdentifierTokenContext = $2->GetIdentifierTokenContext();
      }
    ASSIGNMENT ValueSet
      {
	IdentifierTokenContext = IDENTIFIER;
      }
  ;

ValueSet
  : '{' ElementSetSpec '}'
  ;

Type
  : DefinedType 
  | UsefulType 
  | SelectionType 
/*| TypeFromObject		/* ITU-T Rec. X.681 | ISO/IEC 8824-2, clause 15 */
/*| ValueSetFromObjects		/* ITU-T Rec. X.681 | ISO/IEC 8824-2, clause 15 */
  | ConstrainedType
  | BuiltinType
  ;

BuiltinType
  : BitStringType 
  | BooleanType 
  | CharacterStringType 
  | ChoiceType 
  | EmbeddedPDVType 
  | EnumeratedType 
  | ExternalType 
/*| InstanceOfType		  /* ITU-T Rec. X.681 | ISO/IEC 8824-2, Annex C */
  | IntegerType 
  | NullType 
/*| ObjectClassFieldType	  /* ITU-T Rec. X.681 | ISO/IEC 8824-2, subclause 14.1 */
  | ObjectIdentifierType 
  | OctetStringType 
  | RealType 
  | SequenceType 
  | SequenceOfType 
  | SetType 
  | SetOfType 
  | TaggedType
  ;

NamedType
  : IDENTIFIER Type
      {
	$2->SetName($1);
	$$ = $2;
      }
  | Type	     /* ITU-T Rec. X.680 Appendix H.1 */
      {
	PError << StdError(Warning) << "unnamed field." << endl;
	$1->SetName(new PString(PString::Printf, "_unnamed%u", UnnamedFieldCount++));
      }
/*| SelectionType    /* Unnecessary as have rule in Type for this */
  ;

Value
  : BuiltinValue 
  | ReferencedValue
  ;

BuiltinValue
  : BitStringValue 
  | BooleanValue 
  | CharacterStringValue 
  | ChoiceValue 
/*| EmbeddedPDVValue  synonym to SequenceValue */
/*| EnumeratedValue   synonym to IDENTIFIER    */
/*| ExternalValue     synonym to SequenceValue */
/*| InstanceOfValue   /* undefined in X.680 */
  | SignedNumber      /* IntegerValue */
      {
	$$ = new IntegerValue($1);
      }
  | NullValue 
/*| ObjectClassFieldValue   /* defined in ITU-T Rec. X.681 | ISO/IEC 8824-2, subclause 14.6 */
  | ObjectIdentifierValue
/*| OctetStringValue  equivalent to BitStringValue */
  | RealValue 
/*!!!!
  | SequenceValue 
  | SequenceOfValue 
*/
/*| SetValue	      synonym to SequenceValue */
/*| SetOfValue 	      synonym to SequenceOfValue */
/*| TaggedValue	      synonym to Value */
    { }
  ;

ReferencedValue
  : DefinedValue
/*| ValueFromObject   /* ITU-T Rec. X.681 | ISO/IEC 8824-2, clause 15 */
  ;

NamedValue
  : IDENTIFIER Value
      {
	$2->SetValueName($1);
	$$ = $2;
      }
  ;

BooleanType
  : BOOLEAN_t
      {
	$$ = new BooleanType;
      }
  ;

BooleanValue
  : TRUE_t
      {
	$$ = new BooleanValue(TRUE);
      }
  | FALSE_t
      {
	$$ = new BooleanValue(FALSE);
      }
  ;

IntegerType
  : INTEGER_t
      {
	$$ = new IntegerType;
      }
  | INTEGER_t '{' NamedNumberList '}'
      {
	$$ = new IntegerType($3);
      }
  ;

NamedNumberList
  : NamedNumber		
      {
	$$ = new NamedNumberList;
	$$->Append($1);
      }
  | NamedNumberList ',' NamedNumber
      {
	$1->Append($3);
      }
  ;

NamedNumber
  : IDENTIFIER '(' SignedNumber ')'	
      {
	$$ = new NamedNumber($1, (int)$3);
      }
  | IDENTIFIER '(' DefinedValue ')'
      {
	$$ = new NamedNumber($1, ((DefinedValue*)$3)->GetReference());
	delete $3;
      }
  ;

SignedNumber
  :  INTEGER 
  | '-' INTEGER
      {
	$$ = -$2;
      }
  ;

EnumeratedType
  : ENUMERATED '{' Enumerations '}'
      {
	$$ = $3;
      }
  ;

Enumerations
  : Enumeration
      {
	$$ = new EnumeratedType($1, FALSE, NULL);
      }
  | Enumeration  ',' '.' '.' '.'
      {
	$$ = new EnumeratedType($1, TRUE, NULL);
      }
  | Enumeration  ',' '.' '.' '.' ',' Enumeration
      {
	$$ = new EnumeratedType($1, TRUE, $7);
      }
  ;

Enumeration
  : EnumerationItem
      {
	$$ = new NamedNumberList;
	$$->Append($1);
      }
  | EnumerationItem ',' Enumeration
      {
	$3->InsertAt(0, $1);
	$1->SetAutoNumber((*$3)[1]);
	$$ = $3;
      }
  ;

EnumerationItem
  : IDENTIFIER
      {
	$$ = new NamedNumber($1);
      }
  | NamedNumber
  ;

RealType
  : REAL
      {
	$$ = new RealType;
      }
  ;

RealValue
  : NumericRealValue 
  | SpecialRealValue
  ;

NumericRealValue
  :  '0'
      {
	$$ = new RealValue(0);
      }
/*| SequenceValue	equivalent to other BuiltinValue's */
  ;

SpecialRealValue
  : PLUS_INFINITY
      {
	$$ = new RealValue(0);
      }
  | MINUS_INFINITY
      {
	$$ = new RealValue(0);
      }
  ;

BitStringType
  : BIT STRING 
      {
	$$ = new BitStringType;
      }
  | BIT STRING '{' NamedBitList '}'
      {
	$$ = new BitStringType($4);
      }
  ;

NamedBitList
  : NamedBit 
      {
	$$ = new NamedNumberList;
	$$->Append($1);
      }
  | NamedBitList ',' NamedBit
      {
	$1->InsertAt(0, $3);
      }
  ;

NamedBit
  : IDENTIFIER '(' INTEGER ')' 
      {
	$$ = new NamedNumber($1, (int)$3);
      }
  | IDENTIFIER '(' DefinedValue ')'
      {
	$$ = new NamedNumber($1, ((DefinedValue*)$3)->GetReference());
	delete $3;
      }
  ;

BitStringValue
  : BSTRING
      {
	$$ = new BitStringValue($1);
      }
  | HSTRING
      {
	$$ = new BitStringValue($1);
      }
  | '{' BitIdentifierList '}' 
      {
	$$ = new BitStringValue($2);
      }
  | '{'  '}'
      {
	$$ = new BitStringValue;
      }
  ;

BitIdentifierList
  : BIT_IDENTIFIER
      {
	$$ = new PStringList;
      }
  | BitIdentifierList ',' BIT_IDENTIFIER
      {
	// Look up $3
	$1->SetAt($1->GetSize(), 0);
      }
  ;

OctetStringType
  : OCTET STRING
      {
	$$ = new OctetStringType;
      }
  ;

NullType
  : NULL_t
      {
	$$ = new NullType;
      }
  ;

NullValue
  : NULL_t
      {
	$$ = new NullValue;
      }
  ;

SequenceType
  : SEQUENCE '{' ComponentTypeLists '}'
      {
	$$ = $3;
      }
  | SEQUENCE '{'  '}'
      {
	$$ = new SequenceType(NULL, FALSE, NULL);
      }
  | SEQUENCE '{' ExtensionAndException '}'
      {
	$$ = new SequenceType(NULL, TRUE, NULL);
      }
  ;

ExtensionAndException
  : '.' '.' '.' ExceptionSpec
  ;

ComponentTypeLists
  : ComponentTypeList
      {
	$$ = new SequenceType($1, FALSE, NULL);
      }
  | ComponentTypeList ',' ExtensionAndException
      {
	$$ = new SequenceType($1, TRUE, NULL);
      }
  | ComponentTypeList ',' ExtensionAndException ',' ComponentTypeList
      {
	$$ = new SequenceType($1, TRUE, $5);
      }
  | ExtensionAndException ',' ComponentTypeList
      {
	$$ = new SequenceType(NULL, TRUE, $3);
      }
  ;

ComponentTypeList
  : ComponentType
      {
	$$ = new TypesList;
	$$->Append($1);
      }
  | ComponentTypeList ',' ComponentType
      {
	$1->Append($3);
      }
  ;

ComponentType
  : NamedType
  | NamedType OPTIONAL_t
      {
	$1->SetOptional();
      }
  | NamedType DEFAULT
      {
	IdentifierTokenContext = $1->GetIdentifierTokenContext();
      }
    Value   
      {
	IdentifierTokenContext = IDENTIFIER;
	$1->SetDefaultValue($4);
      }
  | COMPONENTS OF_t Type
      {
	$$ = $3;
      }
  ;

SequenceValue
  : '{' ComponentValueList '}' 
      {
	$$ = $2;
      }
  | '{'  '}'
      {
	$$ = new ValuesList;
      }
  ;

SetValue
  : '{' ComponentValueList '}' 
      {
	$$ = $2;
      }
  | '{'  '}'
      {
	$$ = new ValuesList;
      }
  ;

ComponentValueList
  : NamedValue
      {
	$$ = new ValuesList;
	$$->Append($1);
      }
  | ComponentValueList ',' NamedValue
      {
	$1->Append($3);
      }
  ;

SequenceOfType
  : SEQUENCE OF_t Type
      {
	$$ = new SequenceOfType($3, NULL);
      }
  ;

SequenceOfValue
  : '{' ValueList '}' 
  | '{'  '}'
  ;

ValueList
  : Value
      { }
  | ValueList ',' Value
      { }
  ;

SetType
  : SET '{' ComponentTypeLists '}' 
      {
	$$ = new SetType((SequenceType*)$3);
      }
  | SET '{'  '}'
      {
	$$ = new SetType;
      }
  ;

SetOfType
  : SET OF_t Type
      {
	$$ = new SetOfType($3, NULL);
      }
  ;

SetOfValue
  : '{' ValueList '}'   
  | '{'  '}'
  ;

ChoiceType
  : CHOICE '{' AlternativeTypeLists '}'
      {
	$$ = $3;
      }
  ;

AlternativeTypeLists
  : AlternativeTypeList
      {
	$$ = new ChoiceType($1);
      }
  | AlternativeTypeList ',' ExtensionAndException
      {
	$$ = new ChoiceType($1, TRUE);
      }
  | AlternativeTypeList ',' ExtensionAndException  ','  AlternativeTypeList
      {
	$$ = new ChoiceType($1, TRUE, $5);
      }
  ;

AlternativeTypeList
  : NamedType	
      {
	$$ = new TypesList;
	$$->Append($1);
      }
  | AlternativeTypeList ',' NamedType
      {
	$1->Append($3);
      }
  ;

ChoiceValue
  : IDENTIFIER ':' Value
      {
	$3->SetValueName($1);
	$$ = $3;
      }
  ;

SelectionType
  : IDENTIFIER '<' Type
      {
	$$ = new SelectionType($1, $3);
      }
  ;

TaggedType
  : Tag Type
      {
	$2->SetTag($1.tagClass, $1.tagNumber, Module->GetDefaultTagMode());
	$$ = $2;
      }
  | Tag IMPLICIT Type  
      {
	$3->SetTag($1.tagClass, $1.tagNumber, Tag::Implicit);
	$$ = $3;
      }
  | Tag EXPLICIT Type
      {
	$3->SetTag($1.tagClass, $1.tagNumber, Tag::Explicit);
	$$ = $3;
      }
  ;

Tag
  : '[' Class ClassNumber ']'
      {
	$$.tagClass = (Tag::Type)$2;
	$$.tagNumber = (int)$3;
      }
  ;

ClassNumber
  : INTEGER 
  | DefinedValue
      {
	if ($1->IsDescendant(IntegerValue::Class()))
	  $$ = *(IntegerValue*)$1;
	else
	  PError << StdError(Fatal) << "incorrect value type." << endl;
      }
  ;

Class
  : UNIVERSAL
      {
	$$ = Tag::Universal;
      }
  | APPLICATION
      {
	$$ = Tag::Application;
      }
  | PRIVATE
      {
	$$ = Tag::Private;
      }
  | /* empty */
      {
	$$ = Tag::ContextSpecific;
      }
  ;

EmbeddedPDVType
  : EMBEDDED PDV
      {
	$$ = new EmbeddedPDVType;
      }
  ;

ExternalType
  : EXTERNAL
      {
	$$ = new ExternalType;
      }
  ;

ObjectIdentifierType
  : OBJECT IDENTIFIER_t
      {
	$$ = new ObjectIdentifierType;
      }
  ;

ObjectIdentifierValue
  : '{' ObjIdComponentList '}'
      {
	$$ = new ObjectIdentifierValue($2);
      }
/*| '{' DefinedValue_OID ObjIdComponentList '}'  /* Context dependence for DefinedValue */
  | OID_IDENTIFIER
      {
	$$ = new ObjectIdentifierValue($1);
      }
  ;

ObjIdComponentList
  : ObjIdComponent
      {
	$$ = new PStringList;
	$$->Append($1);
      }
  | ObjIdComponent ObjIdComponentList
      {
	$2->InsertAt(0, $1);
	$$ = $2;
      }
  ;

ObjIdComponent
  : OID_IDENTIFIER
  | INTEGER
      {
	$$ = new PString(PString::Unsigned, (int)$1);
      }
  | NameAndNumberForm
  ;

NumberForm
  : INTEGER
      {
	$$ = new PString(PString::Unsigned, (int)$1);
      }
  | DefinedValue_OID
  ;

NameAndNumberForm
  : OID_IDENTIFIER '(' NumberForm ')'
      {
	delete $1;
	$$ = $3;
      }
  ;

CharacterStringType
  : RestrictedCharacterStringType
  | UnrestrictedCharacterStringType
  ;

RestrictedCharacterStringType
  : BMPString
      {
	$$ = new BMPStringType;
      }
  | GeneralString
      {
	$$ = new GeneralStringType;
      }
  | GraphicString
      {
	$$ = new GraphicStringType;
      }
  | IA5String
      {
	$$ = new IA5StringType;
      }
  | ISO646String
      {
	$$ = new ISO646StringType;
      }
  | NumericString
      {
	$$ = new NumericStringType;
      }
  | PrintableString
      {
	$$ = new PrintableStringType;
      }
  | TeletexString
      {
	$$ = new TeletexStringType;
      }
  | T61String
      {
	$$ = new T61StringType;
      }
  | UniversalString
      {
	$$ = new UniversalStringType;
      }
  | VideotexString
      {
	$$ = new VideotexStringType;
      }
  | VisibleString
      {
	$$ = new VisibleStringType;
      }
  ;

RestrictedCharacterStringValue
  : CSTRING
      {
	$$ = new CharacterStringValue($1);
      }
  | CharacterStringList
      {
	$$ = new CharacterStringValue($1);
      }
  | Quadruple
      {
	$$ = new CharacterStringValue;
      }
  | Tuple
      {
	$$ = new CharacterStringValue;
      }
  ;

CharacterStringList
  : '{' CharSyms '}'
      {
	$$ = $2;
      }
  ;

CharSyms
  : CharsDefn
      {
	$$ = new PStringList;
	$$->Append($1);
      }
  | CharSyms ',' CharsDefn
      {
	$1->Append($3);
      }
  ;

CharsDefn
  : CSTRING 
  | DefinedValue
      {
	PError << StdError(Warning) << "DefinedValue in string unsupported" << endl;
      }
  ;

Quadruple
  :  '{'  INTEGER  ','  INTEGER  ','  INTEGER  ','  INTEGER '}'
  ;

Tuple
  :  '{' INTEGER ',' INTEGER '}'
  ;

UnrestrictedCharacterStringType
  : CHARACTER STRING
      {
	$$ = new UnrestrictedCharacterStringType;
      }
  ;

CharacterStringValue
  : RestrictedCharacterStringValue
/*| UnrestrictedCharacterStringValue	synonym for SequenceValue */
  ;


/* The following useful types are defined in clauses 39-41: */

UsefulType
  : GeneralizedTime
      {
	$$ = new GeneralizedTimeType;
      }
  | UTCTime
      {
	$$ = new UTCTimeType;
      }
  | ObjectDescriptor_t
      {
	$$ = new ObjectDescriptorType;
      }
  ;


/* The following productions are used in clauses 42-45: */

ConstrainedType
  : TypeWithConstraint
  | Type Constraint
      {
	$1->AddConstraint($2);
      }
  ;

TypeWithConstraint
  : SET Constraint OF_t Type
      {
	$$ = new SetOfType($4, $2);
      }
  | SET SizeConstraint OF_t Type
      {
	$$ = new SetOfType($4, $2);
      }
  | SEQUENCE Constraint OF_t Type
      {
	$$ = new SequenceOfType($4, $2);
      }
  | SEQUENCE SizeConstraint OF_t Type
      {
	$$ = new SequenceOfType($4, $2);
      }
  ;

Constraint
  : '(' ConstraintSpec ExceptionSpec ')'
      {
	$$ = $2;
      }
  ;

ConstraintSpec
  : ElementSetSpecs
/*| GeneralConstraint	  /* specified in ITU-T Rec. X.682 | ISO/IEC 8824-3, subclause 8.1 */ 
  ;

ExceptionSpec
  : '!' ExceptionIdentification 
      {
	$$ = $2;
      }
  | /* empty */
      {
	$$ = NULL;
      }
  ;

ElementSetSpecs
  : ElementSetSpec
      {
	$$ = new Constraint($1, FALSE, NULL);
      }
  | ElementSetSpec  ',' '.' '.' '.'
      {
	$$ = new Constraint($1, TRUE, NULL);
      }
  | '.' '.' '.' ',' ElementSetSpec
      {
	$$ = new Constraint(NULL, TRUE, $5);
      }
  | ElementSetSpec  ',' '.' '.' '.' ElementSetSpec
      {
	$$ = new Constraint($1, TRUE, $6);
      }
  ;

ExceptionIdentification
  : SignedNumber
      {
	$$ = new IntegerValue($1);
      }
  | DefinedValue
  | Type ':' Value
      {
	delete $1;
        PError << StdError(Warning) << "Typed exception unsupported" << endl;
	$$ = $3;
      }
  ;

ElementSetSpec
  : Unions
  | ALL Exclusions
      {
	$$ = new ValueElementList;
	$$->Append(new ConstrainAllValueElement($2));
      }
  ;

Unions
  : Intersections
      {
	$$ = new ValueElementList;
	$$->Append(new ListValueElement($1));
      }
  | Unions UnionMark Intersections
      {
	$1->Append(new ListValueElement($3));
      }
  ;

Intersections
  : IntersectionElements 
      {
	$$ = new ValueElementList;
	$$->Append($1);
      }
  | Intersections IntersectionMark IntersectionElements
      {
	$1->Append($3);
      }
  ;

IntersectionElements
  : Elements 
  | Elements Exclusions
      {
	$1->SetExclusions($2);
      }
  ;

Exclusions
  : EXCEPT Elements
      {
	$$ = $2;
      }
  ;

UnionMark
  : '|'	
  | UNION
  ;

IntersectionMark
  : '^'	
  | INTERSECTION
  ;

Elements
  : SubtypeElements 
/*| ObjectSetElements		/* ITU-T Rec. X.681 | ISO/IEC 8824-2, subclause 12.3 */
  | '(' ElementSetSpec ')'
      {
	$$ = new ListValueElement($2);
      }
  ;

SubtypeElements
  : Value
      {
	$$ = new SingleValueElement($1);
      }
  | ContainedSubtype
      {
	$$ = new SubTypeValueElement($1);
      }
  | ValueRange
  | PermittedAlphabet
  | SizeConstraint
      {
	$$ = new ConstraintValueElement($1);
      }
/*| TypeConstraint	synonym for Type */
  | InnerTypeConstraints
      {
        PError << StdError(Warning) << "InnerTypeConstraints unsupported" << endl;
	$$ = NULL;
      }
  ;

ContainedSubtype
  : INCLUDES Type
      {
	$$ = $2;
      }
/*!!!! Keyword NULL is used both as a value and type causing a cntext dependence here
  | NullType
*/
  ;

ValueRange
  : LowerEndpoint '.' '.' UpperEndpoint
      {
	$$ = new ValueRangeElement($1, $4);
      }
  ;

LowerEndpoint
  : LowerEndValue
  | LowerEndValue '<'
  ;

UpperEndpoint
  : UpperEndValue
  | '<' UpperEndValue
      {
	$$ = $2;
      }
  ;

LowerEndValue
  : Value 
  | MIN
      {
	$$ = new MinValue;
      }
  ;

UpperEndValue
  : Value 
  | MAX
      {
	$$ = new MaxValue;
      }
  ;

SizeConstraint
  : SIZE_t Constraint
      {
	$2->SetPrefix(Constraint::SizePrefix);
	$$ = $2;
      }
  ;

PermittedAlphabet
  : FROM Constraint
      {
	$2->SetPrefix(Constraint::FromPrefix);
	$$ = new ConstraintValueElement($2);
      }
  ;

InnerTypeConstraints
  : WITH COMPONENT Constraint
      {
	$3->SetPrefix(Constraint::WithComponentPrefix);
	$$ = $3;
      }
  | WITH COMPONENTS MultipleTypeConstraints
      {
      }
  ;

MultipleTypeConstraints
  : '{' TypeConstraints '}'
      {
	$$ = $2;
      }
  | '{'  '.' '.' '.' ',' TypeConstraints '}'
      {
	$$ = $6;
      }
  ;

TypeConstraints
  : NamedConstraint
      {
	$$ = new ConstraintList;
	$$->Append($1);
      }
  | NamedConstraint ',' TypeConstraints
      {
	$3->Append($1);
	$$ = $3;
      }
  ;

NamedConstraint
  : IDENTIFIER ComponentConstraint
      {
	$$ = $2;
      }
  ;

ComponentConstraint
  : PresenceConstraint
      {
	$$ = new Constraint(NULL, FALSE, NULL);
      }
  | Constraint PresenceConstraint 
  ;

PresenceConstraint
  : PRESENT 
  | ABSENT 
  | OPTIONAL_t
  | /* empty */
  ;

MacroDefinition
  : TYPEREFERENCE MACRO ASSIGNMENT MacroSubstance
      {
	PError << StdError(Warning) << "MACRO unsupported" << endl;
      }
  ;

MacroSubstance
  : BEGIN_t
      {
	InMacroContext = TRUE;
      }
    MacroBody END
      {
	InMacroContext = FALSE;
      }
  | TYPEREFERENCE
      {}
  | TYPEREFERENCE '.' TYPEREFERENCE
      {}
  ;

MacroBody
  : TypeProduction ValueProduction /*SupportingProductions*/
  ;

TypeProduction
  : TYPE_t NOTATION ASSIGNMENT MacroAlternativeList
  ;

ValueProduction
  : VALUE NOTATION ASSIGNMENT MacroAlternativeList
  ;

/*
SupportingProductions
  : ProductionList
  | /* empty *//*
  ;

ProductionList
  : Production
  | ProductionList Production
  ;

Production
  : TYPEREFERENCE ASSIGNMENT MacroAlternativeList

  ;
*/

MacroAlternativeList
  : MacroAlternative
  | MacroAlternative '|' MacroAlternativeList
  ;

MacroAlternative
  : SymbolElement
  | SymbolElement MacroAlternative
  ;

SymbolElement
  : SymbolDefn
  | EmbeddedDefinitions
  ;

SymbolDefn
  : CSTRING
      {}
  | TYPEREFERENCE
      {}
  | TYPEREFERENCE ASSIGNMENT
      {}
  | string_t
  | identifier_t
  | number_t
  | empty_t
  | type_t
  | type_t '(' TYPE_t TYPEREFERENCE ')'
  | value_t '(' Type ')'
  | value_t '(' IDENTIFIER Type ')'
  | value_t '(' VALUE Type ')'
  ;

EmbeddedDefinitions
  : '<' EmbeddedDefinitionList '>'
  ;

EmbeddedDefinitionList
  : EmbeddedDefinition
  | EmbeddedDefinitionList EmbeddedDefinition
  ;

EmbeddedDefinition
  : LocalTypeAssignment
  | LocalValueAssignment
  ;

LocalTypeAssignment
  : TYPEREFERENCE ASSIGNMENT Type
      {}
  ;

LocalValueAssignment
  : IDENTIFIER Type ASSIGNMENT Value
      {}
  ;


ObjectTypeDefinition
  : IDENTIFIER OBJECT_TYPE
      {
	InMIBContext = TRUE;
      }
    SYNTAX Type
    ACCESS ObjectTypeAccess
    STATUS ObjectTypeStatus
    MibDescrPart
    MibReferPart
    MibIndexPart
    MibDefValPart
      {
	IdentifierTokenContext = OID_IDENTIFIER;
      }
    ASSIGNMENT Value
      {
	Module->AddMIB(new MibObject($1, $5, (MibObject::Access)$7, (MibObject::Status)$9, $10, $11, $12, $13, $16));
	InMIBContext = FALSE;
	IdentifierTokenContext = IDENTIFIER;
      }
  ;

ObjectTypeAccess
  : read_only_t
      {
	$$ = MibObject::read_only;
      }
  | read_write_t
      {
	$$ = MibObject::read_write;
      }
  | write_only_t
      {
	$$ = MibObject::write_only;
      }
  | not_accessible_t
      {
	$$ = MibObject::not_accessible;
      }
  ;

ObjectTypeStatus
  : mandatory_t
      {
	$$ = MibObject::mandatory;
      }
  | optional_t
      {
	$$ = MibObject::optional;
      }
  | obsolete_t
      {
	$$ = MibObject::obsolete;
      }
  | deprecated_t
      {
	$$ = MibObject::deprecated;
      }
  ;

MibDescrPart
  : DESCRIPTION_t CSTRING
      {
	$$ = $2;
      }
  | /* empty */
      {
	$$ = NULL;
      }
  ;

MibReferPart
  : REFERENCE_t CSTRING
      {
	$$ = $2;
      }
  | /* empty */
      {
	$$ = NULL;
      }
  ;

MibIndexPart
  : INDEX_t '{' MibIndexTypes '}'
      {
	$$ = $3;
      }
  | /* empty */
      {
	$$ = NULL;
      }
  ;

MibIndexTypes
  : MibIndexType
      {
	$$ = new PStringList;
	$$->Append($1);
      }
  | MibIndexTypes ',' MibIndexType
      {
	$1->Append($3);
      }
  ;

MibIndexType
  : IDENTIFIER
  | TYPEREFERENCE
  ;

MibDefValPart
  : DEFVAL_t '{' Value '}'
      {
	$$ = $3;
      }
  | /* empty */
      {
	$$ = NULL;
      }
  ;

TrapTypeDefinition
  : IDENTIFIER TRAP_TYPE
      {
	InMIBContext = TRUE;
	IdentifierTokenContext = OID_IDENTIFIER;
      }
    ENTERPRISE Value
    MibVarPart
    MibDescrPart
    MibReferPart
    ASSIGNMENT Value
      {
	Module->AddMIB(new MibTrap($1, $5, $6, $7, $8, $10));
	IdentifierTokenContext = IDENTIFIER;
	InMIBContext = FALSE;
      }
  ;

MibVarPart
  : VARIABLES '{' MibVarTypes '}'
      {
	$$ = $3;
      }
  | /* empty */
      {
	$$ = NULL;
      }
  ;

MibVarTypes
  : Value
      {
	$$ = new ValuesList;
	$$->Append($1);
      }
  | MibVarTypes ',' Value
      {
	$1->Append($3);
      }
  ;

