// ----------------------------------------------------------------------------
// Field.h
//
//
// Authors:
// Peter Polidoro polidorop@janelia.hhmi.org
// ----------------------------------------------------------------------------
#ifndef _MODULAR_DEVICE_FIELD_H_
#define _MODULAR_DEVICE_FIELD_H_
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include "SavedVariable.h"
#include "JsonStream.h"
#include "Vector.h"
#include "Parameter.h"
#include "Constants.h"


namespace ModularDevice
{
typedef void (*SetValueCallback)(void);
typedef void (*SetElementValueCallback)(const size_t);

class Field
{
public:
  Field();
  template <typename T>
  Field(const ConstantString & name,
        const T & default_value);
  template <size_t N>
  Field(const ConstantString & name,
        const long (&default_value)[N]):
    parameter_(name),
    saved_variable_(default_value,N)
  {
    parameter_.setTypeLong();
    parameter_.setArrayLengthRange(1,N);
    setCallbacksToNull();
  };
  template <size_t N>
  Field(const ConstantString & name,
        const double (&default_value)[N]) :
    parameter_(name),
    saved_variable_(default_value,N)
  {
    parameter_.setTypeDouble();
    parameter_.setArrayLengthRange(1,N);
    setCallbacksToNull();
  }
  template <size_t N>
  Field(const ConstantString & name,
        const bool (&default_value)[N]) :
    parameter_(name),
    saved_variable_(default_value,N)
  {
    parameter_.setTypeBool();
    parameter_.setArrayLengthRange(1,N);
    setCallbacksToNull();
  }
  template <size_t N>
  Field(const ConstantString & name,
        const char (&default_value)[N]) :
    parameter_(name),
    saved_variable_(default_value,N)
  {
    parameter_.setTypeString();
    string_saved_as_char_array_ = true;
    setCallbacksToNull();
  }
  void setUnits(const ConstantString & name);
  void setRange(const long min, const long max);
  void setRange(const double min, const double max);
  template <size_t N>
  void setSubset(const constants::SubsetMemberType (&subset)[N])
  {
    parameter_.setSubset(subset);
  }
  void attachPreSetValueCallback(SetValueCallback callback);
  void attachPreSetElementValueCallback(SetElementValueCallback callback);
  void attachPostSetValueCallback(SetValueCallback callback);
  void attachPostSetElementValueCallback(SetElementValueCallback callback);
private:
  Parameter parameter_;
  SavedVariable saved_variable_;
  SetValueCallback pre_set_value_callback_;
  SetElementValueCallback pre_set_element_value_callback_;
  SetValueCallback post_set_value_callback_;
  SetElementValueCallback post_set_element_value_callback_;
  bool string_saved_as_char_array_;

  // Saved Variable Methods
  template <typename T>
  bool getDefaultValue(T & value);
  template <size_t N>
  bool getDefaultValue(long (&value)[N])
  {
    if ((getType() != JsonStream::ARRAY_TYPE) ||
        (getArrayElementType() != JsonStream::LONG_TYPE) ||
        (getArrayLength() != N))
    {
      return false;
    }
    return saved_variable_.getDefaultValue(value);
  };
  template <size_t N>
  bool getDefaultValue(double (&value)[N])
  {
    if ((getType() != JsonStream::ARRAY_TYPE) ||
        (getArrayElementType() != JsonStream::DOUBLE_TYPE) ||
        (getArrayLength() != N))
    {
      return false;
    }
    return saved_variable_.getDefaultValue(value);
  };
  template <size_t N>
  bool getDefaultValue(bool (&value)[N])
  {
    if ((getType() != JsonStream::ARRAY_TYPE) ||
        (getArrayElementType() != JsonStream::BOOL_TYPE) ||
        (getArrayLength() != N))
    {
      return false;
    }
    return saved_variable_.getDefaultValue(value);
  };
  template <typename T>
  bool getDefaultElementValue(T & value, const size_t element_index);
  template <typename T>
  bool setValue(const T & value);
  template <size_t N>
  bool setValue(const long (&value)[N])
  {
    bool success = false;
    preSetValueCallback();
    if ((getType() == JsonStream::ARRAY_TYPE) &&
        (getArrayElementType() == JsonStream::LONG_TYPE) &&
        (getArrayLength() == N))
    {
      for (size_t i=0;i<N;++i)
      {
        success = setElementValue(value[i],i);
        if (!success)
        {
          break;
        }
      }
    }
    postSetValueCallback();
    return success;
  };
  template <size_t N>
  bool setValue(const double (&value)[N])
  {
    bool success = false;
    preSetValueCallback();
    if ((getType() == JsonStream::ARRAY_TYPE) &&
        (getArrayElementType() == JsonStream::DOUBLE_TYPE) &&
        (getArrayLength() &= N))
    {
      for (size_t i=0;i<N;++i)
      {
        success = setElementValue(value[i],i);
        if (!success)
        {
          break;
        }
      }
    }
    postSetValueCallback();
    return success;
  };
  template <size_t N>
  bool setValue(const bool (&value)[N])
  {
    bool success = false;
    preSetValueCallback();
    if ((getType() == JsonStream::ARRAY_TYPE) &&
        (getArrayElementType() == JsonStream::BOOL_TYPE) &&
        (getArrayLength() == N))
    {
      for (size_t i=0;i<N;++i)
      {
        success = setElementValue(value[i],i);
        if (!success)
        {
          break;
        }
      }
    }
    postSetValueCallback();
    return success;
  };
  template <typename T>
  bool setElementValue(const T & value, const size_t element_index);
  template <typename T>
  bool getValue(T & value);
  template <size_t N>
  bool getValue(long (&value)[N])
  {
    if ((getType() != JsonStream::ARRAY_TYPE) ||
        (getArrayElementType() != JsonStream::LONG_TYPE) ||
        (getArrayLength() != N))
    {
      return false;
    }
    return saved_variable_.getValue(value);
  };
  template <size_t N>
  bool getValue(double (&value)[N])
  {
    if ((getType() != JsonStream::ARRAY_TYPE) ||
        (getArrayElementType() != JsonStream::DOUBLE_TYPE) ||
        (getArrayLength() != N))
    {
      return false;
    }
    return saved_variable_.getValue(value);
  };
  template <size_t N>
  bool getValue(bool (&value)[N])
  {
    if ((getType() != JsonStream::ARRAY_TYPE) ||
        (getArrayElementType() != JsonStream::BOOL_TYPE) ||
        (getArrayLength() != N))
    {
      return false;
    }
    return saved_variable_.getValue(value);
  };
  template <typename T>
  bool getElementValue(T & value, const size_t element_index);
  void setDefaultValue();
  bool valueIsDefault();
  size_t getArrayLength();
  Parameter & getParameter();
  const ConstantString & getName();
  JsonStream::JsonTypes getType();
  JsonStream::JsonTypes getArrayElementType();
  bool stringIsSavedAsCharArray();
  int findSubsetValueIndex(const long value);
  int findSubsetValueIndex(const char * value);
  Vector<const constants::SubsetMemberType> & getSubset();
  void setCallbacksToNull();
  void preSetValueCallback();
  void preSetElementValueCallback(const size_t element_index);
  void postSetValueCallback();
  void postSetElementValueCallback(const size_t element_index);
  friend class Server;
};
}

#endif
