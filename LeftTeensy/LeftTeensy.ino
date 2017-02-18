#include <Encoder.h>
#include <Adafruit_NeoPixel.h>
#include <FlexCAN.h>
#include <TeensyCANBase.h>

Encoder encoder(5, 6);

const int numberPixels = 34;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(numberPixels, 14, NEO_GRB + NEO_KHZ800);

long lastRead = 0;
long pos = -999;
long rate = 0;

int mode = 0;
int value = 0;
int R = 0;
int G = 0;
int B = 0;
int led_idx = 0;

void resetEncoder(byte* msg) {
  if (msg[0] == 0x72 && msg[1] == 0x65 && msg[2] == 0x73 && msg[3] == 0x65 && msg[4] == 0x74 && msg[5] == 0x65 && msg[6] == 0x6e && msg[7] == 0x63) {
    encoder.write(0);
    pos = 0;
    rate = 0;
    Serial.println("reset");
  }
}

void changeLEDs(byte* msg) {
  mode = msg[6] + (msg[7] << 8);
  value = msg[4] + (msg[5] << 8);
  R = msg[2];
  G = msg[1];
  B = msg[0];
}

void setup(void) {
  CAN_add_id(0x601, &changeLEDs);
  CAN_add_id(0x611, &resetEncoder);
  CAN_begin();
  pixels.begin();
  delay(1000);
  Serial.println("Teensy 3.X CAN Encoder");
}

void writeLongs(uint32_t id, long value1, long value2){
  byte * msg = new byte[8];

  for(int i = 0; i < 4; i++){
    msg[i] = (value1 >> i*8) & 0xFF;
  }
  for(int i = 0; i < 4; i++){
    msg[i + 4] = (value2 >> i*8) & 0xFF;
  }
  
  CAN_write(id, msg);

  delete msg;
}

void loop(void) {
  CAN_update();

  update_pixels();
  pixels.show();

  long newPos = encoder.read();
  if (newPos != pos) {
    rate = ((double) 1000000.0 * (newPos - pos)) / ((double) (micros() - lastRead));
    pos = newPos;
    lastRead = micros();
  }
  else {
    if ((micros() - lastRead) > 1000) {
      rate = 0;
    }
  }

  writeLongs(0x611, pos, rate); // Position
  
  delay(10);
}

void update_pixels(){
  if(mode == 0){
    for(int i = 0; i < numberPixels; i++){
      pixels.setPixelColor(i, pixels.Color(128, 0, 0));
    }
  }
  else{
    led_idx += value;
    for(int i = 0; i < pixels.numPixels(); i++) {
      pixels.setPixelColor(i, Wheel((i*8 + led_idx / 4096) & 0xFF));
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
// From NeoPixel demo code
uint32_t Wheel(byte WheelPos) {
  Serial.println(WheelPos);
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
