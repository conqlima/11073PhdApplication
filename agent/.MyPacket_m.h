//
// Generated file, do not edit! Created by nedtool 4.6 from src/node/application/thermometer/MyPacket.msg.
//

#ifndef _MYPACKET_M_H_
#define _MYPACKET_M_H_

#include <omnetpp.h>

// nedtool version check
#define MSGC_VERSION 0x0406
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of nedtool: 'make clean' should help.
#endif



// cplusplus {{
#include "ApplicationPacket_m.h"

extern "C" {
#include "communication/plugin/plugin.h"
#include "util/bytelib.h"
}
// }}

/**
 * Struct generated from src/node/application/thermometer/MyPacket.msg:13 by nedtool.
 */
struct Tmsg
{
    Tmsg();
    opp_string send_str;
    opp_string recv_str;
};

void doPacking(cCommBuffer *b, Tmsg& a);
void doUnpacking(cCommBuffer *b, Tmsg& a);

/**
 * Class generated from <tt>src/node/application/thermometer/MyPacket.msg:18</tt> by nedtool.
 * <pre>
 * packet MyPacket extends ApplicationPacket
 * {
 *     Tmsg extraData;
 * }
 * </pre>
 */
class MyPacket : public ::ApplicationPacket
{
  protected:
    Tmsg extraData_var;

  private:
    void copy(const MyPacket& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const MyPacket&);

  public:
    MyPacket(const char *name=NULL, int kind=0);
    MyPacket(const MyPacket& other);
    virtual ~MyPacket();
    MyPacket& operator=(const MyPacket& other);
    virtual MyPacket *dup() const {return new MyPacket(*this);}
    virtual void parsimPack(cCommBuffer *b);
    virtual void parsimUnpack(cCommBuffer *b);

    // field getter/setter methods
    virtual Tmsg& getExtraData();
    virtual const Tmsg& getExtraData() const {return const_cast<MyPacket*>(this)->getExtraData();}
    virtual void setExtraData(const Tmsg& extraData);
};

inline void doPacking(cCommBuffer *b, MyPacket& obj) {obj.parsimPack(b);}
inline void doUnpacking(cCommBuffer *b, MyPacket& obj) {obj.parsimUnpack(b);}


#endif // ifndef _MYPACKET_M_H_

