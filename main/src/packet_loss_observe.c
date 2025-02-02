#include "observe.h"
#include "time_api.h"
#include "main.h"

static otCoapResource route;

static Subscription brSubscription;
static bool brSubscribed;

void plObserveRequestHandler(void *aContext,
                             otMessage *aMessage,
                             const otMessageInfo *aMessageInfo)
{
  uint64_t observeValue = 0;
  otError error = coapGetOptionValue(aMessage, OT_COAP_OPTION_OBSERVE, &observeValue);
  handleError(error, "CoAP Get Observe Option Value");

  if (!brSubscribed)
  {
    if (observeValue == OBSERVE_SUBSCRIBE)
    {
      brSubscribed = true;

      brSubscription.token = getToken(aMessage);
      brSubscription.tokenLength = otCoapMessageGetTokenLength(aMessage);
      brSubscription.sequenceNum = 0;

      brSubscription.sockAddr.mAddress = aMessageInfo->mPeerAddr;
      brSubscription.sockAddr.mPort = aMessageInfo->mPeerPort;

      otLogNotePlat("Subscription started for token 0x%llx.", brSubscription.token);
      sendInitialTemperature(aMessage, aMessageInfo, &brSubscription);
      startSendNotifications(&brSubscription);
    }
    else
    {
      otLogWarnPlat("Received cancellation from token %llx when NOT subscribed.",
                    getToken(aMessage));
      sendCoapResponse(aMessage, aMessageInfo);
    }
  }
  else // brSubscribed
  {
    if (observeValue == OBSERVE_CANCEL)
    {
      brSubscribed = false;

      stopSendNotifications(&brSubscription);
      sendCoapResponse(aMessage, aMessageInfo);

      otLogNotePlat("Cancelled subscription for token 0x%llx.", brSubscription.token);
      EmptyMemory(&brSubscription, sizeof(Subscription));
    }
    else
    {
      otLogWarnPlat("Received subscription request from token 0x%llx when already subscribed.",
                    getToken(aMessage));
      sendCoapResponse(aMessage, aMessageInfo);
    }
  }
  return;
}

void plObserveStartServer(void)
{
  startCoapServer(OT_DEFAULT_COAP_PORT);
  createResource(&route, PACKET_LOSS_OBSERVE_ROUTE, OBSERVE_EXPERIMENTS_URI,
                 plObserveRequestHandler);
  return;
}

/**
 * The code for the Experimental Setup server  start callback function comes from
 * the ESP-IDF OpenThread SED state change callback example function:
 * https://github.com/UCSC-ThreadAscon/esp-idf/blob/master/examples/openthread/ot_sleepy_device/deep_sleep/main/esp_ot_sleepy_device.c#L73
 */
void plObserveStartCallback(otChangedFlags changed_flags, void* ctx)
{
  OT_UNUSED_VARIABLE(ctx);
  static otDeviceRole s_previous_role = OT_DEVICE_ROLE_DISABLED;

  if (!OT_INSTANCE) { return; }
  otDeviceRole role = otThreadGetDeviceRole(OT_INSTANCE);

  if ((connected(role) == true) && (connected(s_previous_role) == false))
  {
    PrintDelimiter();
    plObserveStartServer();
    PrintDelimiter();
  }
  s_previous_role = role;
  return;
}