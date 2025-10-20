# Olimex ESP sensors
SNMP sensors with Olimex ESP32-POE-ISO

# Target of the project
Provide a cost effective platform for temperature/hunidity readings that can be expanded with other sensors, if needed.

The minimum requirements are: PoE power, SNMP v2c

# Hardware
## Board
After experimenting with some chinese boards from AliExpress, I decided to use Olimex boards. Olimex is a Bulgarian well established company with excellent support and documentation.

The board of this project is [ESP32-POE-ISO-16MB](https://www.olimex.com/Products/IoT/ESP32/ESP32-POE-ISO/open-source-hardware)

The ISO version has a 3000VDC galvanic insulation from the Ethernet powering, allowing the PoE ethernet and the programming microUSB  to be connected at the same time.

## Sensor
I use the well-known DHT22, reliable for the purpose of this project.


