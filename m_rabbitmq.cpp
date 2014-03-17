/* $LinkerFlags:  -lSimpleAmqpClient */

#include <iostream>
#include <stdlib.h>
#include <string>
#include "m_message.h"
#include "inspircd.h"
#include "filelogger.h"
#include <SimpleAmqpClient/SimpleAmqpClient.h>

class RabbitMQException : public ModuleException
{
public:
	RabbitMQException(const std::string& method, const std::string& error)
		: ModuleException("Error in RabbitMQ " + method + " :: " + error)
	{
	}
};

class RabbitMQMessage : public Message
{
    AmqpClient::Channel::ptr_t mChannel;
    std::string exchangeName;
    std::string routingKey;
    AmqpClient::BasicMessage::ptr_t basicMassage;
    std::string rabbitMessage;
public:
    RabbitMQMessage(const AmqpClient::Channel::ptr_t &channel, std::string& exchange)
    :mChannel(channel),exchangeName(exchange)
    {
        basicMassage = AmqpClient::BasicMessage::Create();        
        routingKey = "inspircdKey";
        rabbitMessage = "";
    }

    RabbitMQMessage(const AmqpClient::Channel::ptr_t &channel, const std::string &exchange, const std::string &routing)
    :mChannel(channel),exchangeName(exchange),routingKey(routing)
    {
        basicMassage = AmqpClient::BasicMessage::Create();    
        rabbitMessage = "";
    }

    virtual const std::string& GetTextString()
    {
        return rabbitMessage;
    }
    virtual bool sendMessage(const std::string &message )
    {
        
        ServerInstance->Logs->Log("RabbitMQMessage", DEBUG, "RabbitMQMessage::sendMessage() INPUT");          
        rabbitMessage = message;
        
        try{
            
            basicMassage->Body(message);
            mChannel->BasicPublish(exchangeName, routingKey, basicMassage);    
            ServerInstance->Logs->Log("RabbitMQMessage", DEBUG, "RabbitMQMessage::sendMessage() "+ message);    
            
        }catch(std::runtime_error &ex){
            std::string msg (ex.what());
            ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "RabbitMQMessage::sendMessage()::ERROR:  " + msg);
            throw RabbitMQException("RabbitMQException::sendMessage()",msg);                 
        }
        
        ServerInstance->Logs->Log("RabbitMQMessage", DEBUG, "RabbitMQMessage::sendMessage() OUTPUT");          
        return true;
    }
    /**
     * Publishes a Basic message
     * @param message the BasicMessage object to publish to the queue
     * @param mandatory requires the message to be delivered to a queue. A MessageReturnedException is thrown
     *  if the message cannot be routed to a queue
     * @param immediate requires the message to be both routed to a queue, and immediately delivered via a consumer
     *  if the message is not routed, or a consumer cannot immediately deliver the message a MessageReturnedException is
     *  thrown
     */    
    virtual bool sendMessage(const std::string &message, bool mandatory, bool immediate)
    {        
        ServerInstance->Logs->Log("RabbitMQMessage", DEBUG, "RabbitMQMessage::sendMessage(...) INPUT");                      
        rabbitMessage = message;
        
        try{
            
            basicMassage->Body(message);
            mChannel->BasicPublish(exchangeName, routingKey, basicMassage,mandatory,immediate);            
            ServerInstance->Logs->Log("RabbitMQMessage", DEBUG, "RabbitMQMessage::sendMessage(...) "+ message);          
            
        }catch(std::runtime_error &ex){
            std::string msg (ex.what());
            ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "RabbitMQMessage::sendMessage(...)::ERROR:  " + msg);
            throw RabbitMQException("RabbitMQException::sendMessage(...)",msg);
        }   
        ServerInstance->Logs->Log("RabbitMQMessage", DEBUG, "RabbitMQMessage::sendMessage(...) OUTPUT");                      
        return true;
    }
};
class RabbitMQFactory : public MessageFactory
{  
    AmqpClient::Channel::ptr_t channel;
    std::string exchange;
    std::string routing;
    std::string rabbitMQHost;
    int rabbitMQPort;
    std::string rabbitMQUsername;
    std::string rabbitMQpassword;
    
public:
   
    RabbitMQFactory(Module* m) : MessageFactory(m, "message/rabbitmq") 
    {
        loadConfig();
    }
    
    virtual Message* Create()
    {        
        return new RabbitMQMessage(channel,exchange,routing);
    }
private:    
    void loadConfig()
    {
        ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "RabbitMQFactory::LoadConfig() INPUT");    
        
        std::stringstream ss;
        
        //data config for create a channel                 
        std::string host                = ServerInstance->Config->ConfValue("ModuleRabbitMQ")->getString("host");
        int port                        = ServerInstance->Config->ConfValue("ModuleRabbitMQ")->getInt("port");       
        std::string username            = ServerInstance->Config->ConfValue("ModuleRabbitMQ")->getString("username");
        std::string password            = ServerInstance->Config->ConfValue("ModuleRabbitMQ")->getString("password");
        std::string vhost               = ServerInstance->Config->ConfValue("ModuleRabbitMQ")->getString("vhost");
        int frame_max                   = ServerInstance->Config->ConfValue("ModuleRabbitMQ")->getInt("frameMax");
        bool useSSL                     = ServerInstance->Config->ConfValue("ModuleRabbitMQ")->getBool("useSSL");
        std::string pathToCaCert        = ServerInstance->Config->ConfValue("ModuleRabbitMQ")->getString("pathToCaCert");
        std::string pathToClientKey     = ServerInstance->Config->ConfValue("ModuleRabbitMQ")->getString("pathToClientKey");
        std::string pathToClientCert    = ServerInstance->Config->ConfValue("ModuleRabbitMQ")->getString("pathToClientCert");
        
        //data config for create a extange
        std::string exchangeName        = ServerInstance->Config->ConfValue("ModuleRabbitMQExtange")->getString("name");
        std::string exchangeType        = ServerInstance->Config->ConfValue("ModuleRabbitMQExtange")->getString("type"); 
        bool isExchangePassive          = ServerInstance->Config->ConfValue("ModuleRabbitMQExtange")->getBool("isPassive"); 
        bool isExchangeDurable          = ServerInstance->Config->ConfValue("ModuleRabbitMQExtange")->getBool("isDurable"); 
        bool isExchangeAutoDelete       = ServerInstance->Config->ConfValue("ModuleRabbitMQExtange")->getBool("isAutoDelete"); 

        //data config for a create a Queue
        std::string queueName           = ServerInstance->Config->ConfValue("ModuleRabbitMQQueue")->getString("name");
        bool isQueuePassive             = ServerInstance->Config->ConfValue("ModuleRabbitMQQueue")->getBool("isPassive"); 
        bool isQueueDurable             = ServerInstance->Config->ConfValue("ModuleRabbitMQQueue")->getBool("isDurable"); 
        bool isQueueExclusive           = ServerInstance->Config->ConfValue("ModuleRabbitMQQueue")->getBool("isExclusive"); 
        bool isQueueAutoDelete          = ServerInstance->Config->ConfValue("ModuleRabbitMQQueue")->getBool("isAutoDelete"); 
        std::string routingKey          = ServerInstance->Config->ConfValue("ModuleRabbitMQQueue")->getString("routingKey"); 
        this->exchange = exchangeName;
        this->routing  = routingKey; 
        this->rabbitMQHost = host;
        this->rabbitMQPort = port;
        this->rabbitMQUsername = username;
        this->rabbitMQpassword = password;
        //only debug 
        ss << port;
        std::string portString = ss.str();
        ss << frame_max;
        std::string frameMaxString = ss.str();
        std::string useSSLString = (useSSL ? "true" :"false");
        std::string isExchangePassiveString = (isExchangePassive ? "true" :"false");
        std::string isExchangeDurableString = (isExchangeDurable ? "true" :"false");
        std::string isExchangeAutoDeleteString = (isExchangeAutoDelete ? "true" :"false");
        
        std::string isQueuePassiveString = (isQueuePassive ? "true" :"false");
        std::string isQueueDurableString = (isQueueDurable ? "true" :"false");
        std::string isQueueExclusiveString = (isQueueExclusive ? "true" :"false");
        std::string isQueueAutoDeleteString = (isQueueAutoDelete ? "true" :"false");
        
        
        ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "RabbitMQFactory::LoadConfig() PARAM-Client.host: " + host);
        ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "RabbitMQFactory::LoadConfig() PARAM-Client.port: " + portString);
        ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "RabbitMQFactory::LoadConfig() PARAM-Client.username: " + username);
        ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "RabbitMQFactory::LoadConfig() PARAM-Client.password: " + password);
        ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "RabbitMQFactory::LoadConfig() PARAM-Client.vhost: " + vhost);
        ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "RabbitMQFactory::LoadConfig() PARAM-Client.frameMaxString: " + frameMaxString);
        ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "RabbitMQFactory::LoadConfig() PARAM-Client.useSSL: " + useSSLString);
        ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "RabbitMQFactory::LoadConfig() PARAM-Client.pathToCaCert: " + pathToCaCert);
        ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "RabbitMQFactory::LoadConfig() PARAM-Client.pathToClientKey: " + pathToClientKey);
        ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "RabbitMQFactory::LoadConfig() PARAM-Client.pathToClientCert: " + pathToClientCert);
        ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "RabbitMQFactory::LoadConfig() PARAM-Extange.name: " + exchangeName);
        ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "RabbitMQFactory::LoadConfig() PARAM-Extange.type: " + exchangeType);
        ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "RabbitMQFactory::LoadConfig() PARAM-Extange.isPassive: " + isExchangePassiveString);
        ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "RabbitMQFactory::LoadConfig() PARAM-Extange.isDurable: " + isExchangeDurableString);
        ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "RabbitMQFactory::LoadConfig() PARAM-Extange.isAutoDelete: " + isExchangeAutoDeleteString);
        ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "RabbitMQFactory::LoadConfig() PARAM-Queue.name: " + queueName);
        ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "RabbitMQFactory::LoadConfig() PARAM-Queue.isPassive: " + isQueuePassiveString);
        ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "RabbitMQFactory::LoadConfig() PARAM-Queue.isDurable: " + isQueueDurableString);
        ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "RabbitMQFactory::LoadConfig() PARAM-Queue.isExclusive: " + isQueueExclusiveString);
        ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "RabbitMQFactory::LoadConfig() PARAM-Queue.isAutoDelete: " + isQueueAutoDeleteString);
        ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "RabbitMQFactory::LoadConfig() PARAM-Queue.routingKey: " + routingKey);
        
        try{
            if(useSSL){
                channel = AmqpClient::Channel::CreateSecure(
                        pathToCaCert, 
                        host, 
                        pathToClientKey, 
                        pathToClientCert, 
                        port, 
                        username, 
                        password, 
                        vhost, 
                        frame_max
                        );
                ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "RabbitMQFactory::LoadConfig() Creatate Secure Client");    
            }else{
                channel = AmqpClient::Channel::Create(
                        host, 
                        port, 
                        username, 
                        password, 
                        vhost, 
                        frame_max                    
                    );
                ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "RabbitMQFactory::LoadConfig() Creatate Client");    
            }
        }catch(std::runtime_error &ex){
                std::string msg (ex.what());
                ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "RabbitMQFactory::LoadConfig()::ERROR:  " + msg);
                throw RabbitMQException("ModuleRabbitMQ::LoadConfig()",msg);           
        }
        
        
        try{
            channel->DeclareExchange(exchangeName,exchangeType,isExchangePassive,isExchangeDurable,isExchangeAutoDelete);
        }catch(std::runtime_error &ex){
                std::string msg (ex.what());
                ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "RabbitMQFactory::LoadConfig::ERROR:  " + msg);
                throw RabbitMQException("ModuleRabbitMQ::LoadConfig()",msg);   
        }
               
        
        ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "RabbitMQFactory::LoadConfig() Declare Queue");            
        try{
            channel->DeclareQueue(queueName,isQueuePassive, isQueueDurable,isQueueExclusive,isQueueAutoDelete);            
            channel->BindQueue(queueName, exchangeName, routingKey);
        }catch(std::runtime_error &ex){
            std::string msg (ex.what());
            ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "RabbitMQFactory::LoadConfig::ERROR:  " + msg);
            throw RabbitMQException("ModuleRabbitMQ::LoadConfig()",msg);            
        }
        ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "RabbitMQFactory::LoadConfig() OUTPUT");            
    }    
    
};
class ModuleRabbitMQ : public Module
{
    RabbitMQFactory rabbitMQFactory;
    Message *message;
   
public:
    ModuleRabbitMQ()
    :rabbitMQFactory(this)
    {
        ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "ModuleRabbitMQ::ModuleRabbitMQ() INPUT");    
        ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "ModuleRabbitMQ::ModuleRabbitMQ() OUTPUT");    
    }
    void init() 
    {
        ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "ModuleRabbitMQ::init() INPUT");    
        message = rabbitMQFactory.Create();
        Implementation eventlist[] = { I_OnUserConnect, I_OnUserDisconnect, I_OnUserPostNick };
	ServerInstance->Modules->Attach(eventlist, this, sizeof(eventlist)/sizeof(Implementation));        
        ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "ModuleRabbitMQ::init() OUTPUT");    
    }
    virtual void OnUserConnect(LocalUser* user){
        ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "ModuleRabbitMQ::OnUserConnect(): " + user->nick);   
            try{
                message->sendMessage("ModuleRabbitMQ::OnUserConnect(): " + user->nick);
            }catch(RabbitMQException &ex){
                ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "ModuleRabbitMQ::OnUserConnect(): Server RabbitMQ is down ");
            }
        }

        void OnUserDisconnect(LocalUser* user) 
        {
            //log debug server               
            ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "ModuleRabbitMQ::OnUserDisconnect(): " + user->nick);   
            try{
                message->sendMessage("ModuleRabbitMQ::OnUserDisconnect(): " + user->nick);
            }catch(RabbitMQException &ex){
                ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "ModuleRabbitMQ::OnUserDisconnect(): Server RabbitMQ is down ");
            }                        
        }
               
        void OnUserPostNick(User* user, const std::string &oldnick)
        {
            ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "ModuleRabbitMQ::OnUserPostNick(): change-nick[new|old] [" + user->nick + "|" + oldnick + "]");   
            try{
                message->sendMessage("ModuleRabbitMQ::OnUserPostNick(): change-nick[new|old] [" + user->nick + "|" + oldnick + "]");
            }catch(RabbitMQException &ex){                
                ServerInstance->Logs->Log("ModuleRabbitMQ", DEBUG, "ModuleRabbitMQ::OnUserPostNick(): Server RabbitMQ is down ");
            }                        
            
        }    
    Version GetVersion()
    {
        return Version("Message Provider Module for RabbitMQ Service", VF_VENDOR);
    }  
};
MODULE_INIT(ModuleRabbitMQ)
