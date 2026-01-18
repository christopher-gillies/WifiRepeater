#ifndef IP_FORWARD_H
#define IP_FORWARD_H

#include "esp_err.h"

/**
 * Enable IP forwarding between STA and AP interfaces
 * This allows packets to be routed between the two networks
 */
esp_err_t ip_forward_init(void);

#endif // IP_FORWARD_H
