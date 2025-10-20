//
// Olimex ESP32 SNP sensors
// 
// 20251020 by Luigi Rosa lists@luigirosa.com
// 

#include <ETH.h>
#include <WiFiUdp.h>
#include <SNMP_Agent.h>
#include <DHT.h>

// DHT22 Configuration
#define DHTPIN 33
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Ethernet Configuration
// Set to true for DHCP, false for static IP
#define USE_DHCP false

// Static IP Configuration (only used if USE_DHCP is false)
IPAddress staticIP(10, 19, 67, 47);
IPAddress gateway(10, 19, 67, 254);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns1(8, 8, 8, 8);
IPAddress dns2(8, 8, 4, 4);

// SNMP Configuration
WiFiUDP udp;
SNMPAgent snmp = SNMPAgent("public");  // Read-only community string

char* OID_SYS_DESCR = ".1.3.6.1.2.1.1.1.0";
char* OID_SYS_UPTIME = ".1.3.6.1.2.1.1.3.0";
char* OID_SYS_ADMIN = ".1.3.6.1.2.1.1.4.0";
char* OID_SYS_NAME = ".1.3.6.1.2.1.1.5.0";
char* OID_SYS_LOCATION = ".1.3.6.1.2.1.1.6.0";
char* OID_TEMP_INDEX = ".1.3.6.1.4.1.2021.13.16.2.1.1.1";      // lmTempSensorsIndex
char* OID_TEMP_DEVICE = ".1.3.6.1.4.1.2021.13.16.2.1.2.1";     // lmTempSensorsDevice (name)
char* OID_TEMP_VALUE =  ".1.3.6.1.4.1.2021.13.16.2.1.3.1";      // lmTempSensorsValue (temp in mC)
char* OID_HUM_INDEX = ".1.3.6.1.4.1.2021.13.16.5.1.1.1";      // humidity index
char* OID_HUM_DEVICE = ".1.3.6.1.4.1.2021.13.16.5.1.2.1";     // humidity desc
char* OID_HUM_VALUE =  ".1.3.6.1.4.1.2021.13.16.5.1.3.1";      // humidity value

// Sensor data variables
int temperature_index = 1;
std::string temperature_description = "Temperature";
int humidity_index = 1;
std::string humidity_description = "% Relative Humidity";
uint32_t temperature = 0;
uint32_t humidity = 0;
uint32_t sysUptime = 0;
std::string sysDescr = "Arduino IDE Olimex ESP32-POE-ISO-16MB WROOM-32UE SNMP Sensor Station v1.0";
std::string sysName = "lr-esp32-snmp-sensor";
std::string sysAdmin = "Luigi Rosa";
std::string sysLocation = "Sotto il tavolo";

unsigned long lastRead = 0;
const unsigned long READ_INTERVAL = 5000; // Read sensor every 5 seconds

// Ethernet event handler
void ethEvent(arduino_event_id_t event) {
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      Serial.println("ETH Started");
      ETH.setHostname("esp32-snmp-sensor");
      break;
    case ARDUINO_EVENT_ETH_CONNECTED:
      Serial.println("ETH Connected");
      break;
    case ARDUINO_EVENT_ETH_GOT_IP:
      Serial.print("ETH MAC: ");
      Serial.print(ETH.macAddress());
      Serial.print(", IPv4: ");
      Serial.print(ETH.localIP());
      Serial.print(", Speed: ");
      Serial.print(ETH.linkSpeed());
      Serial.println("Mbps");
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      break;
    case ARDUINO_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      break;
    default:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\nESP32-POE-ISO SNMP Server with DHT22");
  
  // Initialize DHT22
  dht.begin();
  Serial.println("DHT22 initialized.");
  
  // Initialize Ethernet with event handler
  Network.onEvent(ethEvent);
  ETH.begin();
  
  // Configure IP
  if (USE_DHCP) {
    Serial.println("Using DHCP...");
  } else {
    Serial.println("Using Static IP...");
    if (!ETH.config(staticIP, gateway, subnet, dns1, dns2)) {
      Serial.println("Static IP configuration failed!");
    }
  }
  
  // Wait for Ethernet connection
  Serial.print("Connecting to Ethernet");
  while (!ETH.linkUp()) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nEthernet connected!");
  
  // Display network configuration
  Serial.println("\nNetwork Configuration:");
  Serial.print("IP Address: ");
  Serial.println(ETH.localIP());
  Serial.print("Subnet Mask: ");
  Serial.println(ETH.subnetMask());
  Serial.print("Gateway: ");
  Serial.println(ETH.gatewayIP());
  Serial.print("DNS: ");
  Serial.println(ETH.dnsIP());
  
  // Initialize SNMP Agent
  snmp.setUDP(&udp);
  snmp.begin();
  
  // Add OID handlers - integers for sensor values
  snmp.addIntegerHandler(OID_TEMP_INDEX, &temperature_index);
  snmp.addReadOnlyStaticStringHandler(OID_TEMP_DEVICE, temperature_description);
  snmp.addGaugeHandler(OID_TEMP_VALUE, &temperature);
  snmp.addIntegerHandler(OID_HUM_INDEX, &humidity_index);
  snmp.addReadOnlyStaticStringHandler(OID_HUM_DEVICE, humidity_description);
  snmp.addGaugeHandler(OID_HUM_VALUE, &humidity);
  
  // Add OID handlers - strings for system info
  snmp.addReadOnlyStaticStringHandler(OID_SYS_DESCR, sysDescr);
  snmp.addReadOnlyStaticStringHandler(OID_SYS_NAME, sysName);
  snmp.addReadOnlyStaticStringHandler(OID_SYS_ADMIN, sysAdmin);
  snmp.addReadOnlyStaticStringHandler(OID_SYS_LOCATION, sysLocation);
  
  // Add timestamp handler for uptime
  snmp.addTimestampHandler(OID_SYS_UPTIME, &sysUptime);
  
  // Sort handlers for snmpwalk to work properly
  snmp.sortHandlers();
  
  Serial.println("\nSNMP server started on port 161");
  Serial.println("\nOID Mappings:");
  Serial.print("Temperature (C*1000): ");
  Serial.println(OID_TEMP_VALUE);
  Serial.print("Humidity (%*10): ");
  Serial.println(OID_HUM_VALUE);
  Serial.print("System Description: ");
  Serial.println(OID_SYS_DESCR);
  Serial.print("System Uptime: ");
  Serial.println(OID_SYS_UPTIME);
  Serial.print("System Name: ");
  Serial.println(OID_SYS_NAME);
  Serial.println("\nReading sensor data...\n");
}

void loop() {
  // Process SNMP requests
  snmp.loop();
  
  // Update system uptime (in hundredths of a second)
  sysUptime = (uint32_t)(millis() / 10);
  
  // Read DHT22 sensor periodically
  if (millis() - lastRead >= READ_INTERVAL) {
    lastRead = millis();
    
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    
    // Check if readings are valid
    if (!isnan(h) && !isnan(t)) {
      temperature = (uint32_t)(t * 1000);
      humidity = (uint32_t)(h * 10);
      
      Serial.printf("Sensor Update - Temp: %.1f°C, Humidity: %.1f%%\n", t, h);
    } else {
      Serial.println("Failed to read from DHT22 sensor!");
    }
  }
}
