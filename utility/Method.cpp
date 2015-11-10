// ----------------------------------------------------------------------------
// Method.cpp
//
//
// Authors:
// Peter Polidoro polidorop@janelia.hhmi.org
// ----------------------------------------------------------------------------
#include "Method.h"


namespace ModularDevice
{
CONSTANT_STRING(default_method_name,"");

Method::Method()
{
  setName(default_method_name);
  callback_attached_ = false;
  reserved_ = false;
  parameter_count_ = 0;
  return_type_ = JsonPrinter::NULL_TYPE;
}

Method::Method(const ConstantString &name)
{
  setName(name);
  callback_attached_ = false;
  reserved_ = false;
  parameter_count_ = 0;
  return_type_ = JsonPrinter::NULL_TYPE;
}

void Method::setName(const ConstantString &name)
{
  name_ptr_ = &name;
}

void Method::attachCallback(Callback callback)
{
  callback_ = callback;
  callback_attached_ = true;
  reserved_ = false;
}

void Method::addParameter(Parameter &parameter)
{
  char parameter_name[constants::STRING_LENGTH_PARAMETER_NAME] = {0};
  const ConstantString* parameter_name_ptr = parameter.getNamePointer();
  parameter_name_ptr->copy(parameter_name);
  if (strlen(parameter_name) > 0)
  {
    int parameter_index = findParameterIndex(*parameter_name_ptr);
    if (parameter_index < 0)
    {
      parameter_ptr_array_.push_back(&parameter);
      parameter_count_++;
    }
  }
}

bool Method::compareName(const char *name_to_compare)
{
  char name[constants::STRING_LENGTH_METHOD_NAME] = {0};
  name_ptr_->copy(name);
  return String(name).equalsIgnoreCase(name_to_compare);
}

bool Method::compareName(const ConstantString &name_to_compare)
{
  return (&name_to_compare == name_ptr_);
}

const ConstantString* Method::getNamePointer()
{
  return name_ptr_;
}

void Method::callback()
{
  if ((callback_attached_) && (!isReserved()))
  {
    (*callback_)();
  }
}

void Method::attachReservedCallback(ReservedCallback callback)
{
  reserved_callback_ = callback;
  callback_attached_ = true;
  reserved_ = true;
}

bool Method::isReserved()
{
  return reserved_;
}

void Method::reservedCallback(Server *server)
{
  if ((callback_attached_) && (isReserved()))
  {
    (server->*reserved_callback_)();
  }
}

int Method::findParameterIndex(const ConstantString &parameter_name)
{
  int parameter_index = -1;
  for (unsigned int i=0; i<parameter_ptr_array_.size(); ++i)
  {
    if (parameter_ptr_array_[i]->compareName(parameter_name))
    {
      parameter_index = i;
      break;
    }
  }
  return parameter_index;
}

void Method::setReturnTypeLong()
{
  return_type_ = JsonPrinter::LONG_TYPE;
}

void Method::setReturnTypeDouble()
{
  return_type_ = JsonPrinter::DOUBLE_TYPE;
}

void Method::setReturnTypeBool()
{
  return_type_ = JsonPrinter::BOOL_TYPE;
}

void Method::setReturnTypeNull()
{
  return_type_ = JsonPrinter::NULL_TYPE;
}

void Method::setReturnTypeString()
{
  return_type_ = JsonPrinter::STRING_TYPE;
}

void Method::setReturnTypeObject()
{
  return_type_ = JsonPrinter::OBJECT_TYPE;
}

void Method::setReturnTypeArray()
{
  return_type_ = JsonPrinter::ARRAY_TYPE;
}

JsonPrinter::JsonTypes Method::getReturnType()
{
  return return_type_;
}
}
