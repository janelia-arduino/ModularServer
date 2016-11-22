// ----------------------------------------------------------------------------
// InterruptTester.cpp
//
// Authors:
// Peter Polidoro polidorop@janelia.hhmi.org
// ----------------------------------------------------------------------------
#include "InterruptTester.h"


void InterruptTester::setup()
{
  // Server Setup
  modular_server_.setup();

  // Pin Setup
  pinMode(constants::led_pin, OUTPUT);

  // Add Server Streams
  modular_server_.addServerStream(Serial);

  // Set Device ID
  modular_server_.setDeviceName(constants::device_name);
  modular_server_.setFormFactor(constants::form_factor);

  // Add Hardware
  modular_server_.addHardware(constants::hardware_info,
                              interrupts_);

  // Interrupts
  modular_server::Interrupt & bnc_a_interrupt = modular_server_.createInterrupt(constants::bnc_a_interrupt_name,
                                                                                constants::bnc_a_pin);
  bnc_a_interrupt.enablePullup();

  modular_server::Interrupt & bnc_b_interrupt = modular_server_.createInterrupt(constants::bnc_b_interrupt_name,
                                                                                constants::bnc_b_pin);
  bnc_b_interrupt.enablePullup();

  // Add Firmware
  modular_server_.addFirmware(constants::firmware_info,
                              properties_,
                              parameters_,
                              functions_,
                              callbacks_);

  // Properties

  // Parameters

  // Functions

  // Callbacks
  modular_server::Callback & set_led_on_callback = modular_server_.createCallback(constants::set_led_on_callback_name);
  set_led_on_callback.attachFunctor(makeFunctor((Functor0 *)0,*this,&InterruptTester::setLedOnHandler));
  set_led_on_callback.attachTo(bnc_a_interrupt,modular_server::interrupt::mode_falling);

  modular_server::Callback & set_led_off_callback = modular_server_.createCallback(constants::set_led_off_callback_name);
  set_led_off_callback.attachFunctor(makeFunctor((Functor0 *)0,*this,&InterruptTester::setLedOffHandler));
  set_led_off_callback.attachTo(bnc_b_interrupt,modular_server::interrupt::mode_falling);

  // Begin Streams
  Serial.begin(constants::baudrate);

  // Start Modular Device Server
  modular_server_.startServer();
}

void InterruptTester::update()
{
  modular_server_.handleServerRequests();
}

// Handlers must be non-blocking (avoid 'delay')
//
// modular_server_.parameter(parameter_name).getValue(value) value type must be either:
// fixed-point number (int, long, etc.)
// floating-point number (float, double)
// bool
// const char *
// ArduinoJson::JsonArray *
// ArduinoJson::JsonObject *
//
// For more info read about ArduinoJson parsing https://github.com/janelia-arduino/ArduinoJson
//
// modular_server_.property(property_name).getValue(value) value type must match the property default type
// modular_server_.property(property_name).setValue(value) value type must match the property default type
// modular_server_.property(property_name).getElementValue(value) value type must match the property array element default type
// modular_server_.property(property_name).setElementValue(value) value type must match the property array element default type

void InterruptTester::setLedOnHandler()
{
  digitalWrite(constants::led_pin, HIGH);
}

void InterruptTester::setLedOffHandler()
{
  digitalWrite(constants::led_pin, LOW);
}

