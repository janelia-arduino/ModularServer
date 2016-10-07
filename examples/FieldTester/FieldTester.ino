#include "Flash.h"
#include <EEPROM.h>
#include "Streaming.h"
#include "ArduinoJson.h"
#include "JsonSanitizer.h"
#include "JsonStream.h"
#include "Array.h"
#include "Vector.h"
#include "MemoryFree.h"
#include "ConstantVariable.h"
#include "SavedVariable.h"
#include "Functor.h"

#include "Constants.h"

#include "ModularServer.h"
#include "FieldTester.h"


FieldTester dev;

void setup()
{
  dev.setup();
}

void loop()
{
  dev.update();
}
