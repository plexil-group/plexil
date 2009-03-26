#ifndef LCM_BASE_IMPL_HH
#define LCM_BASE_IMPL_HH

#include "PlexilCommRelay.hh"
#include <driveCommand.h>
#include <driveResponse.h>
#include <telemetryDouble.h>


class PlexilCommRelay;

static void
driveCommand_handler (const lcm_recv_buf_t *rbuf, const char * channel, 
                      const driveCommand * msg, void * user)
{
  PlexilCommRelay* server = static_cast<PlexilCommRelay*>(user);
  printf ("GOT something\n");
  server->receivedMessage("drive");
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
  }

  void unsubscribeFromMessages()
  {
    driveCommand_unsubscribe(m_lcm, driveCommandSub);
  }

  void sendMessage(const std::string& name, const std::string& contents)
  {
    if (name == "drive")
      {
        driveResponse data;
        data.retValue=atoi(contents.c_str());
        driveResponse_publish(m_lcm, "DRIVERESPONSE", &data);
      }
    else if (name == "foo")
      {
        telemetryDouble data;
        double values[1];
        values[0] = static_cast<double>(atoi(contents.c_str()));
        data.state = "foo";
        data.number = 1;
        data.values = values;
        telemetryDouble_publish(m_lcm, "TELEMETRYDOUBLE", &data);
      }
  }

private:
  lcm_t *m_lcm;
  PlexilCommRelay* m_plxlCommRelay;
  driveCommand_subscription_t *driveCommandSub;

};

#endif
