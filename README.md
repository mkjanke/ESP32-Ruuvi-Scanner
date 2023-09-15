# ESP32-Ruuvi-Scanner
  Simple ESP32 Ruvui temperature/humidity decoder for v5 devices

  Uses Arduino framework, NimBLE library. Tested on ESP32.

  See:  
  
    * [https://github.com/ruuvi/ruuvi-sensor-protocols/blob/master/dataformat_05.md]
    * [https://mybeacons.info/packetFormats.html#hiresX]
    * [https://github.com/PascalBod/ESPIDFRuuviTag/blob/master/main/ruuvi_tag.c]

  Listens for Ruuvi Service UUID, mfg. ID 0x0499 and mfg. data version 0x05.
  
  Ignores all other advertisments

