#ifndef _MAIN_H
#define _MAIN_H

extern unsigned lineNumber;
extern PString  fileName;
extern FILE * yyin;
extern int yyparse();

void yyerror(char * str);


/////////////////////////////////////////
//
//  standard error output from parser
//

enum StdErrorType { Warning, Fatal };

class StdError {
  public:
    StdError(StdErrorType ne) : e(ne) { }
    //StdError(StdErrorType ne, unsigned ln) : e(ne), l(ln) { }
    friend ostream & operator<<(ostream & out, const StdError & e);

  protected:
    StdErrorType e;
    //unsigned     l;
};


/////////////////////////////////////////
//
//  intermediate structures from parser
//


class NamedNumber : public PObject
{
    PCLASSINFO(NamedNumber, PObject)
  public:
    NamedNumber(PString * nam);
    NamedNumber(PString * nam, int num);
    NamedNumber(PString * nam, const PString & ref);
    void PrintOn(ostream &) const;

    void SetAutoNumber(const NamedNumber & prev);
    const PString & GetName() const { return name; }
    int GetNumber() const { return number; }

  protected:
    PString name;
    PString reference;
    int number;
    BOOL autonumber;
};

PLIST(NamedNumberList, NamedNumber);


// Types

class Tag : public PObject
{
    PCLASSINFO(Tag, PObject)
  public:
    enum Type {
      Universal,
      Application,
      ContextSpecific,
      Private
    };
    enum UniversalTags {
      IllegalUniversalTag,
      UniversalBoolean,
      UniversalInteger,
      UniversalBitString,
      UniversalOctetString,
      UniversalNull,
      UniversalObjectId,
      UniversalObjectDescriptor,
      UniversalExternalType,
      UniversalReal,
      UniversalEnumeration,
      UniversalEmbeddedPDV,
      UniversalSequence = 16,
      UniversalSet,
      UniversalNumericString,
      UniversalPrintableString,
      UniversalTeletexString,
      UniversalVideotexString,
      UniversalIA5String,
      UniversalUTCTime,
      UniversalGeneralisedTime,
      UniversalGraphicString,
      UniversalVisibleString,
      UniversalGeneralString,
      UniversalUniversalString,
      UniversalBMPString = 30
    };
    enum Mode {
      Implicit,
      Explicit,
      Automatic
    };
    Tag(unsigned tagNum);

    void PrintOn(ostream &) const;

    Type type;
    unsigned number;
    Mode mode;

    static const char * classNames[];
    static const char * modeNames[];
};


class ValueElementBase;

PLIST(ValueElementList, ValueElementBase);


class Constraint : public PObject
{
    PCLASSINFO(Constraint, PObject)
  public:
    Constraint(ValueElementList * std, BOOL extend, ValueElementList * ext);
    void PrintOn(ostream &) const;
    enum Prefix {
      NoPrefix,
      SizePrefix,
      FromPrefix,
      WithComponentPrefix
    };
    void SetPrefix(Prefix pref) { prefix = pref; }
    BOOL HasPrefix(Prefix pref);
    BOOL IsExtendable() const { return extendable; }

    virtual void GenerateCplusplus(ostream & hdr, ostream & cxx);

  protected:
    ValueElementList standard;
    BOOL extendable;
    ValueElementList extensions;
    Prefix prefix;
};

PLIST(ConstraintList, Constraint);


class ValueElementBase : public PObject
{
    PCLASSINFO(ValueElementBase, PObject)
  public:
    ValueElementBase();
    void SetExclusions(ValueElementBase * excl) { exclusions = excl; }

    virtual void GenerateCplusplus(const PString & fn, ostream & hdr, ostream & cxx);
    virtual BOOL HasPrefix(Constraint::Prefix pref);

  protected:
    ValueElementBase * exclusions;
};


class ConstrainAllValueElement : public ValueElementBase
{
    PCLASSINFO(ConstrainAllValueElement, ValueElementBase)
  public:
    ConstrainAllValueElement(ValueElementBase * excl);
};



class ListValueElement : public ValueElementBase
{
    PCLASSINFO(ListValueElement, ValueElementBase)
  public:
    ListValueElement(ValueElementList * list);
    void PrintOn(ostream &) const;

    virtual void GenerateCplusplus(const PString & fn, ostream & hdr, ostream & cxx);
    virtual BOOL HasPrefix(Constraint::Prefix pref);

  protected:
    ValueElementList elements;
};


class ValueBase;

class SingleValueElement : public ValueElementBase
{
    PCLASSINFO(SingleValueElement, ValueElementBase)
  public:
    SingleValueElement(ValueBase * val);
    ~SingleValueElement();
    void PrintOn(ostream &) const;

    virtual void GenerateCplusplus(const PString & fn, ostream & hdr, ostream & cxx);

  protected:
    ValueBase * value;
};


class ValueRangeElement : public ValueElementBase
{
    PCLASSINFO(ValueRangeElement, ValueElementBase)
  public:
    ValueRangeElement(ValueBase * lowerBound, ValueBase * upperBound);
    ~ValueRangeElement();
    void PrintOn(ostream &) const;

    virtual void GenerateCplusplus(const PString & fn, ostream & hdr, ostream & cxx);

  protected:
    ValueBase * lower;
    ValueBase * upper;
};


class TypeBase;

class SubTypeValueElement : public ValueElementBase
{
    PCLASSINFO(SubTypeValueElement, ValueElementBase)
  public:
    SubTypeValueElement(TypeBase * typ);
    ~SubTypeValueElement();
  protected:
    TypeBase * type;
};


class ConstraintValueElement : public ValueElementBase
{
    PCLASSINFO(ConstraintValueElement, ValueElementBase)
  public:
    ConstraintValueElement(Constraint * con);
    ~ConstraintValueElement();
    void PrintOn(ostream &) const;

    virtual void GenerateCplusplus(const PString & fn, ostream & hdr, ostream & cxx);
    virtual BOOL HasPrefix(Constraint::Prefix pref);

  protected:
    Constraint * constraint;
};


class TypeBase;

PLIST(TypesList, TypeBase);

class TypeBase : public PObject
{
    PCLASSINFO(TypeBase, PObject)
  public:
    void PrintOn(ostream &) const;

    virtual int GetIdentifierTokenContext() const;
    virtual int GetBraceTokenContext() const;

    const PString & GetName() const { return name; }
    void SetName(PString * name);
    PString GetIdentifier() const;
    void SetTag(Tag::Type cls, unsigned num, Tag::Mode mode);
    const Tag & GetTag() const { return tag; }
    BOOL HasNonStandardTag() const { return tag != defaultTag; }
    void AddConstraint(Constraint * constraint) { constraints.Append(constraint); }
    BOOL HasConstraints() const { return constraints.GetSize() > 0; }
    BOOL IsOptional() const { return isOptional; }
    void SetOptional() { isOptional = TRUE; }
    void SetDefaultValue(ValueBase * value) { defaultValue = value; }

    virtual void FlattenUsedTypes(TypesList & types);
    virtual TypeBase * FlattenThisType(const TypeBase & parent, TypesList & types);
    virtual BOOL IsChoice() const;
    virtual void GenerateCplusplus(ostream & hdr, ostream & cxx);
    virtual void GenerateOperators(ostream & hdr, ostream & cxx, const PString & className);
    virtual const char * GetAncestorClass() const = 0;
    virtual PString GetTypeName() const;
    virtual BOOL References(const TypeBase & type) const;
    virtual void ResolveReference(TypesList & types);

    BOOL IsGenerated() const { return isGenerated; }
    void BeginGenerateCplusplus(ostream & hdr, ostream & cxx);
    void EndGenerateCplusplus(ostream & hdr, ostream & cxx);
    void GenerateCplusplusConstructor(ostream & hdr, ostream & cxx);
    void GenerateCplusplusConstraints(const PString & prefix, ostream & hdr, ostream & cxx);

  protected:
    TypeBase(unsigned tagNum);
    TypeBase(const PString & nam, const Tag & theTag, BOOL opt);

    void PrintStart(ostream &) const;
    void PrintFinish(ostream &) const;

    PString name;
    Tag tag;
    Tag defaultTag;
    ConstraintList constraints;
    BOOL isOptional;
    ValueBase * defaultValue;
    BOOL isGenerated;
};


class DefinedType : public TypeBase
{
    PCLASSINFO(DefinedType, TypeBase)
  public:
    DefinedType(PString * name);
    DefinedType(TypeBase * refType, const TypeBase & parent, TypesList & types);
    void PrintOn(ostream &) const;
    virtual void FlattenUsedTypes(TypesList & types);
    virtual TypeBase * FlattenThisType(const TypeBase & parent, TypesList & types);
    virtual BOOL IsChoice() const;
    virtual void GenerateOperators(ostream & hdr, ostream & cxx, const PString & className);
    virtual const char * GetAncestorClass() const;
    virtual PString GetTypeName() const;
    virtual BOOL References(const TypeBase & type) const;
    virtual void ResolveReference(TypesList & types);
  protected:

    PString referenceName;
    TypeBase * baseType;
    BOOL resolved;
};


class SelectionType : public TypeBase
{
    PCLASSINFO(SelectionType, TypeBase)
  public:
    SelectionType(PString * name, TypeBase * base);
    ~SelectionType();
    void PrintOn(ostream &) const;
    virtual void FlattenUsedTypes(TypesList & types);
    virtual TypeBase * FlattenThisType(const TypeBase & parent, TypesList & types);
    virtual void GenerateCplusplus(ostream & hdr, ostream & cxx);
    virtual const char * GetAncestorClass() const;
    virtual BOOL References(const TypeBase & type) const;
  protected:
    PString selection;
    TypeBase * baseType;
};


class BooleanType : public TypeBase
{
    PCLASSINFO(BooleanType, TypeBase)
  public:
    BooleanType();
    virtual void GenerateOperators(ostream & hdr, ostream & cxx, const PString & className);
    virtual const char * GetAncestorClass() const;
};


class IntegerType : public TypeBase
{
    PCLASSINFO(IntegerType, TypeBase)
  public:
    IntegerType();
    IntegerType(NamedNumberList *);
    virtual void GenerateOperators(ostream & hdr, ostream & cxx, const PString & className);
    virtual const char * GetAncestorClass() const;
  protected:
    NamedNumberList allowedValues;
};


class EnumeratedType : public TypeBase
{
    PCLASSINFO(EnumeratedType, TypeBase)
  public:
    EnumeratedType(NamedNumberList * enums, BOOL extend, NamedNumberList * ext);
    void PrintOn(ostream &) const;
    virtual TypeBase * FlattenThisType(const TypeBase & parent, TypesList & types);
    virtual void GenerateCplusplus(ostream & hdr, ostream & cxx);
    virtual void GenerateOperators(ostream & hdr, ostream & cxx, const PString & className);
    virtual const char * GetAncestorClass() const;
  protected:
    NamedNumberList enumerations;
    PINDEX numEnums;
    BOOL extendable;
};


class RealType : public TypeBase
{
    PCLASSINFO(RealType, TypeBase)
  public:
    RealType();
    virtual const char * GetAncestorClass() const;
};


class BitStringType : public TypeBase
{
    PCLASSINFO(BitStringType, TypeBase)
  public:
    BitStringType();
    BitStringType(NamedNumberList *);
    virtual int GetIdentifierTokenContext() const;
    virtual int GetBraceTokenContext() const;
    virtual const char * GetAncestorClass() const;
  protected:
    NamedNumberList allowedBits;
};


class OctetStringType : public TypeBase
{
    PCLASSINFO(OctetStringType, TypeBase)
  public:
    OctetStringType();
    virtual void GenerateOperators(ostream & hdr, ostream & cxx, const PString & className);
    virtual const char * GetAncestorClass() const;
};


class NullType : public TypeBase
{
    PCLASSINFO(NullType, TypeBase)
  public:
    NullType();
    virtual const char * GetAncestorClass() const;
};


class SequenceType : public TypeBase
{
    PCLASSINFO(SequenceType, TypeBase)
    void PrintOn(ostream &) const;
  public:
    SequenceType(TypesList * std,
                 BOOL extendable,
                 TypesList * extensions,
                 unsigned tagNum = Tag::UniversalSequence);
    virtual void FlattenUsedTypes(TypesList & types);
    virtual TypeBase * FlattenThisType(const TypeBase & parent, TypesList & types);
    virtual void GenerateCplusplus(ostream & hdr, ostream & cxx);
    virtual const char * GetAncestorClass() const;
    virtual BOOL References(const TypeBase & type) const;
  protected:
    TypesList fields;
    PINDEX numFields;
    BOOL extendable;
};


class SequenceOfType : public TypeBase
{
    PCLASSINFO(SequenceOfType, TypeBase)
  public:
    SequenceOfType(TypeBase * base, Constraint * constraint, unsigned tag = Tag::UniversalSequence);
    ~SequenceOfType();
    void PrintOn(ostream &) const;
    virtual void FlattenUsedTypes(TypesList & types);
    virtual TypeBase * FlattenThisType(const TypeBase & parent, TypesList & types);
    virtual void GenerateCplusplus(ostream & hdr, ostream & cxx);
    virtual const char * GetAncestorClass() const;
  protected:
    TypeBase * baseType;
};


class SetType : public SequenceType
{
    PCLASSINFO(SetType, SequenceType)
  public:
    SetType();
    SetType(SequenceType * seq);
    virtual const char * GetAncestorClass() const;
};


class SetOfType : public SequenceOfType
{
    PCLASSINFO(SetOfType, SequenceOfType)
  public:
    SetOfType(TypeBase * base, Constraint * constraint);
};


class ChoiceType : public SequenceType
{
    PCLASSINFO(ChoiceType, SequenceType)
  public:
    ChoiceType(TypesList * std = NULL,
               BOOL extendable = FALSE,
               TypesList * extensions = NULL);
    virtual void GenerateCplusplus(ostream & hdr, ostream & cxx);
    virtual BOOL IsChoice() const;
    virtual const char * GetAncestorClass() const;
    virtual BOOL References(const TypeBase & type) const;
};


class EmbeddedPDVType : public TypeBase
{
    PCLASSINFO(EmbeddedPDVType, TypeBase)
  public:
    EmbeddedPDVType();
    virtual const char * GetAncestorClass() const;
};


class ExternalType : public TypeBase
{
    PCLASSINFO(ExternalType, TypeBase)
  public:
    ExternalType();
    virtual const char * GetAncestorClass() const;
};


class StringTypeBase : public TypeBase
{
    PCLASSINFO(StringTypeBase, TypeBase)
  public:
    StringTypeBase(int tag);
    virtual int GetBraceTokenContext() const;
};


class BMPStringType : public StringTypeBase
{
    PCLASSINFO(BMPStringType, StringTypeBase)
  public:
    BMPStringType();
    virtual const char * GetAncestorClass() const;
};


class GeneralStringType : public StringTypeBase
{
    PCLASSINFO(GeneralStringType, StringTypeBase)
  public:
    GeneralStringType();
    virtual const char * GetAncestorClass() const;
};


class GraphicStringType : public StringTypeBase
{
    PCLASSINFO(GraphicStringType, StringTypeBase)
  public:
    GraphicStringType();
    virtual const char * GetAncestorClass() const;
};


class IA5StringType : public StringTypeBase
{
    PCLASSINFO(IA5StringType, StringTypeBase)
  public:
    IA5StringType();
    virtual const char * GetAncestorClass() const;
};


class ISO646StringType : public StringTypeBase
{
    PCLASSINFO(ISO646StringType, StringTypeBase)
  public:
    ISO646StringType();
    virtual const char * GetAncestorClass() const;
};


class NumericStringType : public StringTypeBase
{
    PCLASSINFO(NumericStringType, StringTypeBase)
  public:
    NumericStringType();
    virtual const char * GetAncestorClass() const;
};


class PrintableStringType : public StringTypeBase
{
    PCLASSINFO(PrintableStringType, StringTypeBase)
  public:
    PrintableStringType();
    virtual const char * GetAncestorClass() const;
};


class TeletexStringType : public StringTypeBase
{
    PCLASSINFO(TeletexStringType, StringTypeBase)
  public:
    TeletexStringType();
    virtual const char * GetAncestorClass() const;
};


class T61StringType : public StringTypeBase
{
    PCLASSINFO(T61StringType, StringTypeBase)
  public:
    T61StringType();
    virtual const char * GetAncestorClass() const;
};


class UniversalStringType : public StringTypeBase
{
    PCLASSINFO(UniversalStringType, StringTypeBase)
  public:
    UniversalStringType();
    virtual const char * GetAncestorClass() const;
};


class VideotexStringType : public StringTypeBase
{
    PCLASSINFO(VideotexStringType, StringTypeBase)
  public:
    VideotexStringType();
    virtual const char * GetAncestorClass() const;
};


class VisibleStringType : public StringTypeBase
{
    PCLASSINFO(VisibleStringType, StringTypeBase)
  public:
    VisibleStringType();
    virtual const char * GetAncestorClass() const;
};


class UnrestrictedCharacterStringType : public StringTypeBase
{
    PCLASSINFO(UnrestrictedCharacterStringType, StringTypeBase)
  public:
    UnrestrictedCharacterStringType();
    virtual const char * GetAncestorClass() const;
};


class GeneralizedTimeType : public TypeBase
{
    PCLASSINFO(GeneralizedTimeType, TypeBase)
  public:
    GeneralizedTimeType();
    virtual const char * GetAncestorClass() const;
};


class UTCTimeType : public TypeBase
{
    PCLASSINFO(UTCTimeType, TypeBase)
  public:
    UTCTimeType();
    virtual const char * GetAncestorClass() const;
};


class ObjectDescriptorType : public TypeBase
{
    PCLASSINFO(ObjectDescriptorType, TypeBase)
  public:
    ObjectDescriptorType();
    virtual const char * GetAncestorClass() const;
};


class ObjectIdentifierType : public TypeBase
{
    PCLASSINFO(ObjectIdentifierType, TypeBase)
  public:
    ObjectIdentifierType();
    virtual int GetIdentifierTokenContext() const;
    virtual const char * GetAncestorClass() const;
};


// Values

class ValueBase : public PObject
{
    PCLASSINFO(ValueBase, PObject)
  public:
    void SetValueName(PString * name);
    const PString & GetName() const { return valueName; }

    virtual void GenerateCplusplus(ostream & hdr, ostream & cxx);

  protected:
    void PrintBase(ostream &) const;
    PString valueName;
};

PLIST(ValuesList, ValueBase);


class DefinedValue : public ValueBase
{
    PCLASSINFO(DefinedValue, ValueBase)
  public:
    DefinedValue(PString * name);
    void PrintOn(ostream &) const;
    const PString & GetReference() const { return referenceName; }
    virtual void GenerateCplusplus(ostream & hdr, ostream & cxx);
  protected:
    PString referenceName;
    ValueBase * actualValue;
    BOOL resolved;
};


class BooleanValue : public ValueBase
{
    PCLASSINFO(BooleanValue, ValueBase)
  public:
    BooleanValue(BOOL newVal);
    void PrintOn(ostream &) const;
    virtual void GenerateCplusplus(ostream & hdr, ostream & cxx);
  protected:
    BOOL value;
};


class IntegerValue : public ValueBase
{
    PCLASSINFO(IntegerValue, ValueBase)
  public:
    IntegerValue(PInt64 newVal);
    void PrintOn(ostream &) const;
    virtual void GenerateCplusplus(ostream & hdr, ostream & cxx);

    operator PInt64() const { return value; }
    operator long() const { return (long)value; }

  protected:
    PInt64 value;
};


class RealValue : public ValueBase
{
    PCLASSINFO(RealValue, ValueBase)
  public:
    RealValue(double newVal);
  protected:
    double value;
};


class BitStringValue : public ValueBase
{
    PCLASSINFO(BitStringValue, ValueBase)
  public:
    BitStringValue() { }
    BitStringValue(PString * newVal);
    BitStringValue(PStringList * newVal);
  protected:
    PBYTEArray value;
};


class NullValue : public ValueBase
{
    PCLASSINFO(NullValue, ValueBase)
};


class CharacterValue : public ValueBase
{
    PCLASSINFO(CharacterValue, ValueBase)
  public:
    CharacterValue(BYTE c);
    CharacterValue(BYTE t1, BYTE t2);
    CharacterValue(BYTE q1, BYTE q2, BYTE q3, BYTE q4);
    void PrintOn(ostream &) const;
    virtual void GenerateCplusplus(ostream & hdr, ostream & cxx);
  protected:
    unsigned value;
};


class CharacterStringValue : public ValueBase
{
    PCLASSINFO(CharacterStringValue, ValueBase)
  public:
    CharacterStringValue() { }
    CharacterStringValue(PString * newVal);
    CharacterStringValue(PStringList * newVal);
    void PrintOn(ostream &) const;
    virtual void GenerateCplusplus(ostream & hdr, ostream & cxx);
  protected:
    PString value;
};


class ObjectIdentifierValue : public ValueBase
{
    PCLASSINFO(ObjectIdentifierValue, ValueBase)
  public:
    ObjectIdentifierValue(PString * newVal);
    ObjectIdentifierValue(PStringList * newVal);
    void PrintOn(ostream &) const;
  protected:
    PStringList value;
};


class MinValue : public ValueBase
{
    PCLASSINFO(MinValue, ValueBase)
  public:
    void PrintOn(ostream &) const;
    virtual void GenerateCplusplus(ostream & hdr, ostream & cxx);
};


class MaxValue : public ValueBase
{
    PCLASSINFO(MaxValue, ValueBase)
  public:
    void PrintOn(ostream &) const;
    virtual void GenerateCplusplus(ostream & hdr, ostream & cxx);
};


class SequenceValue : public ValueBase
{
    PCLASSINFO(SequenceValue, ValueBase)
  public:
    SequenceValue(ValuesList * list = NULL);
    void PrintOn(ostream &) const;
  protected:
    ValuesList values;
};


class MibBase : public PObject
{
    PCLASSINFO(MibBase, PObject)
  public:
    MibBase(PString * name, PString * descr, PString * refer, ValueBase * val);
    virtual ~MibBase();
  protected:
    PString name;
    PString description;
    PString reference;
    ValueBase * value;
};

PLIST(MibList, MibBase);


class MibObject : public MibBase
{
    PCLASSINFO(MibObject, MibBase)
  public:
    enum Access {
      read_only,
      read_write,
      write_only,
      not_accessible,
    };
    enum Status {
      mandatory,
      optional,
      obsolete,
      deprecated
    };
    MibObject(PString * name, TypeBase * type, Access acc, Status stat,
              PString * descr, PString * refer, PStringList * idx,
              ValueBase * defVal,
              ValueBase * setVal);
    ~MibObject();
    void PrintOn(ostream &) const;
  protected:
    TypeBase * type;
    Access access;
    Status status;
    PStringList index;
    ValueBase * defaultValue;
};


class MibTrap : public MibBase
{
    PCLASSINFO(MibTrap, MibBase)
  public:
    MibTrap(PString * nam, ValueBase * ent, ValuesList * var,
            PString * descr, PString * refer, ValueBase * val);
    ~MibTrap();
    void PrintOn(ostream &) const;
  protected:
    ValueBase * enterprise;
    ValuesList variables;
};


class ImportModule : public PObject
{
    PCLASSINFO(ImportModule, PObject)
  public:
    ImportModule(PString * name, PStringList * syms);

    void PrintOn(ostream &) const;

  protected:
    PString moduleName;
    PStringList symbols;
};

PLIST(ImportsList, ImportModule);


class ModuleDefinition : public PObject
{
    PCLASSINFO(ModuleDefinition, PObject)
  public:
    ModuleDefinition(PString * name, PStringList * id, Tag::Mode defTagMode);

    void PrintOn(ostream &) const;

    Tag::Mode GetDefaultTagMode() const { return defaultTagMode; }

    void SetExportAll();
    void SetExports(PStringList * syms);

    void AddImport(ImportModule * mod) { imports.Append(mod); }
    void AddType(TypeBase * type) { types.Append(type); }
    void AddValue(ValueBase * val) { values.Append(val); }
    void AddMIB(MibBase * mib) { mibs.Append(mib); }

    const TypesList & GetTypes() const { return types; }
    const ValuesList & GetValues() const { return values; }

    int GetIndentLevel() const { return indentLevel; }
    void SetIndentLevel(int delta) { indentLevel += delta; }

    void GenerateCplusplus(ostream & hdr, ostream & cxx);

    PString moduleName;

  protected:
    PStringList definitiveId;
    Tag::Mode defaultTagMode;
    PStringList exports;
    BOOL exportAll;
    ImportsList imports;
    TypesList types;
    ValuesList values;
    MibList mibs;

    int indentLevel;
};


extern ModuleDefinition * Module;


#endif
