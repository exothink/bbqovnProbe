/*
C:\Users\EliteDesk\Documents\PlatformIO\Projects\bbqovnProbe\src\main.cpp      je 9/14/24

reads BBQOVN probe temperature via ble notify. Don't let probe connect to BLE base!
works well
***WOW*** probe has been running stand alone for over 24hrs on a single charge   9/15

RAM:   [=         ]   9.7% (used 31860 bytes from 327680 bytes)  board = esp32doit-devkit-v1
Flash: [====      ]  40.9% (used 535537 bytes from 1310720 bytes)

RAM:   [=         ]   7.9% (used 25744 bytes from 327680 bytes)  board = 4d_systems_esp32s3_gen4_r8n16
Flash: [=         ]   6.5% (used 427597 bytes from 6553600 bytes)
 */

#include <ArduinoBLE.h>

#define DEV_ADR "33:34:50:00:9c:59"                           // BBQOVN probe adr
#define TEMP_CHAR_UUID "4c0a0ff9-1c6d-1c49-e07f-10f1c55905ca" // 2901"
#define SERVICE "8800c4fb-a175-b666-2ebb-1e97e6fa47b3"

#define Serial Serial0 // ***If board is: esp32-s3-devkitc-1-n16r8v***

#define VERS "bbqovnProbe"
BLEDevice peripheral;
BLECharacteristic tempCharacteristic;
long count = 0;

void printTemperature(BLECharacteristic characteristic)
{
  Serial.print(F("Temp: "));
  // const uint8_t *value = characteristic.value();
  float val = 100 * (characteristic.value()[2] - 0x30) + 10 * (characteristic.value()[3] - 0x30) + characteristic.value()[4] - 0x30;
  val = val / 10.0;
  float f = 9 / 5.0 * val + 32;
  Serial.print(val, 1);
  Serial.print(F("C   "));
  Serial.print(f, 1);
  Serial.print(F("F\t"));

  int secs = millis() / 1000; // to hr:min:sec
  int hrs = secs / 3600;
  secs = secs % 3600;
  int mins = secs / 60;
  secs = secs % 60;

  count++;

  char buf[21];
  sprintf(buf, "%02d:%02d:%02d\t%d", hrs, mins, secs, count);
  Serial.println(buf);
}

void setup()
{
  Serial.begin(115200);
  BLE.begin();
  // Serial.println(VERS);
  Serial.println((String) "\n\n" + __TIMESTAMP__ + "  " + VERS);

  // if (!BLE.begin()) // setup hdwr.  Start GATT.
  // {
  //   Serial.println("Starting BLE failed!");
  //   while (1)
  //     ;
  // }
  Serial.println(F("\nBBQOVN Probe Client Starting"));

  // Start scanning for BLE peripherals

  // BLE.scanForUuid("4c0a0ff9-1c6d-1c49-e07f-10f1c55905ca"); // no
  // BLE.setAdvertisedServiceUuid(SERVICE); //no
  // BLE.scanForUuid(SERVICE); //no
  // BLE.scanForAddress(DEV_ADR); //yes
  if (BLE.scanForUuid("180a"))
    ; // yes
  Serial.println(F("Found Uuid"));

  Serial.println(F("Looking for temp probe"));
  long time = millis();
  do
  {
    peripheral = BLE.available(); // Wait for temp probe
    if ((millis() - time) > 5000)
    {
      Serial.println(F("Timed Out"));
      break;
    }
  } while (!peripheral);

  // if (peripheral.connect())
  //  if (BLE.connected(peripheral))
  //  if (peripheral.address() == DEV_ADR)
  //  if (peripheral.connect())
  // Serial.println(peripheral.address());
  if (peripheral.address() == DEV_ADR)
  {
    Serial.println(F("Found temp probe"));
    BLE.stopScan(); // found. Stop Scan.
    if (peripheral.connect())
    {
      Serial.println(F("Connected to temp probe"));
    }
    else
    {
      Serial.println(F("Can't connect to temp probe"));
    }

    if (peripheral.discoverAttributes())
    {
      Serial.println(F("Discovering attributes ..."));
      tempCharacteristic = peripheral.characteristic(TEMP_CHAR_UUID); // get temp notify char
      Serial.println(F("Get the characteristic"));
      if (tempCharacteristic)
      {
        tempCharacteristic.subscribe(); // Subscribe to the notify characteristic
        Serial.println(F("subscribed to peripheral"));
      }
    }
  }
  else
    Serial.println(F("Couldn't connect to temp probe. May be connected elsewhere."));
}

void loop()
{
  while (peripheral.connected()) // Check if new data is available
  {
    if (tempCharacteristic.valueUpdated()) // notify characteristic only sends vals that have changed
      printTemperature(tempCharacteristic);
  }
  BLE.disconnect();
  Serial.println(F("Not Connected"));
  while (1)
    ; // halt. done.
}
