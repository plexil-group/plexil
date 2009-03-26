#ifndef LCM_BASE_IMPL_SASEXEC_HH
#define LCM_BASE_IMPL_SASEXEC_HH

#include <driveResponse.h>
#include <telemetryDouble.h>
#include "SASAdaptor.hh"


static void
driveResponse_handler (const lcm_recv_buf_t *rbuf, const char * channel, 
                       const driveResponse * msg, void * user)
{
  SASAdaptor* server = static_cast<SASAdaptor*>(user);
  server->postCommandResponse("drive", msg->retValue);
}

static void
telemetryDouble_handler (const lcm_recv_buf_t *rbuf, const char * channel, 
                       const telemetryDouble * msg, void * user)
{
  SASAdaptor* server = static_cast<SASAdaptor*>(user);
  printf ("GOT some response for telemetry: %s %d %f\n", msg->state, msg->number, msg->values[0]);
  
  server->postTelemetryState(msg->state, msg->number, msg->values);
}


class LcmBaseImplSASExec
{
public:
  LcmBaseImplSASExec(lcm_t *lcm, SASAdaptor* sasAdaptor)
    : m_lcm(lcm), m_sasAdaptor(sasAdaptor)
  {
    subscribeToMessages();
  }

  ~LcmBaseImplSASExec()
  {
    unsubscribeFromMessages();
  }

  void subscribeToMessages()
  {
    driveRespSub = driveResponse_subscribe(m_lcm, "DRIVERESPONSE", 
                                           &driveResponse_handler, m_sasAdaptor);
    telDouble = telemetryDouble_subscribe(m_lcm, "TELEMETRYDOUBLE",
                                          &telemetryDouble_handler, m_sasAdaptor);
  }

  void unsubscribeFromMessages()
  {
    driveResponse_unsubscribe(m_lcm, driveRespSub);
    telemetryDouble_unsubscribe(m_lcm, telDouble);
  }

private:
  lcm_t *m_lcm;
  SASAdaptor *m_sasAdaptor;
  driveResponse_subscription_t *driveRespSub;
  telemetryDouble_subscription_t *telDouble;
};

#endif
