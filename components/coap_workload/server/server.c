/**
 * All of the code in this file is based upon the both CoAP and CoAP secure
 * source code used as a part of the OpenThread codebase.
 *
 * https://github.com/UCSC-ThreadAscon/openthread/tree/main/src/cli
*/
#include "coap_workload.h"

/**
 * This function is a modified version of `HandleRequest()` from the OpenThread CLI
 * Secure CoAP source code:
 * https://github.com/UCSC-ThreadAscon/openthread/blob/main/src/cli/cli_coap_secure.cpp#L814
*/
void sendCoapResponse(otMessage *aRequest, const otMessageInfo *aRequestInfo)
{
  otMessage *aResponse = NULL;
  otCoapCode status = OT_COAP_CODE_EMPTY;

  bool isConfirmable = otCoapMessageGetType(aRequest) == OT_COAP_TYPE_CONFIRMABLE;
  bool isGet = otCoapMessageGetCode(aRequest) == OT_COAP_CODE_GET;

  if (isConfirmable || isGet) {
    if (isGet) {
      status = OT_COAP_CODE_CONTENT;
    }
    else {
      status = OT_COAP_CODE_VALID;
    }

    aResponse = otCoapNewMessage(OT_INSTANCE, NULL);
    if (aResponse == NULL) {
      otLogCritPlat("Failed to initialize a new message for CoAP response.");
    }

    otError error = otCoapMessageInitResponse(aResponse, aRequest,
                                              OT_COAP_TYPE_ACKNOWLEDGMENT,
                                              status);
    HandleMessageError("coap message init response", aResponse, error);

    error = otCoapSendResponse(OT_INSTANCE, aResponse, aRequestInfo);
    HandleMessageError("send response", aResponse, error);
  }

  return;
}

otError createResource(otCoapResource *resource,
                       const char *resourceName,
                       const char *uri,
                       otCoapRequestHandler requestHandler)
{
  resource->mNext = NULL;
  resource->mContext = NULL;
  resource->mHandler = requestHandler;
  resource->mUriPath = uri;

  otCoapAddResource(OT_INSTANCE, resource);
  otLogNotePlat("Created %s server at '%s'.", resourceName, resource->mUriPath);
  return OT_ERROR_NONE;
}

void resourceDestructor(otCoapResource *resource)
{
  otLogNotePlat("Closing '%s'", resource->mUriPath);
  otCoapRemoveResource(OT_INSTANCE, resource);
  free(resource);
  return;
}

void startCoapServer(uint16_t port) {
  otError error = otCoapStart(OT_INSTANCE, port);

  if (error != OT_ERROR_NONE) {
    otLogCritPlat("Failed to start COAP server.");
  } else {
    otLogNotePlat("Started CoAP server at port %d.", port);
  }
  return;
}

const otCoapOption *coapGetOption(otMessage *aMessage, uint16_t optionNum)
{
  otCoapOptionIterator iterator;
  EmptyMemory(&iterator, sizeof(otCoapOptionIterator));
 
  otError error = otCoapOptionIteratorInit(&iterator, aMessage);
  if (error != OT_ERROR_NONE) {
    otLogCritPlat("Failed to create Options Iterator for CoAP Request.");
    return NULL;
  }

  const otCoapOption *option =
    otCoapOptionIteratorGetFirstOptionMatching(&iterator, optionNum);
  return option;
}