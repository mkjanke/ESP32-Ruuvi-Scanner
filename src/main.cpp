/*----------------------------------------------------------------
  Simple Ruuvi temperature/humidity decoder for v5 devices

  Uses Arduino framework, NimBLE library. Tested on ESP32.

  See:  https://github.com/ruuvi/ruuvi-sensor-protocols/blob/master/dataformat_05.md
        https://mybeacons.info/packetFormats.html#hiresX
        https://github.com/PascalBod/ESPIDFRuuviTag/blob/master/main/ruuvi_tag.c

  Listens for Ruuvi Service UUID, mfg. ID 0x0499 and mfg. data version 0x05. Ignores all other advertisments
*/

#include <Arduino.h>

#include "NimBLEDevice.h"
NimBLEScan* pBLEScan;

// Ruuvi v5 serivice UUID
NimBLEUUID serviceUuid("6e400001-b5a3-f393-e0a9-e50e24dcca9e");

class MyAdvertisedDeviceCallbacks : public NimBLEAdvertisedDeviceCallbacks {
  void onResult(NimBLEAdvertisedDevice* advertisedDevice) {
    if (advertisedDevice->getServiceUUID() == serviceUuid) {
      // Serial.printf("Ruuvi Device: %s \n", advertisedDevice->toString().c_str());

      // Look for Ruuvi mfg. ID
      if (((byte)advertisedDevice->getManufacturerData().data()[0] == 0x99) &&
          ((byte)advertisedDevice->getManufacturerData().data()[1] == 0x04)) {
        std::string output = advertisedDevice->getName() + " " + advertisedDevice->getAddress().toString() + " ";
        Serial.println(output.c_str());

        char* manufacturerdata =
            NimBLEUtils::buildHexData(NULL, (uint8_t*)advertisedDevice->getManufacturerData().data(),
                                      advertisedDevice->getManufacturerData().length());
        Serial.println(manufacturerdata);
        if (manufacturerdata != NULL) {
          free(manufacturerdata);
        }
        uint8_t* MFRdata;
        float tempInC;
        float humPct = 0;
        float atmPressure = 0;

        MFRdata = (uint8_t*)advertisedDevice->getManufacturerData().data();

        // Version 5 data format
        // https://mybeacons.info/packetFormats.html#hiresX
        //
        if (MFRdata[2] == 0x05) {
          if (!(MFRdata[3] == 0x80 && MFRdata[4] == 0x00)) {
            tempInC = ((float)((int16_t)((MFRdata[4] << 0) | (MFRdata[3]) << 8)) * .005);
          }
          if (!(MFRdata[6] == 0xff && MFRdata[5] == 0xff)) {
            humPct = ((float)((uint16_t)((MFRdata[6] << 0) | (MFRdata[5]) << 8))) / 400;
          }
          if (!(MFRdata[8] == 0xff && MFRdata[7] == 0xff)) {
            atmPressure = ((float)((uint16_t)((MFRdata[8] << 0) | (MFRdata[7]) << 8)) + 50000);
          }
          Serial.printf("Temperature (C): %4.1f Humidity(%): %4.1f Atm Pressure: %5.0f\n", tempInC, humPct,
                        atmPressure);
        }
      }

    } else {
      pBLEScan->erase(advertisedDevice->getAddress());
    }
  }
};
void setup() {
  Serial.begin(115200);

  NimBLEDevice::init("");
  pBLEScan = NimBLEDevice::getScan();  // create new scan

  // Set the callback for when devices are discovered, include duplicates.
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(), true);
  pBLEScan->setActiveScan(true);  // Set active scanning, this will get more data from the advertiser.

  // Values taken from NimBLE examples
  pBLEScan->setInterval(97);   // How often the scan occurs / switches channels; in milliseconds,
  pBLEScan->setWindow(37);     // How long to scan during the interval; in milliseconds.
  pBLEScan->setMaxResults(0);  // do not store the scan results, use callback only.
}

void loop() {
  // If an error occurs that stops the scan, it will be restarted here.
  if (pBLEScan->isScanning() == false) {
    // Start scan with: duration = 0 seconds(forever), no scan end callback, not a continuation of a previous scan.
    pBLEScan->start(0, nullptr, false);
  }
  // Free memory from unused devices?
  if (pBLEScan->getResults().getCount() > 10) {
    pBLEScan->stop();
  }
  delay(2000);
}
