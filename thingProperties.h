// Code generated by Arduino IoT Cloud, DO NOT EDIT.

#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

const char SSID[]     = SECRET_SSID;    // Network SSID (name)
const char PASS[]     = SECRET_OPTIONAL_PASS;    // Network password (use for WPA, or use as key for WEP)


String nadvCloud;
float humedadSueloCloud;
float lvlpHCloud;
float solarCloud;
float temperaturaCloud;
float vpdCloud;

void initProperties(){

  ArduinoCloud.addProperty(nadvCloud, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(humedadSueloCloud, READ, 5 * SECONDS, NULL);
  ArduinoCloud.addProperty(lvlpHCloud, READ, 5 * SECONDS, NULL);
  ArduinoCloud.addProperty(solarCloud, READ, 5 * SECONDS, NULL);
  ArduinoCloud.addProperty(temperaturaCloud, READ, 5 * SECONDS, NULL);
  ArduinoCloud.addProperty(vpdCloud, READ, 5 * SECONDS, NULL);

}

WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);
