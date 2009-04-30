#ifndef LCM_BASE_IMPL_HH
#define LCM_BASE_IMPL_HH

#include "PlexilCommRelay.hh"
#include <driveCommand.h>
#include <genericCommand.h>
#include <genericResponse.h>
#include <driveResponse.h>
#include <telemetryDouble.h>
#include "ResponseMessage.hh"


class PlexilCommRelay;

static void
driveCommand_handler (const lcm_recv_buf_t *rbuf, const char * channel, 
                      const driveCommand * msg, void * user)
{
  PlexilCommRelay* server = static_cast<PlexilCommRelay*>(user);
  printf ("GOT something\n");
  server->receivedMessage("drive");
}

static void
genericCommand_handler (const lcm_recv_buf_t *rbuf, const char * channel, 
                        const genericCommand * msg, void * user)
{
  PlexilCommRelay* server = static_cast<PlexilCommRelay*>(user);
  printf ("GOT a generic command name: %s\n", msg->name);
  server->receivedMessage(msg->name);
}

class LcmBaseImpl
{
public:
  LcmBaseImpl(lcm_t *lcm, PlexilCommRelay *plxlCommRelay)
    : m_lcm(lcm), m_plxlCommRelay(plxlCommRelay)
  {
    subscribeToMessages();
  }
  ~LcmBaseImpl()
  {
    unsubscribeFromMessages();
  }

  void subscribeToMessages()
  {
    driveCommandSub = driveCommand_subscribe(m_lcm, "DRIVECOMMAND", 
                                             &driveCommand_handler, 
                                             m_plxlCommRelay);
    genericCommandSub = genericCommand_subscribe(m_lcm, "GENERICCOMMAND", 
                                                 &genericCommand_handler, 
                                                 m_plxlCommRelay);
  }

  void unsubscribeFromMessages()
  {
    driveCommand_unsubscribe(m_lcm, driveCommandSub);
    genericCommand_unsubscribe(m_lcm, genericCommandSub);
  }

  void sendMessage(const std::string& name, const std::string& contents, int type)
  {
    if (type == MSG_COMMAND)
      {/*
        driveResponse data;
        data.retValue=atoi(contents.c_str());
        driveResponse_publish(m_lcm, "DRIVERESPONSE", &data);
       */

        // TODO handle contents of length > 1. Will need to parse the 
        // contents string

        genericResponse data;
        double retValue[1];
        retValue[0] = atof(contents.c_str());
        data.name = const_cast<char *>(name.c_str());
        data.number = 1;
        data.retValue = retValue;
        printf("Publishing generic response for: %s\n", data.name);
        genericResponse_publish(m_lcm, "GENERICRESPONSE", &data);
      }
    else if (type == MSG_TELEMETRY)
      {
        // TODO handle contents of length > 1. Will need to parse the 
        // contents string
        telemetryDouble data;
        double values[1];
        values[0] = static_cast<double>(atoi(contents.c_str()));
        data.state = const_cast<char *>(name.c_str());
        data.number = 1;
        data.values = values;
        telemetryDouble_publish(m_lcm, "TELEMETRYDOUBLE", &data);
      }
  }

private:
  lcm_t *m_lcm;
  PlexilCommRelay* m_plxlCommRelay;
  driveCommand_subscription_t *driveCommandSub;
  genericCommand_subscription_t *genericCommandSub;

};

#endif
