#pragma once
/////////////////////////////////////////////////////////////////////////
// Translater.h - Translates messages to/from managed and native types //
// ver 1.0                                                             //
// Vikrant Bhopatrao, CSE687 - Object Oriented Design, Spring 2018     //
/////////////////////////////////////////////////////////////////////////
/*
*  Package Operations:
* ---------------------
*  This C++\Cli Package contains one class, Translater.  It's purpose is to convert
*  managed messages, CsMessage, to native messages, Message, and back.
*
* Public Interface :
* ------------------
* - Translater();
* - bool listen(CsEndPoint^ ep);
* - void postMessage(CsMessage^ msg);
* - CsMessage^ getMessage();
* - CsMessage^ fromMessage(Message& msg);
* - Message fromCsMessage(CsMessage^ csMsg);
* - void stop();
*
*  Required Files:
* -----------------
*  Translater.h, Translater.cpp
*  CsMessage.h
*
*  Required References:
* ----------------------
*  CommLibWrapper.dll
*  Message.lib
*  
*
*  Maintenance History:
* ----------------------
*  ver 1.0 : 5 April 2018
*/

// Project > Properties > C/C++ > Language > Conformance mode set to No
// Add references to System, System.Core

#include <string>
#include "../CppCommWithFileXfer/MsgPassingComm/IComm.h"
#include "CsMessage.h"
#include "../CommLibWrapper/CommLibWrapper.h"

using namespace System;
using namespace System::Text;

namespace MsgPassingCommunication
{
  public ref class Translater
  {
  public:
    Translater();
    bool listen(CsEndPoint^ ep);
    void postMessage(CsMessage^ msg);
    CsMessage^ getMessage();
    CsMessage^ fromMessage(Message& msg);
    Message fromCsMessage(CsMessage^ csMsg);
	void stop();
  private:
    CsEndPoint^ ep_;
    MsgPassingCommunication::CommFactory* pFactory;
    MsgPassingCommunication::IComm* pComm;
  };
  //----< initialize endpoint >----------------------------------------

  Translater::Translater()
  {
    ep_ = gcnew CsEndPoint;
  }
  //----< set client listen endpoint and start Comm >------------------

  bool Translater::listen(CsEndPoint^ ep)
  {
    std::cout << "\n  using CommFactory to create instance of Comm on native heap";
    ep_->machineAddress = ep->machineAddress;
    ep_->port = ep->port;
    pFactory = new CommFactory;
    pComm = pFactory->create(Sutils::MtoNstr(ep_->machineAddress), ep_->port);
    bool started=pComm->start();
    delete pFactory;
	return started;
  }

  void Translater::stop() {
	  std::cout << "\n  Closing Comm of Client...";
	  pComm->stop();
	  delete pComm;
  }
  //----< convert native message to managed message >------------------

  CsMessage^ Translater::fromMessage(Message& msg)
  {
    CsMessage^ csMsg = gcnew CsMessage;
    Message::Attributes attribs = msg.attributes();
    for (auto attrib : attribs)
    {
      csMsg->add(Sutils::NtoMstr(attrib.first), Sutils::NtoMstr(attrib.second));
    }
    return csMsg;
  }
  //----< convert managed message to native message >------------------

  Message Translater::fromCsMessage(CsMessage^ csMsg)
  {
    Message msg;
    auto enumer = csMsg->attributes->GetEnumerator();
    while (enumer.MoveNext())
    {
      String^ key = enumer.Current.Key;
      String^ value = enumer.Current.Value;
      msg.attribute(Sutils::MtoNstr(key), Sutils::MtoNstr(value));
    }
    return msg;
  }
  //----< use Comm to post message >-----------------------------------

  inline void Translater::postMessage(CsMessage^ csMsg)
  {
    std::cout << "\n  Sending message to Repository Server\n";
    Message msg = this->fromCsMessage(csMsg);
	msg.show();
    pComm->postMessage(msg);
  }
  //----< get message from Comm >--------------------------------------

  inline CsMessage^ Translater::getMessage()
  {
    Message msg = pComm->getMessage();
	std::cout << "\n  Getting message from Repository Server\n";
	msg.show();
    return fromMessage(msg);
  }
}