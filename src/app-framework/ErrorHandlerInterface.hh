#ifndef ERROR_HANDLER_INTERFACE_HH
#define ERROR_HANDLER_INTERFACE_HH

#include <exception>
#include <PlanError.hh>
#include <InterfaceError.hh>

namespace PLEXIL
{

class ErrorHandlerInterface
{
public:
  virtual ~ErrorHandlerInterface(){}

  virtual void handlePlanError(PlanError const & planError) = 0;

  virtual void handleInterfaceError(InterfaceError const & interfaceError) = 0;

  virtual void handleError(Error const & error) = 0;

  virtual void handleStdException(std::exception const & stdException) = 0;

  virtual void handleUnknownError() = 0;
};


}

#endif // ERROR_HANDLER_INTERFACE_HH

