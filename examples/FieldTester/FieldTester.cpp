// ----------------------------------------------------------------------------
// FieldTester.cpp
//
// Authors:
// Peter Polidoro polidorop@janelia.hhmi.org
// ----------------------------------------------------------------------------
#include "FieldTester.h"


void FieldTester::setup()
{
  // Pin Setup

  // Add Server Streams
  modular_server_.addServerStream(Serial);

  // Set Device ID
  modular_server_.setDeviceName(constants::device_name);
  modular_server_.setFormFactor(constants::form_factor);

  // Add Hardware Info
  modular_server_.addHardwareInfo(constants::hardware_info);

  // Add Firmware
  modular_server_.addFirmware(constants::firmware_info,
                              fields_,
                              parameters_,
                              methods_);

  // Fields
  modular_server::Field & double_field = modular_server_.createField(constants::double_field_name,constants::double_default);

  modular_server::Field & bool_field = modular_server_.createField(constants::bool_field_name,constants::bool_default);
  bool_field.attachPreSetValueCallback(makeFunctor((Functor0 *)0,*this,&FieldTester::preSetFieldValueCallback));
  bool_field.attachPostSetValueCallback(makeFunctor((Functor0 *)0,*this,&FieldTester::postSetFieldValueCallback));

  modular_server::Field & long_array_field = modular_server_.createField(constants::long_array_field_name,constants::long_array_default);
  long_array_field.setRange(constants::long_array_element_min,constants::long_array_element_max);

  modular_server::Field & double_array_field = modular_server_.createField(constants::double_array_field_name,constants::double_array_default);
  double_array_field.setRange(constants::double_array_element_min,constants::double_array_element_max);

  modular_server::Field & bool_array_field = modular_server_.createField(constants::bool_array_field_name,constants::bool_array_default);
  bool_array_field.attachPreSetValueCallback(makeFunctor((Functor0 *)0,*this,&FieldTester::preSetFieldValueCallback));
  bool_array_field.attachPostSetValueCallback(makeFunctor((Functor0 *)0,*this,&FieldTester::postSetFieldValueCallback));
  bool_array_field.attachPreSetElementValueCallback(makeFunctor((Functor1<const size_t> *)0,*this,&FieldTester::preSetFieldElementValueCallback));
  bool_array_field.attachPostSetElementValueCallback(makeFunctor((Functor1<const size_t> *)0,*this,&FieldTester::postSetFieldElementValueCallback));

  modular_server::Field & string_field = modular_server_.createField(constants::string_field_name,constants::string_default);
  string_field.attachPreSetValueCallback(makeFunctor((Functor0 *)0,*this,&FieldTester::preSetFieldValueCallback));
  string_field.attachPostSetValueCallback(makeFunctor((Functor0 *)0,*this,&FieldTester::postSetFieldValueCallback));
  string_field.attachPreSetElementValueCallback(makeFunctor((Functor1<const size_t> *)0,*this,&FieldTester::preSetFieldElementValueCallback));
  string_field.attachPostSetElementValueCallback(makeFunctor((Functor1<const size_t> *)0,*this,&FieldTester::postSetFieldElementValueCallback));

  modular_server::Field & odd_field = modular_server_.createField(constants::odd_field_name,constants::odd_default);
  odd_field.setSubset(constants::odd_subset);

  modular_server::Field & mode_field = modular_server_.createField(constants::mode_field_name,constants::mode_ptr_default);
  mode_field.setSubset(constants::mode_ptr_subset);

  modular_server::Field & odd_array_field = modular_server_.createField(constants::odd_array_field_name,constants::odd_array_default);
  odd_array_field.setSubset(constants::odd_subset);

  // Parameters
  modular_server::Parameter & long_array_parameter = modular_server_.createParameter(constants::long_array_parameter_name);
  long_array_parameter.setTypeLong();
  long_array_parameter.setTypeArray();

  modular_server::Parameter & length_parameter = modular_server_.createParameter(constants::length_parameter_name);
  length_parameter.setRange(constants::length_min,constants::length_max);

  modular_server::Parameter & count_parameter = modular_server_.createParameter(constants::count_parameter_name);
  count_parameter.setTypeLong();
  count_parameter.setSubset(constants::count_subset);

  modular_server::Parameter & count_array_parameter = modular_server_.createParameter(constants::count_array_parameter_name);
  count_array_parameter.setTypeLong();
  count_array_parameter.setRange(constants::count_array_element_min,constants::count_array_element_max);
  count_array_parameter.setArrayLengthRange(constants::count_array_length_min,constants::count_array_length_max);
  count_array_parameter.setSubset(constants::count_subset);

  modular_server::Parameter & direction_parameter = modular_server_.createParameter(constants::direction_parameter_name);
  direction_parameter.setTypeString();
  direction_parameter.setSubset(constants::direction_ptr_subset);

  modular_server::Parameter & direction_array_parameter = modular_server_.createParameter(constants::direction_array_parameter_name);
  direction_array_parameter.setTypeString();
  direction_array_parameter.setArrayLengthRange(constants::direction_array_length_min,constants::direction_array_length_max);
  direction_array_parameter.setSubset(constants::direction_ptr_subset);

  // Methods
  modular_server::Method & get_doubled_method = modular_server_.createMethod(constants::get_doubled_method_name);
  get_doubled_method.attachCallback(makeFunctor((Functor0 *)0,*this,&FieldTester::getDoubledCallback));
  get_doubled_method.setReturnTypeDouble();

  modular_server::Method & get_bool_method = modular_server_.createMethod(constants::get_bool_method_name);
  get_bool_method.attachCallback(makeFunctor((Functor0 *)0,*this,&FieldTester::getBoolCallback));
  get_bool_method.setReturnTypeBool();

  modular_server::Method & get_long_array_fixed_method = modular_server_.createMethod(constants::get_long_array_fixed_method_name);
  get_long_array_fixed_method.attachCallback(makeFunctor((Functor0 *)0,*this,&FieldTester::getLongArrayFixedCallback));
  get_long_array_fixed_method.setReturnTypeArray();

  modular_server::Method & get_long_array_variable_method = modular_server_.createMethod(constants::get_long_array_variable_method_name);
  get_long_array_variable_method.attachCallback(makeFunctor((Functor0 *)0,*this,&FieldTester::getLongArrayVariableCallback));
  get_long_array_variable_method.setReturnTypeArray();

  modular_server::Method & set_long_array_fixed_method = modular_server_.createMethod(constants::set_long_array_fixed_method_name);
  set_long_array_fixed_method.attachCallback(makeFunctor((Functor0 *)0,*this,&FieldTester::setLongArrayFixedCallback));
  set_long_array_fixed_method.setReturnTypeBool();

  modular_server::Method & set_long_array_variable_method = modular_server_.createMethod(constants::set_long_array_variable_method_name);
  set_long_array_variable_method.attachCallback(makeFunctor((Functor0 *)0,*this,&FieldTester::setLongArrayVariableCallback));
  set_long_array_variable_method.setReturnTypeBool();

  modular_server::Method & set_long_array_parameter_method = modular_server_.createMethod(constants::set_long_array_parameter_method_name);
  set_long_array_parameter_method.attachCallback(makeFunctor((Functor0 *)0,*this,&FieldTester::setLongArrayParameterCallback));
  set_long_array_parameter_method.addParameter(long_array_parameter);
  set_long_array_parameter_method.setReturnTypeBool();

  modular_server::Method & get_string_all_method = modular_server_.createMethod(constants::get_string_all_method_name);
  get_string_all_method.attachCallback(makeFunctor((Functor0 *)0,*this,&FieldTester::getStringAllCallback));
  get_string_all_method.setReturnTypeString();

  modular_server::Method & get_string_some_method = modular_server_.createMethod(constants::get_string_some_method_name);
  get_string_some_method.attachCallback(makeFunctor((Functor0 *)0,*this,&FieldTester::getStringSomeCallback));
  get_string_some_method.addParameter(length_parameter);
  get_string_some_method.setReturnTypeString();

  modular_server::Method & get_count_method = modular_server_.createMethod(constants::get_count_method_name);
  get_count_method.attachCallback(makeFunctor((Functor0 *)0,*this,&FieldTester::getCountCallback));
  get_count_method.addParameter(count_parameter);
  get_count_method.setReturnTypeString();

  modular_server::Method & get_count_array_method = modular_server_.createMethod(constants::get_count_array_method_name);
  get_count_array_method.attachCallback(makeFunctor((Functor0 *)0,*this,&FieldTester::getCountArrayCallback));
  get_count_array_method.addParameter(count_array_parameter);
  get_count_array_method.setReturnTypeArray();

  modular_server::Method & get_direction_method = modular_server_.createMethod(constants::get_direction_method_name);
  get_direction_method.attachCallback(makeFunctor((Functor0 *)0,*this,&FieldTester::getDirectionCallback));
  get_direction_method.addParameter(direction_parameter);
  get_direction_method.setReturnTypeString();

  modular_server::Method & get_direction_array_method = modular_server_.createMethod(constants::get_direction_array_method_name);
  get_direction_array_method.attachCallback(makeFunctor((Functor0 *)0,*this,&FieldTester::getDirectionArrayCallback));
  get_direction_array_method.addParameter(direction_array_parameter);
  get_direction_array_method.setReturnTypeArray();

  modular_server::Method & check_mode_method = modular_server_.createMethod(constants::check_mode_method_name);
  check_mode_method.attachCallback(makeFunctor((Functor0 *)0,*this,&FieldTester::checkModeCallback));
  check_mode_method.setReturnTypeString();

  modular_server::Method & increment_mode_method = modular_server_.createMethod(constants::increment_mode_method_name);
  increment_mode_method.attachCallback(makeFunctor((Functor0 *)0,*this,&FieldTester::incrementModeCallback));
  increment_mode_method.setReturnTypeString();

  // Begin Streams
  Serial.begin(constants::baudrate);

  // Start Modular Device Server
  modular_server_.startServer();
}

void FieldTester::update()
{
  modular_server_.handleServerRequests();
}

// Callbacks must be non-blocking (avoid 'delay')
//
// modular_server_.getParameterValue must be cast to either:
// const char *
// long
// double
// bool
// ArduinoJson::JsonArray &
// ArduinoJson::JsonObject &
//
// For more info read about ArduinoJson parsing https://github.com/janelia-arduino/ArduinoJson
//
// modular_server_.getFieldValue type must match the field default type
// modular_server_.setFieldValue type must match the field default type
// modular_server_.getFieldElementValue type must match the field array element default type
// modular_server_.setFieldElementValue type must match the field array element default type

void FieldTester::getDoubledCallback()
{
  double value;
  modular_server_.getFieldValue(constants::double_field_name,value);
  value *= 2;
  modular_server_.writeResultToResponse(value);
}

void FieldTester::getBoolCallback()
{
  bool value;
  modular_server_.getFieldValue(constants::bool_field_name,value);
  modular_server_.writeResultToResponse(value);
}

void FieldTester::getLongArrayFixedCallback()
{
  long long_array[constants::LONG_ARRAY_LENGTH];
  modular_server_.getFieldValue(constants::long_array_field_name,long_array);
  modular_server_.writeResultToResponse(long_array);
}

void FieldTester::getLongArrayVariableCallback()
{
  size_t array_length = modular_server_.getFieldArrayLength(constants::long_array_field_name);
  long long_array[array_length];
  modular_server_.getFieldValue(constants::long_array_field_name,long_array,array_length);
  modular_server_.writeResultToResponse(long_array,array_length);
}

void FieldTester::setLongArrayFixedCallback()
{
  long long_array[constants::LONG_ARRAY_LENGTH];
  long_array[0] = 1;
  long_array[1] = 2;
  long_array[2] = 102;
  long_array[3] = 103;
  bool success = modular_server_.setFieldValue(constants::long_array_field_name,long_array);
  modular_server_.writeResultToResponse(success);
}

void FieldTester::setLongArrayVariableCallback()
{
  size_t array_length = modular_server_.getFieldArrayLength(constants::long_array_field_name);
  long long_array[array_length-1];
  long_array[0] = -1;
  long_array[1] = -2;
  long_array[2] = 9;
  bool success = modular_server_.setFieldValue(constants::long_array_field_name,long_array,array_length-1);
  modular_server_.writeResultToResponse(success);
}

void FieldTester::setLongArrayParameterCallback()
{
  ArduinoJson::JsonArray & long_array = modular_server_.getParameterValue(constants::long_array_parameter_name);
  bool success = modular_server_.setFieldValue(constants::long_array_field_name,long_array);
  modular_server_.writeResultToResponse(success);
}

void FieldTester::getStringAllCallback()
{
  size_t array_length = modular_server_.getFieldArrayLength(constants::string_field_name);
  char string[array_length];
  modular_server_.getFieldValue(constants::string_field_name,string,array_length);
  modular_server_.writeResultToResponse(string);
}

void FieldTester::getStringSomeCallback()
{
  long length = modular_server_.getParameterValue(constants::length_parameter_name);
  size_t array_length = length + 1;
  char string[array_length];
  modular_server_.getFieldValue(constants::string_field_name,string,array_length);
  modular_server_.writeResultToResponse(string);
}

void FieldTester::getCountCallback()
{
  long count = modular_server_.getParameterValue(constants::count_parameter_name);
  switch (count)
  {
    case 10:
    {
      modular_server_.writeResultToResponse("ten");
      break;
    }
    case 20:
    {
      modular_server_.writeResultToResponse("twenty");
      break;
    }
    case 30:
    {
      modular_server_.writeResultToResponse("thirty");
      break;
    }
    case 40:
    {
      modular_server_.writeResultToResponse("forty");
      break;
    }
    case 50:
    {
      modular_server_.writeResultToResponse("fifty");
      break;
    }
    case 60:
    {
      modular_server_.writeResultToResponse("sixty");
      break;
    }
    case 70:
    {
      modular_server_.writeResultToResponse("seventy");
      break;
    }
    default:
    {
      modular_server_.writeResultToResponse("???");
      break;
    }
  }
}

void FieldTester::getCountArrayCallback()
{
  ArduinoJson::JsonArray & count_array = modular_server_.getParameterValue(constants::count_array_parameter_name);
  modular_server_.writeResultToResponse(&count_array);
}

void FieldTester::getDirectionCallback()
{
  const char * direction = modular_server_.getParameterValue(constants::direction_parameter_name);
  modular_server_.writeResultToResponse(direction);
}

void FieldTester::getDirectionArrayCallback()
{
  ArduinoJson::JsonArray & direction_array = modular_server_.getParameterValue(constants::direction_array_parameter_name);
  modular_server_.writeResultToResponse(&direction_array);
}

void FieldTester::checkModeCallback()
{
  const ConstantString * mode_ptr;
  modular_server_.getFieldValue(constants::mode_field_name,mode_ptr);
  if (mode_ptr == &constants::mode_rising)
  {
    modular_server_.writeResultToResponse("Mode set to rising!");
  }
  else if (mode_ptr == &constants::mode_falling)
  {
    modular_server_.writeResultToResponse("Mode set to falling!");
  }
  else if (mode_ptr == &constants::mode_change)
  {
    modular_server_.writeResultToResponse("Mode set to change!");
  }
  else
  {
    modular_server_.writeResultToResponse("???");
  }
}

void FieldTester::incrementModeCallback()
{
  const ConstantString * mode_ptr;
  modular_server_.getFieldValue(constants::mode_field_name,mode_ptr);
  if (mode_ptr == &constants::mode_rising)
  {
    modular_server_.setFieldValue(constants::mode_field_name,&constants::mode_falling);
    modular_server_.writeResultToResponse("Mode was set to rising, now set to falling!");
  }
  else if (mode_ptr == &constants::mode_falling)
  {
    modular_server_.setFieldValue(constants::mode_field_name,&constants::mode_change);
    modular_server_.writeResultToResponse("Mode was set to falling, now set to change!");
  }
  else if (mode_ptr == &constants::mode_change)
  {
    modular_server_.setFieldValue(constants::mode_field_name,&constants::mode_rising);
    modular_server_.writeResultToResponse("Mode was set to change, now set to rising!");
  }
  else
  {
    modular_server_.writeResultToResponse("???");
  }
}

void FieldTester::preSetFieldValueCallback()
{
  // Writing to Serial like this makes response invalid!
  // Do not do this in a real device!
  // Uncomment next line just to demonstrate setFieldValueCallback

  // Serial << " setting field value! ";
}

void FieldTester::postSetFieldValueCallback()
{
  // Writing to Serial like this makes response invalid!
  // Do not do this in a real device!
  // Uncomment next line just to demonstrate setFieldValueCallback

  // Serial << " set field value! ";
}

void FieldTester::preSetFieldElementValueCallback(const size_t element_index)
{
  // Writing to Serial like this makes response invalid!
  // Do not do this in a real device!
  // Uncomment next line just to demonstrate setFieldElementValueCallback

  // Serial << " setting field element value at index " << element_index << "! ";
}

void FieldTester::postSetFieldElementValueCallback(const size_t element_index)
{
  // Writing to Serial like this makes response invalid!
  // Do not do this in a real device!
  // Uncomment next line just to demonstrate setFieldElementValueCallback

  // Serial << " set field element value at index " << element_index << "! ";
}