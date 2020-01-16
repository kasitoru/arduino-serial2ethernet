/*
  Arduino Serial To Ethernet Converter
  Author: Sergey Avdeev <avdeevsv91@gmail.com>
  URL: https://github.com/avdeevsv91/arduino-serial2ethernet
*/

#define MACADDRESS 0x00,0x01,0x02,0x03,0x04,0x05
#define TCPPORT 23
#define MAXCLIENTS 3
#define BAUDRATE 2400

#include <UIPEthernet.h>

uint8_t MAC[6] = {MACADDRESS};
EthernetServer server = EthernetServer(TCPPORT);
EthernetClient clients[MAXCLIENTS];

void setup() {
  Serial.begin(BAUDRATE);
  Ethernet.begin(MAC);
  server.begin();
}

void loop() {
  size_t size;
  // Ethernet -> Serial
  if(EthernetClient client = server.available()) {
    // New TCP client
  	uint8_t count = 0;
    bool is_new = true;
  	for(uint8_t i=0; i<MAXCLIENTS; i++) {
      if(clients[i]==client && is_new) {
        is_new = false;
      }
      if(clients[i]) count++;
    }
    // New TCP client
    if(is_new) {
      if(count<MAXCLIENTS) {
        for(uint8_t i=0; i<MAXCLIENTS; i++) {
          if(!clients[i]) {
            clients[i] = client;
            client.flush();
            break;
          }
        }
      } else {
        client.println(F("Too many connections!"));
        client.stop();
      }
    }
  	// Data
    while((size = client.available()) > 0) {
      uint8_t* message = (uint8_t*)malloc(size);
      size = client.read(message, size);
      Serial.write(message, size);
      free(message);
    }
  }
  // Serial -> Ethernet
  while((size = Serial.available()) > 0) {
    uint8_t* message = (uint8_t*)malloc(size);
    size = Serial.readBytes(message, size);
    for(uint8_t i=0; i<MAXCLIENTS; i++) {
      if(clients[i]) {
        clients[i].write(message, size);
      }
    }
    free(message);
  }
  // Offline clients
  for(uint8_t i=0; i<MAXCLIENTS; i++) {
    if(!(clients[i].connected())) {
      clients[i].stop();
    }
  }
}
