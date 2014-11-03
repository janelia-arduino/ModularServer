// ----------------------------------------------------------------------------
// Callbacks.h
//
//
// Authors:
// Peter Polidoro polidorop@janelia.hhmi.org
// ----------------------------------------------------------------------------
#ifndef CALLBACKS_H
#define CALLBACKS_H
#include "RemoteDevice.h"
#include "Constants.h"
#include "NonBlockBlink.h"


namespace callbacks
{
void setLedOnCallback();

void setLedOffCallback();

void getLedPinCallback();

void blinkLedCallback();
}
#endif
