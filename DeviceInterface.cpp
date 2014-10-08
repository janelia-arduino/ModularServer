// ----------------------------------------------------------------------------
// DeviceInterface.cpp
//
//
// Authors:
// Peter Polidoro polidorop@janelia.hhmi.org
// ----------------------------------------------------------------------------
#include "DeviceInterface.h"

Command::Command(char *name="")
{
  setName(name);
  callback_attached_ = false;
  reserved_ = false;
}

void Command::setName(char *name)
{
  strncpy(name_,name,CMD_NAME_LENGTH_MAX);
}

boolean Command::compareName(char *name_to_compare)
{
  return String(name_).equalsIgnoreCase(name_to_compare);
}

char* Command::getName()
{
  return name_;
}

void Command::printName()
{
  Serial << name_ << endl;
}

void Command::attachCallback(Callback callback)
{
  callback_ = callback;
  callback_attached_ = true;
  reserved_ = false;
}

void Command::callback()
{
  if ((callback_attached_) && (!isReserved()))
  {
    (*callback_)();
  }
}

void Command::attachReservedCallback(ReservedCallback callback)
{
  reserved_callback_ = callback;
  callback_attached_ = true;
  reserved_ = true;
}

boolean Command::isReserved()
{
  return reserved_;
}

void Command::reservedCallback(DeviceInterface *dev_int)
{
  if ((callback_attached_) && (isReserved()))
  {
    (dev_int->*reserved_callback_)();
  }
}

DeviceInterface::DeviceInterface(Stream &stream)
{
  stream_ = &stream;
  model_number_ = 0;
  serial_number_ = 0;
  firmware_number_ = 0;
  Command get_device_info_cmd("getDeviceInfo");
  get_device_info_cmd.attachReservedCallback(&DeviceInterface::getDeviceInfoCallback);
  addCommand(get_device_info_cmd);
  Command get_commands_cmd("getCommands");
  get_commands_cmd.attachReservedCallback(&DeviceInterface::getCommandsCallback);
  addCommand(get_commands_cmd);
  Command get_response_codes_cmd("getResponseCodes");
  get_response_codes_cmd.attachReservedCallback(&DeviceInterface::getResponseCodesCallback);
  addCommand(get_response_codes_cmd);
}

void DeviceInterface::setMessageStream(Stream &stream)
{
  stream_ = &stream;
}

void DeviceInterface::processMessage()
{
  while (stream_->available() > 0)
  {
    int message_length = stream_->readBytesUntil(EOL,message_,MESSAGE_LENGTH_MAX);
    if (message_length == 0)
    {
      continue;
    }
    message_[message_length] = '\0';
    response = Generator::JsonObject<JSON_RESPONSE_SIZE>();
    if (message_[0] == JSON_OBJECT_START_CHAR)
    {
      message_type_ = OBJECT_MESSAGE;
      Parser::JsonObject json_object = parser_.parse(message_);
      if (json_object.success())
      {
        processObjectMessage(json_object);
      }
      else
      {
        response["status"] = ERROR;
        response["err_msg"] = "Parsing JSON object message failed! Could be invalid JSON or too many tokens.";
        response["received_msg"] = message_;
      }
    }
    else
    {
      message_type_ = ARRAY_MESSAGE;
      if (message_[0] != JSON_ARRAY_START_CHAR)
      {
        message_type_ = COMMAND_LINE_MESSAGE;
        String msg_str = String("[") + String(message_) + String("]");
        msg_str.toCharArray(message_,MESSAGE_LENGTH_MAX);
      }
      Parser::JsonArray json_array = parser_.parse(message_);
      if (json_array.success())
      {
        processArrayMessage(json_array);
      }
      else
      {
        response["status"] = ERROR;
        response["err_msg"] = "Parsing JSON array message failed! Could be invalid JSON or too many tokens.";
        response["received_msg"] = message_;
      }
      if (!response.containsKey("status"))
      {
        response["status"] = SUCCESS;
      }
      if (message_type_ == COMMAND_LINE_MESSAGE)
      {
        response.prettyPrintTo(*stream_);
      }
      else
      {
        response.printTo(*stream_);
      }
      *stream_ << endl;
    }
  }
}

void DeviceInterface::addCommand(Command cmd)
{
  char* name = cmd.getName();
  if (String(name).length() > 0)
  {
    int command_index = getCommandIndex(name);
    if (command_index < 0)
    {
      command_vector_.push_back(cmd);
    }
    else
    {
      command_vector_[command_index] = Command(name);
    }
  }
}

void DeviceInterface::setModelNumber(int model_number)
{
  model_number_ = model_number;
}

void DeviceInterface::setFirmwareNumber(int firmware_number)
{
  firmware_number_ = firmware_number;
}

// Command DeviceInterface::getCommandByName(char *name)
// {
//   int command_index = getCommandIndex(name);
//   if (command_index >= 0)
//   {
//     return command_vector_[command_index];
//   }
//   else
//   {
//     return Command(name);
//   }
// }

void DeviceInterface::processObjectMessage(Parser::JsonObject &json_object)
{
}

void DeviceInterface::processArrayMessage(Parser::JsonArray &json_array)
{
  char* cmd = json_array[0];
  processCommand(cmd);
}

void DeviceInterface::processCommand(char *command_str)
{
  int command_id = String(command_str).toInt();
  int command_index;
  if (String(command_str).compareTo(String("0")) == 0)
  {
    command_index = 0;
    response["cmd_id"] = 0;
  }
  else if (command_id > 0)
  {
    command_index = command_id;
    response["cmd_id"] = command_id;
  }
  else
  {
    command_index = getCommandIndex(command_str);
    response["cmd"] = command_str;
  }
  if ((command_index >= 0) && (command_index < command_vector_.size()))
  {
    if (command_vector_[command_index].isReserved())
    {
      command_vector_[command_index].reservedCallback(this);
    }
    else
    {
      command_vector_[command_index].callback();
    }
  }
  else
  {
    response["status"] = ERROR;
    response["err_msg"] = "Unknown command.";
  }
}

int DeviceInterface::getCommandIndex(char *command_name)
{
  int command_index = -1;
  for (std::vector<Command>::iterator it = command_vector_.begin();
       it != command_vector_.end();
       ++it)
  {
      if (it->compareName(command_name))
      {
        command_index = std::distance(command_vector_.begin(),it);
        break;
      }
    }
  return command_index;
}

void DeviceInterface::printAllCommandNames()
{
  for (std::vector<Command>::iterator it = command_vector_.begin();
       it != command_vector_.end();
       ++it)
  {
    it->printName();
  }
}

void DeviceInterface::getDeviceInfoCallback()
{
  response["model_number"] = model_number_;
  response["serial_number"] = serial_number_;
  response["firmware_number"] = firmware_number_;
}

void DeviceInterface::getCommandsCallback()
{
  for (std::vector<Command>::iterator it = command_vector_.begin();
       it != command_vector_.end();
       ++it)
  {
    int command_index = std::distance(command_vector_.begin(),it);
    if (!command_vector_[command_index].isReserved())
    {
      char* command_name = it->getName();
      int command_id = command_index;
      response[command_name] = command_id;
    }
  }
}

void DeviceInterface::getResponseCodesCallback()
{
  response["rsp_success"] = SUCCESS;
  response["rsp_error"] = ERROR;
}

DeviceInterface device_interface(Serial);
