#pragma once

#include "tight_loop.h"
#include "independent_variables.h"

#include <openthread/thread_ftd.h>

/**
 * https://datatracker.ietf.org/doc/html/rfc7641#section-2
 */
#define OBSERVE_SUBSCRIBE 0
#define OBSERVE_CANCEL 1

#define THROUGHPUT_OBSERVE_URI "throughput-observe"
#define THROUGHPUT_OBSERVE_NAME "Throughput Observe"

#define PACKET_LOSS_OBSERVE_URI "packet-loss-observe"
#define PACKET_LOSS_OBSERVER_NAME "Packet Loss Observe"

#define NOTIFICATION_INTERVAL_SECONDS 60

typedef struct NotificationCallbackArgs
{
  uint8_t *requestBytes;
  otMessageInfo *requestInfo;
}
NotificationCallbackArgs;

void startSendNotifications(NotificationCallbackArgs *args);
void stopSendNotifications();

void tpObserveStartCallback(otChangedFlags changed_flags, void* ctx);