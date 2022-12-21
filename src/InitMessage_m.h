//
// Generated file, do not edit! Created by opp_msgtool 6.0 from InitMessage.msg.
//

#ifndef __INITMESSAGE_M_H
#define __INITMESSAGE_M_H

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <omnetpp.h>

// opp_msgtool version check
#define MSGC_VERSION 0x0600
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of opp_msgtool: 'make clean' should help.
#endif

class InitMessage;
/**
 * Class generated from <tt>InitMessage.msg:1</tt> by opp_msgtool.
 * <pre>
 * packet InitMessage
 * {
 *     int WS;
 *     double TO;
 *     double PT;
 *     double TD;
 *     double ED;
 *     double DD;
 *     double LP;
 *     int startTime;
 *     bool startingNode;
 * }
 * </pre>
 */
class InitMessage : public ::omnetpp::cPacket
{
  protected:
    int WS = 0;
    double TO = 0;
    double PT = 0;
    double TD = 0;
    double ED = 0;
    double DD = 0;
    double LP = 0;
    int startTime = 0;
    bool startingNode = false;

  private:
    void copy(const InitMessage& other);

  protected:
    bool operator==(const InitMessage&) = delete;

  public:
    InitMessage(const char *name=nullptr, short kind=0);
    InitMessage(const InitMessage& other);
    virtual ~InitMessage();
    InitMessage& operator=(const InitMessage& other);
    virtual InitMessage *dup() const override {return new InitMessage(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    virtual int getWS() const;
    virtual void setWS(int WS);

    virtual double getTO() const;
    virtual void setTO(double TO);

    virtual double getPT() const;
    virtual void setPT(double PT);

    virtual double getTD() const;
    virtual void setTD(double TD);

    virtual double getED() const;
    virtual void setED(double ED);

    virtual double getDD() const;
    virtual void setDD(double DD);

    virtual double getLP() const;
    virtual void setLP(double LP);

    virtual int getStartTime() const;
    virtual void setStartTime(int startTime);

    virtual bool getStartingNode() const;
    virtual void setStartingNode(bool startingNode);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const InitMessage& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, InitMessage& obj) {obj.parsimUnpack(b);}


namespace omnetpp {

template<> inline InitMessage *fromAnyPtr(any_ptr ptr) { return check_and_cast<InitMessage*>(ptr.get<cObject>()); }

}  // namespace omnetpp

#endif // ifndef __INITMESSAGE_M_H
