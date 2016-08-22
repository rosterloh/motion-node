#include "mbed.h"
#include <RFM69.h>

#define GATEWAY_ID    1
#define NODE_ID       9
#define NETWORKID     101

#define FREQUENCY     RF69_433MHZ
#define IS_RFM69HW
#define ENCRYPT_KEY   "EncryptKey123456"  // use same 16byte encryption key for all devices on net
#define ACK_TIME      50                  // max msec for ACK wait
#define VERSION       "1.0.0"

#define RFM69_MOSI    PB10
#define RFM69_MISO    PA12
#define RFM69_SCK     PB11
#define RFM69_CS      PA06    // D8
#define RFM69_IRQ     PA09    // D3
#define RFM69_RST     PA05    // D4
#define LED           PA17    // D13

#define MSGBUFSIZE    64  // message buffersize, but for this demo we only use:
                          // 1-byte NODEID + 4-bytes for time + 1-byte for temp in C + 2-bytes for vcc(mV)
char msgBuf[MSGBUFSIZE];

Serial pc(PB22, PB23);
DigitalOut led(LED);
DigitalOut rst(RFM69_RST);
RFM69 radio(RFM69_MOSI, RFM69_MISO, RFM69_SCK, RFM69_CS, RFM69_IRQ);
Timer tmr;

int main() {
  memset(msgBuf, 0, sizeof(msgBuf));
  int i=1;
  long l;
  tmr.start();

  pc.printf("Motion Node %s...\r\n", VERSION);

  // Hard Reset the RFM module
  rst = 1;
  Thread::wait(100);
  rst = 0;
  Thread::wait(100);

  radio.initialize(FREQUENCY, NODE_ID, NETWORKID);
  radio.setHighPower();
  radio.setPowerLevel(20);     // power output ranges from 0 (5dBm) to 31 (20dBm)
  radio.encrypt(ENCRYPT_KEY);
  radio.promiscuous(false);
  msgBuf[0] = (uint8_t)NODE_ID;

  while (true) {
    uint8_t tempC =  radio.readTemperature(-1); // -1 = user cal factor, adjust for correct ambient
    uint8_t tempF = 1.8 * tempC + 32; // 9/5=1.8

    l = tmr.read_ms();  // load time

    sprintf((char*)msgBuf, "#%d, t=%Lu, temp=%dF, RSSI=%d ", i, l, tempF, radio.RSSI);
    if(radio.sendWithRetry((uint8_t) GATEWAY_ID, msgBuf, strlen(msgBuf), true))
      pc.printf("Packet %d sent, Ack ok!\r\n",i++);
    else pc.printf("Packet %d sent, no Ack!\r\n",i++);
    Thread::wait(1000);
    led = !led;
  }
}
