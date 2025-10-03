#ifndef CROSS
#ifndef __IRSOCKETCONNECTOR_H_
#define __IRSOCKETCONNECTOR_H_

#include "Common.h"

// ACE dependencies temporarily disabled
// #include <ace/Connector.h>
// #include <ace/SOCK_Connector.h>

// #include "IRSocket.h"

class IRSocketConnector
{
public:
    IRSocketConnector(void) { }
    virtual ~IRSocketConnector(void) { }

protected:
    // ACE functionality temporarily disabled
    // virtual int handle_timeout(const ACE_Time_Value& /*current_time*/, const void* /*act = 0*/) { return 0; }
    // virtual int handle_accept_error(void) { return 0; }
};

#endif /* __IRSOCKETCONNECTOR_H_ */
/// @}
#endif
