//
// Generated file, do not edit! Created by opp_msgtool 6.0 from InitMessage.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wshadow"
#  pragma clang diagnostic ignored "-Wconversion"
#  pragma clang diagnostic ignored "-Wunused-parameter"
#  pragma clang diagnostic ignored "-Wc++98-compat"
#  pragma clang diagnostic ignored "-Wunreachable-code-break"
#  pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wsuggest-attribute=noreturn"
#  pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif

#include <iostream>
#include <sstream>
#include <memory>
#include <type_traits>
#include "InitMessage_m.h"

namespace omnetpp {

// Template pack/unpack rules. They are declared *after* a1l type-specific pack functions for multiple reasons.
// They are in the omnetpp namespace, to allow them to be found by argument-dependent lookup via the cCommBuffer argument

// Packing/unpacking an std::vector
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::vector<T,A>& v)
{
    int n = v.size();
    doParsimPacking(buffer, n);
    for (int i = 0; i < n; i++)
        doParsimPacking(buffer, v[i]);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::vector<T,A>& v)
{
    int n;
    doParsimUnpacking(buffer, n);
    v.resize(n);
    for (int i = 0; i < n; i++)
        doParsimUnpacking(buffer, v[i]);
}

// Packing/unpacking an std::list
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::list<T,A>& l)
{
    doParsimPacking(buffer, (int)l.size());
    for (typename std::list<T,A>::const_iterator it = l.begin(); it != l.end(); ++it)
        doParsimPacking(buffer, (T&)*it);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::list<T,A>& l)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        l.push_back(T());
        doParsimUnpacking(buffer, l.back());
    }
}

// Packing/unpacking an std::set
template<typename T, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::set<T,Tr,A>& s)
{
    doParsimPacking(buffer, (int)s.size());
    for (typename std::set<T,Tr,A>::const_iterator it = s.begin(); it != s.end(); ++it)
        doParsimPacking(buffer, *it);
}

template<typename T, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::set<T,Tr,A>& s)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        T x;
        doParsimUnpacking(buffer, x);
        s.insert(x);
    }
}

// Packing/unpacking an std::map
template<typename K, typename V, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::map<K,V,Tr,A>& m)
{
    doParsimPacking(buffer, (int)m.size());
    for (typename std::map<K,V,Tr,A>::const_iterator it = m.begin(); it != m.end(); ++it) {
        doParsimPacking(buffer, it->first);
        doParsimPacking(buffer, it->second);
    }
}

template<typename K, typename V, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::map<K,V,Tr,A>& m)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        K k; V v;
        doParsimUnpacking(buffer, k);
        doParsimUnpacking(buffer, v);
        m[k] = v;
    }
}

// Default pack/unpack function for arrays
template<typename T>
void doParsimArrayPacking(omnetpp::cCommBuffer *b, const T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimPacking(b, t[i]);
}

template<typename T>
void doParsimArrayUnpacking(omnetpp::cCommBuffer *b, T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimUnpacking(b, t[i]);
}

// Default rule to prevent compiler from choosing base class' doParsimPacking() function
template<typename T>
void doParsimPacking(omnetpp::cCommBuffer *, const T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimPacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

template<typename T>
void doParsimUnpacking(omnetpp::cCommBuffer *, T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimUnpacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

}  // namespace omnetpp

Register_Class(InitMessage)

InitMessage::InitMessage(const char *name, short kind) : ::omnetpp::cPacket(name, kind)
{
}

InitMessage::InitMessage(const InitMessage& other) : ::omnetpp::cPacket(other)
{
    copy(other);
}

InitMessage::~InitMessage()
{
}

InitMessage& InitMessage::operator=(const InitMessage& other)
{
    if (this == &other) return *this;
    ::omnetpp::cPacket::operator=(other);
    copy(other);
    return *this;
}

void InitMessage::copy(const InitMessage& other)
{
    this->WS = other.WS;
    this->TO = other.TO;
    this->PT = other.PT;
    this->TD = other.TD;
    this->ED = other.ED;
    this->DD = other.DD;
    this->LP = other.LP;
    this->startTime = other.startTime;
    this->startingNode = other.startingNode;
}

void InitMessage::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cPacket::parsimPack(b);
    doParsimPacking(b,this->WS);
    doParsimPacking(b,this->TO);
    doParsimPacking(b,this->PT);
    doParsimPacking(b,this->TD);
    doParsimPacking(b,this->ED);
    doParsimPacking(b,this->DD);
    doParsimPacking(b,this->LP);
    doParsimPacking(b,this->startTime);
    doParsimPacking(b,this->startingNode);
}

void InitMessage::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->WS);
    doParsimUnpacking(b,this->TO);
    doParsimUnpacking(b,this->PT);
    doParsimUnpacking(b,this->TD);
    doParsimUnpacking(b,this->ED);
    doParsimUnpacking(b,this->DD);
    doParsimUnpacking(b,this->LP);
    doParsimUnpacking(b,this->startTime);
    doParsimUnpacking(b,this->startingNode);
}

int InitMessage::getWS() const
{
    return this->WS;
}

void InitMessage::setWS(int WS)
{
    this->WS = WS;
}

double InitMessage::getTO() const
{
    return this->TO;
}

void InitMessage::setTO(double TO)
{
    this->TO = TO;
}

double InitMessage::getPT() const
{
    return this->PT;
}

void InitMessage::setPT(double PT)
{
    this->PT = PT;
}

double InitMessage::getTD() const
{
    return this->TD;
}

void InitMessage::setTD(double TD)
{
    this->TD = TD;
}

double InitMessage::getED() const
{
    return this->ED;
}

void InitMessage::setED(double ED)
{
    this->ED = ED;
}

double InitMessage::getDD() const
{
    return this->DD;
}

void InitMessage::setDD(double DD)
{
    this->DD = DD;
}

double InitMessage::getLP() const
{
    return this->LP;
}

void InitMessage::setLP(double LP)
{
    this->LP = LP;
}

int InitMessage::getStartTime() const
{
    return this->startTime;
}

void InitMessage::setStartTime(int startTime)
{
    this->startTime = startTime;
}

bool InitMessage::getStartingNode() const
{
    return this->startingNode;
}

void InitMessage::setStartingNode(bool startingNode)
{
    this->startingNode = startingNode;
}

class InitMessageDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_WS,
        FIELD_TO,
        FIELD_PT,
        FIELD_TD,
        FIELD_ED,
        FIELD_DD,
        FIELD_LP,
        FIELD_startTime,
        FIELD_startingNode,
    };
  public:
    InitMessageDescriptor();
    virtual ~InitMessageDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(InitMessageDescriptor)

InitMessageDescriptor::InitMessageDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(InitMessage)), "omnetpp::cPacket")
{
    propertyNames = nullptr;
}

InitMessageDescriptor::~InitMessageDescriptor()
{
    delete[] propertyNames;
}

bool InitMessageDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<InitMessage *>(obj)!=nullptr;
}

const char **InitMessageDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *InitMessageDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int InitMessageDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 9+base->getFieldCount() : 9;
}

unsigned int InitMessageDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_WS
        FD_ISEDITABLE,    // FIELD_TO
        FD_ISEDITABLE,    // FIELD_PT
        FD_ISEDITABLE,    // FIELD_TD
        FD_ISEDITABLE,    // FIELD_ED
        FD_ISEDITABLE,    // FIELD_DD
        FD_ISEDITABLE,    // FIELD_LP
        FD_ISEDITABLE,    // FIELD_startTime
        FD_ISEDITABLE,    // FIELD_startingNode
    };
    return (field >= 0 && field < 9) ? fieldTypeFlags[field] : 0;
}

const char *InitMessageDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "WS",
        "TO",
        "PT",
        "TD",
        "ED",
        "DD",
        "LP",
        "startTime",
        "startingNode",
    };
    return (field >= 0 && field < 9) ? fieldNames[field] : nullptr;
}

int InitMessageDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "WS") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "TO") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "PT") == 0) return baseIndex + 2;
    if (strcmp(fieldName, "TD") == 0) return baseIndex + 3;
    if (strcmp(fieldName, "ED") == 0) return baseIndex + 4;
    if (strcmp(fieldName, "DD") == 0) return baseIndex + 5;
    if (strcmp(fieldName, "LP") == 0) return baseIndex + 6;
    if (strcmp(fieldName, "startTime") == 0) return baseIndex + 7;
    if (strcmp(fieldName, "startingNode") == 0) return baseIndex + 8;
    return base ? base->findField(fieldName) : -1;
}

const char *InitMessageDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",    // FIELD_WS
        "double",    // FIELD_TO
        "double",    // FIELD_PT
        "double",    // FIELD_TD
        "double",    // FIELD_ED
        "double",    // FIELD_DD
        "double",    // FIELD_LP
        "int",    // FIELD_startTime
        "bool",    // FIELD_startingNode
    };
    return (field >= 0 && field < 9) ? fieldTypeStrings[field] : nullptr;
}

const char **InitMessageDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *InitMessageDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int InitMessageDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    InitMessage *pp = omnetpp::fromAnyPtr<InitMessage>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void InitMessageDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    InitMessage *pp = omnetpp::fromAnyPtr<InitMessage>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'InitMessage'", field);
    }
}

const char *InitMessageDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    InitMessage *pp = omnetpp::fromAnyPtr<InitMessage>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string InitMessageDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    InitMessage *pp = omnetpp::fromAnyPtr<InitMessage>(object); (void)pp;
    switch (field) {
        case FIELD_WS: return long2string(pp->getWS());
        case FIELD_TO: return double2string(pp->getTO());
        case FIELD_PT: return double2string(pp->getPT());
        case FIELD_TD: return double2string(pp->getTD());
        case FIELD_ED: return double2string(pp->getED());
        case FIELD_DD: return double2string(pp->getDD());
        case FIELD_LP: return double2string(pp->getLP());
        case FIELD_startTime: return long2string(pp->getStartTime());
        case FIELD_startingNode: return bool2string(pp->getStartingNode());
        default: return "";
    }
}

void InitMessageDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    InitMessage *pp = omnetpp::fromAnyPtr<InitMessage>(object); (void)pp;
    switch (field) {
        case FIELD_WS: pp->setWS(string2long(value)); break;
        case FIELD_TO: pp->setTO(string2double(value)); break;
        case FIELD_PT: pp->setPT(string2double(value)); break;
        case FIELD_TD: pp->setTD(string2double(value)); break;
        case FIELD_ED: pp->setED(string2double(value)); break;
        case FIELD_DD: pp->setDD(string2double(value)); break;
        case FIELD_LP: pp->setLP(string2double(value)); break;
        case FIELD_startTime: pp->setStartTime(string2long(value)); break;
        case FIELD_startingNode: pp->setStartingNode(string2bool(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'InitMessage'", field);
    }
}

omnetpp::cValue InitMessageDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    InitMessage *pp = omnetpp::fromAnyPtr<InitMessage>(object); (void)pp;
    switch (field) {
        case FIELD_WS: return pp->getWS();
        case FIELD_TO: return pp->getTO();
        case FIELD_PT: return pp->getPT();
        case FIELD_TD: return pp->getTD();
        case FIELD_ED: return pp->getED();
        case FIELD_DD: return pp->getDD();
        case FIELD_LP: return pp->getLP();
        case FIELD_startTime: return pp->getStartTime();
        case FIELD_startingNode: return pp->getStartingNode();
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'InitMessage' as cValue -- field index out of range?", field);
    }
}

void InitMessageDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    InitMessage *pp = omnetpp::fromAnyPtr<InitMessage>(object); (void)pp;
    switch (field) {
        case FIELD_WS: pp->setWS(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_TO: pp->setTO(value.doubleValue()); break;
        case FIELD_PT: pp->setPT(value.doubleValue()); break;
        case FIELD_TD: pp->setTD(value.doubleValue()); break;
        case FIELD_ED: pp->setED(value.doubleValue()); break;
        case FIELD_DD: pp->setDD(value.doubleValue()); break;
        case FIELD_LP: pp->setLP(value.doubleValue()); break;
        case FIELD_startTime: pp->setStartTime(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_startingNode: pp->setStartingNode(value.boolValue()); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'InitMessage'", field);
    }
}

const char *InitMessageDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

omnetpp::any_ptr InitMessageDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    InitMessage *pp = omnetpp::fromAnyPtr<InitMessage>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void InitMessageDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    InitMessage *pp = omnetpp::fromAnyPtr<InitMessage>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'InitMessage'", field);
    }
}

namespace omnetpp {

}  // namespace omnetpp

