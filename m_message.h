/* 
 * File:   m_message.h
 * Author: ant3
 *
 * Created on 7 de marzo de 2014, 10:20
 */

#ifndef M_MESSAGE_H
#define	M_MESSAGE_H

#include "inspircd.h"

class Message : public classbase
{
protected:
        Message(){}

public:
    virtual ~Message(){}

    virtual const std::string& GetTextString() = 0;
    
    virtual bool sendMessage(const std::string &message ) = 0;
};
class MessageFactory : public DataProvider
{
 public:
	MessageFactory(Module* Creator, const std::string& Name) : DataProvider(Creator, Name) {}

	virtual Message* Create() = 0;        
};
#endif	/* M_MESSAGE_H */

