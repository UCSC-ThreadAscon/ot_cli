#include "tight_loop.h"
#include "time_api.h"
#include "main.h"

#define EXPECTED_TOTAL_BYTES 4000

static otSockAddr socket;

static uint32_t packetsAcked;
static uint32_t totalBytes;
static struct timeval startTime;
static struct timeval endTime;

void tpConfirmableSend(otSockAddr *socket)
{
  uint32_t payload = 0;
  createRandomPayload((uint8_t *) &payload);
  request(socket, (void *) &payload, TIGHT_LOOP_PAYLOAD_BYTES, THROUGHPUT_CONFIRMABLE_URI,
          tpConfirmableResponseCallback, OT_COAP_TYPE_CONFIRMABLE);
  return;
}

void tpConfirmableResponseCallback(void *aContext,
                                   otMessage *aMessage,
                                   const otMessageInfo *aMessageInfo,
                                   otError aResult)
{
  if (aResult != OT_ERROR_NONE)
  {
    otLogWarnPlat("Failed to transmit CoAP request. Reason: %s",
                  otThreadErrorToString(aResult));
  }
  else
  {
    if (packetsAcked < MAX_PACKETS)
    {
      packetsAcked += 1;
      totalBytes += TIGHT_LOOP_PAYLOAD_BYTES;

      if (packetsAcked == MAX_PACKETS)
      {
        /* Check that 4000 total bytes (i.e. 4 bytes * 1000 packets)
           have been received.
        */
        assert(totalBytes == EXPECTED_TOTAL_BYTES);

        /** The throughput formula is:
         *
         *           MAX_PACKETS * PAYLOAD_SIZE_BYTES
         *      -----------------------------------------   bytes/time
         *                    t_end - t_start
         * 
         */
        endTime = getTimevalNow();

        double denominatorUs = timeDiffUs(startTime, endTime);
        double denominatorMs = US_TO_MS(denominatorUs);
        double denominatorSecs = US_TO_SECONDS(denominatorUs);

        double throughputSecs = totalBytes / denominatorSecs;
        double throughputMs = totalBytes / denominatorMs;
        double throughputUs = totalBytes / denominatorUs;

        PrintDelimiter();
        otLogNotePlat("The throughput is:");
        otLogNotePlat("%.7f bytes/second, or", throughputSecs);
        otLogNotePlat("%.7f bytes/ms, or", throughputMs);
        otLogNotePlat("%.7f bytes/us.", throughputUs);
        otLogNotePlat("Duration: %.7f seconds", denominatorSecs);
        otLogNotePlat("Total Received: %" PRIu32 " bytes", totalBytes);
        otLogNotePlat("Number of packets sent and ACKed: %" PRIu32 "", packetsAcked);
        PrintDelimiter();

        startNextTrial();
        return;
      }
    }
  }

  tpConfirmableSend(&socket);
  return;
}

void tpConfirmableMain()
{
  coapStart();
  InitSocket(&socket, SERVER_IP);

  PrintDelimiter();
  otLogNotePlat("Starting the throughput experimental trial!");
  PrintDelimiter();

  startTime = getTimevalNow();
  tpConfirmableSend(&socket);
  return;
}

/**
 * The code for the Throughput Server start callback function comes from the ESP-IDF
 * OpenThread SED state change callback example function:
 * https://github.com/UCSC-ThreadAscon/esp-idf/blob/master/examples/openthread/ot_sleepy_device/deep_sleep/main/esp_ot_sleepy_device.c#L73
 */
void tpConfirmableStartCallback(otChangedFlags changed_flags, void* ctx)
{
  OT_UNUSED_VARIABLE(ctx);
  static otDeviceRole s_previous_role = OT_DEVICE_ROLE_DISABLED;

  otInstance* instance = esp_openthread_get_instance();
  if (!instance)
  {
    return;
  }

  otDeviceRole role = otThreadGetDeviceRole(instance);
  if ((connected(role) == true) && (connected(s_previous_role) == false))
  {
    tpConfirmableMain();
  }
  s_previous_role = role;
  return;
}