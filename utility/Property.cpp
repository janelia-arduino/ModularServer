// ----------------------------------------------------------------------------
// Property.cpp
//
//
// Authors:
// Peter Polidoro polidorop@janelia.hhmi.org
// ----------------------------------------------------------------------------
#include "Property.h"


namespace modular_server
{

namespace property
{
// Parameters
CONSTANT_STRING(function_parameter_name,"function");
CONSTANT_STRING(value_parameter_name,"value");

// Functions
CONSTANT_STRING(get_value_function_name,"getValue");
CONSTANT_STRING(set_value_function_name,"setValue");
CONSTANT_STRING(get_default_value_function_name,"getDefaultValue");
CONSTANT_STRING(set_value_to_default_function_name,"setValueToDefault");

// Array Parameters
CONSTANT_STRING(element_index_parameter_name,"element_index");
CONSTANT_STRING(element_value_parameter_name,"element_value");

// Array Functions
CONSTANT_STRING(get_element_value_function_name,"getElementValue");
CONSTANT_STRING(set_element_value_function_name,"setElementValue");
CONSTANT_STRING(get_default_element_value_function_name,"getDefaultElementValue");
CONSTANT_STRING(set_element_value_to_default_function_name,"setElementValueToDefault");
CONSTANT_STRING(set_all_element_values_function_name,"setAllElementValues");
}

Parameter Property::property_parameters_[property::PARAMETER_COUNT_MAX];
Function Property::property_functions_[property::FUNCTION_COUNT_MAX];
Parameter Property::property_array_parameters_[property::ARRAY_PARAMETER_COUNT_MAX];
Function Property::property_array_functions_[property::ARRAY_FUNCTION_COUNT_MAX];
ConcatenatedArray<Parameter,property::FUNCTION_PARAMETER_TYPE_COUNT> Property::parameters_;
ConcatenatedArray<Function,property::FUNCTION_PARAMETER_TYPE_COUNT> Property::functions_;
Response * Property::response_ptr_;
Functor4<Property &, bool, bool, int> Property::write_property_to_response_functor_;
Functor1wRet<const ConstantString &, ArduinoJson::JsonVariant> Property::get_parameter_value_functor_;
Functor2wRet<Parameter &, ArduinoJson::JsonVariant &, bool> Property::check_parameter_functor_;

Parameter & Property::createParameter(const ConstantString & parameter_name)
{
  int parameter_index = findParameterIndex(parameter_name);
  if (parameter_index < 0)
  {
    parameters_.push_back(Parameter(parameter_name));
    parameters_.back().setFirmwareName(constants::firmware_name);
    return parameters_.back();
  }
}

Parameter & Property::parameter(const ConstantString & parameter_name)
{
  int parameter_index = findParameterIndex(parameter_name);
  if ((parameter_index >= 0) && (parameter_index < (int)parameters_.size()))
  {
    return parameters_[parameter_index];
  }
}

Parameter & Property::copyParameter(Parameter parameter,const ConstantString & parameter_name)
{
  parameters_.push_back(parameter);
  parameters_.back().setName(parameter_name);
  return parameters_.back();
}

Function & Property::createFunction(const ConstantString & function_name)
{
  int function_index = findFunctionIndex(function_name);
  if (function_index < 0)
  {
    functions_.push_back(Function(function_name));
    functions_.back().setFirmwareName(constants::firmware_name);
    return functions_.back();
  }
}

Function & Property::function(const ConstantString & function_name)
{
  int function_index = findFunctionIndex(function_name);
  if ((function_index >= 0) && (function_index < (int)functions_.size()))
  {
    return functions_[function_index];
  }
}

// public
Property::Property()
{
  response_ptr_ = NULL;
}

void Property::setUnits(const ConstantString & name)
{
  parameter_.setUnits(name);
}

void Property::setRange(const long min, const long max)
{
  parameter_.setRange(min,max);
}

void Property::setRange(const double min, const double max)
{
  parameter_.setRange(min,max);
}

void Property::setSubset(constants::SubsetMemberType * subset, size_t max_size, size_t size)
{
  parameter_.setSubset(subset,max_size,size);
}

void Property::addValueToSubset(constants::SubsetMemberType & value)
{
  parameter_.addValueToSubset(value);
}

void Property::attachPreSetValueFunctor(const Functor0 & functor)
{
  pre_set_value_functor_ = functor;
}

void Property::attachPreSetElementValueFunctor(const Functor1<const size_t> & functor)
{
  pre_set_element_value_functor_ = functor;
}

void Property::attachPostSetValueFunctor(const Functor0 & functor)
{
  post_set_value_functor_ = functor;
}

void Property::attachPostSetElementValueFunctor(const Functor1<const size_t> & functor)
{
  post_set_element_value_functor_ = functor;
}

template <>
bool Property::getValue<long>(long & value)
{
  if (getType() != JsonStream::LONG_TYPE)
  {
    return false;
  }
  return saved_variable_.getValue(value);
}

template <>
bool Property::getValue<double>(double & value)
{
  if (getType() != JsonStream::DOUBLE_TYPE)
  {
    return false;
  }
  return saved_variable_.getValue(value);
}

template <>
bool Property::getValue<bool>(bool & value)
{
  if (getType() != JsonStream::BOOL_TYPE)
  {
    return false;
  }
  return saved_variable_.getValue(value);
}

bool Property::getValue(const ConstantString * & value)
{
  if ((getType() != JsonStream::STRING_TYPE) ||
      stringIsSavedAsCharArray())
  {
    value = NULL;
    return false;
  }
  return saved_variable_.getValue(value);
}

template <>
bool Property::getElementValue<long>(const size_t element_index, long & value)
{
  if ((getType() != JsonStream::ARRAY_TYPE) ||
      (getArrayElementType() != JsonStream::LONG_TYPE))
  {
    return false;
  }
  return saved_variable_.getElementValue(element_index,value);
}

template <>
bool Property::getElementValue<double>(const size_t element_index, double & value)
{
  if ((getType() != JsonStream::ARRAY_TYPE) ||
      (getArrayElementType() != JsonStream::DOUBLE_TYPE))
  {
    return false;
  }
  return saved_variable_.getElementValue(element_index,value);
}

template <>
bool Property::getElementValue<bool>(const size_t element_index, bool & value)
{
  if ((getType() != JsonStream::ARRAY_TYPE) ||
      (getArrayElementType() != JsonStream::BOOL_TYPE))
  {
    return false;
  }
  return saved_variable_.getElementValue(element_index,value);
}

template <>
bool Property::getElementValue<char>(const size_t element_index, char & value)
{
  if (getType() != JsonStream::STRING_TYPE)
  {
    return false;
  }
  if (!stringIsSavedAsCharArray())
  {
    const ConstantString * cs_ptr;
    getValue(cs_ptr);
    size_t str_len = cs_ptr->length();
    if (element_index <= str_len)
    {
      char str[str_len+1];
      cs_ptr->copy(str);
      value = str[element_index];
      return true;
    }
    else
    {
      return false;
    }
  }
  return saved_variable_.getElementValue(element_index,value);
}

template <>
bool Property::getDefaultValue<long>(long & value)
{
  if (getType() != JsonStream::LONG_TYPE)
  {
    return false;
  }
  return saved_variable_.getDefaultValue(value);
}

template <>
bool Property::getDefaultValue<double>(double & value)
{
  if (getType() != JsonStream::DOUBLE_TYPE)
  {
    return false;
  }
  return saved_variable_.getDefaultValue(value);
}

template <>
bool Property::getDefaultValue<bool>(bool & value)
{
  if (getType() != JsonStream::BOOL_TYPE)
  {
    return false;
  }
  return saved_variable_.getDefaultValue(value);
}

bool Property::getDefaultValue(const ConstantString * & value)
{
  if ((getType() != JsonStream::STRING_TYPE) ||
      stringIsSavedAsCharArray())
  {
    value = NULL;
    return false;
  }
  return saved_variable_.getDefaultValue(value);
}

template <>
bool Property::getDefaultElementValue<long>(const size_t element_index, long & value)
{
  if ((getType() != JsonStream::ARRAY_TYPE) ||
      (getArrayElementType() != JsonStream::LONG_TYPE))
  {
    return false;
  }
  return saved_variable_.getDefaultElementValue(element_index,value);
}

template <>
bool Property::getDefaultElementValue<double>(const size_t element_index, double & value)
{
  if ((getType() != JsonStream::ARRAY_TYPE) ||
      (getArrayElementType() != JsonStream::DOUBLE_TYPE))
  {
    return false;
  }
  return saved_variable_.getDefaultElementValue(element_index,value);
}

template <>
bool Property::getDefaultElementValue<bool>(const size_t element_index, bool & value)
{
  if ((getType() != JsonStream::ARRAY_TYPE) ||
      (getArrayElementType() != JsonStream::BOOL_TYPE))
  {
    return false;
  }
  return saved_variable_.getDefaultElementValue(element_index,value);
}

template <>
bool Property::getDefaultElementValue<char>(const size_t element_index, char & value)
{
  if (getType() != JsonStream::STRING_TYPE)
  {
    return false;
  }
  if (!stringIsSavedAsCharArray())
  {
    const ConstantString * cs_ptr;
    getDefaultValue(cs_ptr);
    size_t str_len = cs_ptr->length();
    if (element_index <= str_len)
    {
      char str[str_len+1];
      cs_ptr->copy(str);
      value = str[element_index];
      return true;
    }
    else
    {
      return false;
    }
  }
  return saved_variable_.getDefaultElementValue(element_index,value);
}

template <>
bool Property::setElementValue<long>(const size_t element_index, const long & value)
{
  bool success = false;
  preSetElementValueFunctor(element_index);
  if ((getType() == JsonStream::ARRAY_TYPE) &&
      (getArrayElementType() == JsonStream::LONG_TYPE))
  {
    if (parameter_.rangeIsSet())
    {
      long min = parameter_.getMin().l;
      long max = parameter_.getMax().l;
      if ((value >= min) && (value <= max))
      {
        success = saved_variable_.setElementValue(element_index,value);
      }
    }
    else
    {
      success = saved_variable_.setElementValue(element_index,value);
    }
  }
  postSetElementValueFunctor(element_index);
  return success;
}

template <>
bool Property::setElementValue<double>(const size_t element_index, const double & value)
{
  bool success = false;
  preSetElementValueFunctor(element_index);
  if ((getType() == JsonStream::ARRAY_TYPE) &&
      (getArrayElementType() == JsonStream::DOUBLE_TYPE))
  {
    if (parameter_.rangeIsSet())
    {
      double min = parameter_.getMin().d;
      double max = parameter_.getMax().d;
      if ((value >= min) && (value <= max))
      {
        success = saved_variable_.setElementValue(element_index,value);
      }
    }
    else
    {
      success = saved_variable_.setElementValue(element_index,value);
    }
  }
  postSetElementValueFunctor(element_index);
  return success;
}

template <>
bool Property::setElementValue<bool>(const size_t element_index, const bool & value)
{
  bool success = false;
  preSetElementValueFunctor(element_index);
  if ((getType() == JsonStream::ARRAY_TYPE) &&
      (getArrayElementType() == JsonStream::BOOL_TYPE))
  {
    success = saved_variable_.setElementValue(element_index,value);
  }
  postSetElementValueFunctor(element_index);
  return success;
}

template <>
bool Property::setElementValue<char>(const size_t element_index, const char & value)
{
  bool success = false;
  preSetElementValueFunctor(element_index);
  if ((getType() == JsonStream::STRING_TYPE) &&
      (stringIsSavedAsCharArray()))
  {
    success = saved_variable_.setElementValue(element_index,value);
  }
  postSetElementValueFunctor(element_index);
  return success;
}

template <>
bool Property::setValue<long>(const long & value)
{
  bool success = false;
  preSetValueFunctor();
  if (getType() == JsonStream::LONG_TYPE)
  {
    if (parameter_.rangeIsSet())
    {
      long min = parameter_.getMin().l;
      long max = parameter_.getMax().l;
      if ((value >= min) && (value <= max))
      {
        success = saved_variable_.setValue(value);
      }
    }
    else
    {
      success = saved_variable_.setValue(value);
    }
  }
  postSetValueFunctor();
  return success;
}

template <>
bool Property::setValue<double>(const double & value)
{
  bool success = false;
  preSetValueFunctor();
  if (getType() == JsonStream::DOUBLE_TYPE)
  {
    if (parameter_.rangeIsSet())
    {
      double min = parameter_.getMin().d;
      double max = parameter_.getMax().d;
      if ((value >= min) && (value <= max))
      {
        success = saved_variable_.setValue(value);
      }
    }
    else
    {
      success = saved_variable_.setValue(value);
    }
  }
  postSetValueFunctor();
  return success;
}

template <>
bool Property::setValue<bool>(const bool & value)
{
  bool success = false;
  preSetValueFunctor();
  if (getType() == JsonStream::BOOL_TYPE)
  {
    success = saved_variable_.setValue(value);
  }
  postSetValueFunctor();
  return success;
}

template <>
bool Property::setValue<const ConstantString *>(const ConstantString * const & value)
{
  bool success = false;
  preSetValueFunctor();
  if ((getType() == JsonStream::STRING_TYPE) &&
      !stringIsSavedAsCharArray())
  {
    success = saved_variable_.setValue(value);
  }
  postSetValueFunctor();
  return success;
}

template <>
bool Property::setValue<ConstantString *>(ConstantString * const & value)
{
  bool success = false;
  preSetValueFunctor();
  if ((getType() == JsonStream::STRING_TYPE) &&
      !stringIsSavedAsCharArray())
  {
    success = saved_variable_.setValue(value);
  }
  postSetValueFunctor();
  return success;
}

bool Property::setValue(ArduinoJson::JsonArray & value)
{
  bool success = false;
  size_t array_length = getArrayLength();
  JsonStream::JsonTypes property_type = getType();
  if (property_type == JsonStream::ARRAY_TYPE)
  {
    size_t N = value.size();
    size_t array_length_min = min(array_length,N);
    JsonStream::JsonTypes array_element_type = getArrayElementType();
    preSetValueFunctor();
    switch (array_element_type)
    {
      case JsonStream::LONG_TYPE:
      {
        for (size_t i=0; i<array_length_min; ++i)
        {
          long v = value[i];
          success = setElementValue<long>(i,v);
          if (!success)
          {
            break;
          }
        }
        break;
      }
      case JsonStream::DOUBLE_TYPE:
      {
        for (size_t i=0; i<array_length_min; ++i)
        {
          double v = value[i];
          success = setElementValue<double>(i,v);
          if (!success)
          {
            break;
          }
        }
        break;
      }
      case JsonStream::BOOL_TYPE:
      {
        for (size_t i=0; i<array_length_min; ++i)
        {
          bool v = value[i];
          success = setElementValue<bool>(i,v);
          if (!success)
          {
            break;
          }
        }
        break;
      }
      case JsonStream::NULL_TYPE:
      {
        break;
      }
      case JsonStream::STRING_TYPE:
      {
        break;
      }
      case JsonStream::OBJECT_TYPE:
      {
        break;
      }
      case JsonStream::ARRAY_TYPE:
      {
        break;
      }
      case JsonStream::ANY_TYPE:
      {
        break;
      }
    }
    postSetValueFunctor();
  }
  return success;
}

void Property::setValueToDefault()
{
  preSetValueFunctor();
  saved_variable_.setValueToDefault();
  postSetValueFunctor();
}

void Property::setElementValueToDefault(const size_t element_index)
{
  preSetElementValueFunctor(element_index);
  saved_variable_.setElementValueToDefault(element_index);
  postSetElementValueFunctor(element_index);
}

bool Property::valueIsDefault()
{
  return saved_variable_.valueIsDefault();
}

size_t Property::getArrayLength()
{
  if ((getType() == JsonStream::STRING_TYPE) &&
      (!stringIsSavedAsCharArray()))
  {
    const ConstantString * cs_ptr;
    getValue(cs_ptr);
    return cs_ptr->length() + 1;
  }
  return saved_variable_.getArrayLength();
}

size_t Property::getStringLength()
{
  size_t length = 0;
  JsonStream::JsonTypes property_type = getType();
  if (property_type == JsonStream::STRING_TYPE)
  {
    size_t array_length_max = getArrayLength();
    size_t array_length = 1;
    char value;
    while (array_length < array_length_max)
    {
      getElementValue(array_length-1,value);
      if (value == 0)
      {
        return array_length - 1;
      }
      ++array_length;
    }
    return array_length_max;
  }
  return length;
}

// private
template <>
Property::Property<long>(const ConstantString & name,
                         const long & default_value) :
parameter_(name),
  saved_variable_(default_value)
{
  parameter_.setTypeLong();
}

template <>
Property::Property<double>(const ConstantString & name,
                           const double & default_value) :
parameter_(name),
  saved_variable_(default_value)
{
  parameter_.setTypeDouble();
}

template <>
Property::Property<bool>(const ConstantString & name,
                         const bool & default_value) :
parameter_(name),
  saved_variable_(default_value)
{
  parameter_.setTypeBool();
}

template <>
Property::Property<const ConstantString *>(const ConstantString & name,
                                           const ConstantString * const & default_value) :
parameter_(name),
  saved_variable_(default_value)
{
  parameter_.setTypeString();
  string_saved_as_char_array_ = false;
}

Parameter & Property::parameter()
{
  return parameter_;
}

bool Property::compareName(const char * name_to_compare)
{
  return parameter_.compareName(name_to_compare);
}

bool Property::compareName(const ConstantString & name_to_compare)
{
  return parameter_.compareName(name_to_compare);
}

const ConstantString & Property::getName()
{
  return parameter_.getName();
}

const ConstantString & Property::getFirmwareName()
{
  return parameter_.getFirmwareName();
}

bool Property::firmwareNameInArray(ArduinoJson::JsonArray & firmware_name_array)
{
  return parameter_.firmwareNameInArray(firmware_name_array);
}

JsonStream::JsonTypes Property::getType()
{
  return parameter_.getType();
}

JsonStream::JsonTypes Property::getArrayElementType()
{
  return parameter_.getArrayElementType();
}

bool Property::stringIsSavedAsCharArray()
{
  return string_saved_as_char_array_;
}

int Property::findSubsetValueIndex(const long value)
{
  return parameter_.findSubsetValueIndex(value);
}

int Property::findSubsetValueIndex(const char * value)
{
  return parameter_.findSubsetValueIndex(value);
}

Vector<constants::SubsetMemberType> & Property::getSubset()
{
  return parameter_.getSubset();
}

void Property::preSetValueFunctor()
{
  if (pre_set_value_functor_)
  {
    pre_set_value_functor_();
  }
}

void Property::preSetElementValueFunctor(const size_t element_index)
{
  if (pre_set_element_value_functor_)
  {
    pre_set_element_value_functor_(element_index);
  }
}

void Property::postSetValueFunctor()
{
  if (post_set_value_functor_)
  {
    post_set_value_functor_();
  }
}

void Property::postSetElementValueFunctor(const size_t element_index)
{
  if (post_set_element_value_functor_)
  {
    post_set_element_value_functor_(element_index);
  }
}

void Property::updateFunctionsAndParameters()
{
  JsonStream::JsonTypes type = getType();

  // Parameters
  parameters_.clear();
  parameters_.addArray(property_parameters_);

  Parameter & value_parameter = copyParameter(parameter(),property::value_parameter_name);

  // Functions
  functions_.clear();
  functions_.addArray(property_functions_);

  Function & get_value_function = createFunction(property::get_value_function_name);
  get_value_function.attachFunctor(makeFunctor((Functor0 *)0,*this,&Property::getValueHandler));
  get_value_function.setReturnType(type);

  Function & set_value_function = createFunction(property::set_value_function_name);
  set_value_function.attachFunctor(makeFunctor((Functor0 *)0,*this,&Property::setValueHandler));
  set_value_function.addParameter(value_parameter);

  Function & get_default_value_function = createFunction(property::get_default_value_function_name);
  get_default_value_function.attachFunctor(makeFunctor((Functor0 *)0,*this,&Property::getDefaultValueHandler));
  get_default_value_function.setReturnType(type);

  Function & set_value_to_default_function = createFunction(property::set_value_to_default_function_name);
  set_value_to_default_function.attachFunctor(makeFunctor((Functor0 *)0,*this,&Property::setValueToDefaultHandler));

  if ((type == JsonStream::ARRAY_TYPE) || ((type == JsonStream::STRING_TYPE) && stringIsSavedAsCharArray()))
  {
    JsonStream::JsonTypes array_element_type = getArrayElementType();

    // Array Parameters
    parameters_.addArray(property_array_parameters_);

    Parameter & element_index_parameter = createParameter(property::element_index_parameter_name);
    element_index_parameter.setTypeLong();
    element_index_parameter.setRange(parameter().getArrayLengthMin(),parameter().getArrayLengthMax());

    Parameter & element_value_parameter = copyParameter(parameter().getElementParameter(),property::element_value_parameter_name);

    // Array Functions
    functions_.addArray(property_array_functions_);

    Function & get_element_value_function = createFunction(property::get_element_value_function_name);
    get_element_value_function.attachFunctor(makeFunctor((Functor0 *)0,*this,&Property::getElementValueHandler));
    get_element_value_function.addParameter(element_index_parameter);
    get_element_value_function.setReturnType(array_element_type);

    Function & set_element_value_function = createFunction(property::set_element_value_function_name);
    set_element_value_function.attachFunctor(makeFunctor((Functor0 *)0,*this,&Property::setElementValueHandler));
    set_element_value_function.addParameter(element_index_parameter);
    set_element_value_function.addParameter(element_value_parameter);

    Function & get_default_element_value_function = createFunction(property::get_default_element_value_function_name);
    get_default_element_value_function.attachFunctor(makeFunctor((Functor0 *)0,*this,&Property::getDefaultElementValueHandler));
    get_default_element_value_function.addParameter(element_index_parameter);
    get_default_element_value_function.setReturnType(array_element_type);

    Function & set_element_value_to_default_function = createFunction(property::set_element_value_to_default_function_name);
    set_element_value_to_default_function.attachFunctor(makeFunctor((Functor0 *)0,*this,&Property::setElementValueToDefaultHandler));
    set_element_value_to_default_function.addParameter(element_index_parameter);

    Function & set_all_element_values_function = createFunction(property::set_all_element_values_function_name);
    set_all_element_values_function.attachFunctor(makeFunctor((Functor0 *)0,*this,&Property::setAllElementValuesHandler));
    set_all_element_values_function.addParameter(element_value_parameter);
  }
}

void Property::getValueHandler()
{
  response_ptr_->writeResultKey();
  write_property_to_response_functor_(*this,false,false,-1);
}

void Property::setValueHandler()
{
  ArduinoJson::JsonVariant json_value = get_parameter_value_functor_(property::value_parameter_name);
  bool parameter_ok = check_parameter_functor_(parameter(property::value_parameter_name),json_value);
  if (!parameter_ok)
  {
    return;
  }
  JsonStream::JsonTypes type = getType();
  switch (type)
  {
    case JsonStream::LONG_TYPE:
    {
      long value = get_parameter_value_functor_(property::value_parameter_name);
      setValue(value);
      break;
    }
    case JsonStream::DOUBLE_TYPE:
    {
      double value = get_parameter_value_functor_(property::value_parameter_name);
      setValue(value);
      break;
    }
    case JsonStream::BOOL_TYPE:
    {
      bool value = get_parameter_value_functor_(property::value_parameter_name);
      setValue(value);
      break;
    }
    case JsonStream::NULL_TYPE:
    {
      break;
    }
    case JsonStream::STRING_TYPE:
    {
      const char * value = get_parameter_value_functor_(property::value_parameter_name);
      size_t array_length = strlen(value) + 1;
      setValue(value,array_length);
      break;
    }
    case JsonStream::OBJECT_TYPE:
    {
      break;
    }
    case JsonStream::ARRAY_TYPE:
    {
      ArduinoJson::JsonArray & value = get_parameter_value_functor_(property::value_parameter_name);
      setValue(value);
      break;
    }
    case JsonStream::ANY_TYPE:
    {
      break;
    }
  }
}

void Property::getDefaultValueHandler()
{
  response_ptr_->writeResultKey();
  write_property_to_response_functor_(*this,false,true,-1);
}

void Property::setValueToDefaultHandler()
{
  setValueToDefault();
}

void Property::getElementValueHandler()
{
  ArduinoJson::JsonVariant json_value = get_parameter_value_functor_(property::element_index_parameter_name);
  bool parameter_ok = check_parameter_functor_(parameter(property::element_index_parameter_name),json_value);
  if (!parameter_ok)
  {
    return;
  }
  long element_index = get_parameter_value_functor_(property::element_index_parameter_name);
  response_ptr_->writeResultKey();
  write_property_to_response_functor_(*this,false,false,element_index);
}

void Property::setElementValueHandler()
{
  ArduinoJson::JsonVariant json_value = get_parameter_value_functor_(property::element_index_parameter_name);
  bool parameter_ok = check_parameter_functor_(parameter(property::element_index_parameter_name),json_value);
  if (!parameter_ok)
  {
    return;
  }
  long element_index = get_parameter_value_functor_(property::element_index_parameter_name);

  json_value = get_parameter_value_functor_(property::element_value_parameter_name);
  parameter_ok = check_parameter_functor_(parameter(property::element_value_parameter_name),json_value);
  if (!parameter_ok)
  {
    return;
  }

  JsonStream::JsonTypes type = getType();
  switch (type)
  {
    case JsonStream::LONG_TYPE:
    {
      response_ptr_->returnParameterInvalidError(constants::property_not_array_type_error_data);
      break;
    }
    case JsonStream::DOUBLE_TYPE:
    {
      response_ptr_->returnParameterInvalidError(constants::property_not_array_type_error_data);
      break;
    }
    case JsonStream::BOOL_TYPE:
    {
      response_ptr_->returnParameterInvalidError(constants::property_not_array_type_error_data);
      break;
    }
    case JsonStream::NULL_TYPE:
    {
      break;
    }
    case JsonStream::STRING_TYPE:
    {
      if (!stringIsSavedAsCharArray())
      {
        response_ptr_->returnParameterInvalidError(constants::cannot_set_element_in_string_property_with_subset_error_data);
        break;
      }
      size_t array_length = getArrayLength();
      if ((size_t)element_index >= (array_length - 1))
      {
        response_ptr_->returnParameterInvalidError(constants::property_element_index_out_of_bounds_error_data);
        return;
      }
      const char * value = get_parameter_value_functor_(property::element_value_parameter_name);
      size_t string_length = strlen(value);
      if (string_length >= 1)
      {
        char v = value[0];
        setElementValue(element_index,v);
      }
      break;
    }
    case JsonStream::OBJECT_TYPE:
    {
      break;
    }
    case JsonStream::ARRAY_TYPE:
    {
      size_t array_length = getArrayLength();
      if ((size_t)element_index >= array_length)
      {
        response_ptr_->returnParameterInvalidError(constants::property_element_index_out_of_bounds_error_data);
        return;
      }
      JsonStream::JsonTypes array_element_type = getArrayElementType();
      switch (array_element_type)
      {
        case JsonStream::LONG_TYPE:
        {
          long value = get_parameter_value_functor_(property::element_value_parameter_name);
          setElementValue(element_index,value);
          break;
        }
        case JsonStream::DOUBLE_TYPE:
        {
          double value = get_parameter_value_functor_(property::element_value_parameter_name);
          setElementValue(element_index,value);
          break;
        }
        case JsonStream::BOOL_TYPE:
        {
          bool value = get_parameter_value_functor_(property::element_value_parameter_name);
          setElementValue(element_index,value);
          break;
        }
        case JsonStream::NULL_TYPE:
        {
          break;
        }
        case JsonStream::STRING_TYPE:
        {
          break;
        }
        case JsonStream::OBJECT_TYPE:
        {
          break;
        }
        case JsonStream::ARRAY_TYPE:
        {
          break;
        }
        case JsonStream::ANY_TYPE:
        {
          break;
        }
      }
      break;
    }
    case JsonStream::ANY_TYPE:
    {
      break;
    }
  }
}

void Property::getDefaultElementValueHandler()
{
  ArduinoJson::JsonVariant json_value = get_parameter_value_functor_(property::element_index_parameter_name);
  bool parameter_ok = check_parameter_functor_(parameter(property::element_index_parameter_name),json_value);
  if (!parameter_ok)
  {
    return;
  }
  long element_index = get_parameter_value_functor_(property::element_index_parameter_name);
  response_ptr_->writeResultKey();
  write_property_to_response_functor_(*this,false,true,element_index);
}

void Property::setElementValueToDefaultHandler()
{
  ArduinoJson::JsonVariant json_value = get_parameter_value_functor_(property::element_index_parameter_name);
  bool parameter_ok = check_parameter_functor_(parameter(property::element_index_parameter_name),json_value);
  if (!parameter_ok)
  {
    return;
  }
  long element_index = get_parameter_value_functor_(property::element_index_parameter_name);
  setElementValueToDefault(element_index);
}

void Property::setAllElementValuesHandler()
{
  ArduinoJson::JsonVariant json_value = get_parameter_value_functor_(property::element_value_parameter_name);
  bool parameter_ok = check_parameter_functor_(parameter(property::element_value_parameter_name),json_value);
  if (!parameter_ok)
  {
    return;
  }

  JsonStream::JsonTypes type = getType();
  switch (type)
  {
    case JsonStream::LONG_TYPE:
    {
      break;
    }
    case JsonStream::DOUBLE_TYPE:
    {
      break;
    }
    case JsonStream::BOOL_TYPE:
    {
      break;
    }
    case JsonStream::NULL_TYPE:
    {
      break;
    }
    case JsonStream::STRING_TYPE:
    {
      if (!stringIsSavedAsCharArray())
      {
        response_ptr_->returnParameterInvalidError(constants::cannot_set_element_in_string_property_with_subset_error_data);
        break;
      }
      const char * value = get_parameter_value_functor_(property::element_value_parameter_name);
      size_t string_length = strlen(value);
      if (string_length >= 1)
      {
        char v = value[0];
        setAllElementValues(v);
      }
      break;
    }
    case JsonStream::OBJECT_TYPE:
    {
      break;
    }
    case JsonStream::ARRAY_TYPE:
    {
      JsonStream::JsonTypes array_element_type = getArrayElementType();
      switch (array_element_type)
      {
        case JsonStream::LONG_TYPE:
        {
          long value = get_parameter_value_functor_(property::element_value_parameter_name);
          setAllElementValues(value);
          break;
        }
        case JsonStream::DOUBLE_TYPE:
        {
          double value = get_parameter_value_functor_(property::element_value_parameter_name);
          setAllElementValues(value);
          break;
        }
        case JsonStream::BOOL_TYPE:
        {
          bool value = get_parameter_value_functor_(property::element_value_parameter_name);
          setAllElementValues(value);
          break;
        }
        case JsonStream::NULL_TYPE:
        {
          break;
        }
        case JsonStream::STRING_TYPE:
        {
          break;
        }
        case JsonStream::OBJECT_TYPE:
        {
          break;
        }
        case JsonStream::ARRAY_TYPE:
        {
          break;
        }
        case JsonStream::ANY_TYPE:
        {
          break;
        }
      }
      break;
    }
    case JsonStream::ANY_TYPE:
    {
      break;
    }
  }
}

}