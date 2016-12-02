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

int sendEncoder(byte* msg, byte* resp) {
  if (msg[0] == 0) {
    resp[0] = pos & 0xff;
    resp[1] = (pos >> 8) & 0xff;
    resp[2] = (pos >> 16) & 0xff;
    resp[3] = (pos >> 24) & 0xff;

    resp[4] = 0; // Mode

    for (int i = 5; i < 8; i++) {
      resp[i] = 0;
    }

    return 0;
  }
  else if (msg[0] == 1) {
    resp[0] = rate & 0xff;
    resp[1] = (rate >> 8) & 0xff;
    resp[2] = (rate >> 16) & 0xff;
    resp[3] = (rate >> 24) & 0xff;

    resp[4] = 1; // Mode

    for (int i = 5; i < 8; i++) {
      resp[i] = 0;
    }

    return 0;
  }
  else if (msg[0] == 0x72 && msg[1] == 0x65 && msg[2] == 0x73 && msg[3] == 0x65 && msg[4] == 0x74 && msg[5] == 0x65 && msg[6] == 0x6e && msg[7] == 0x63) {
    encoder.write(0);
    pos = 0;
    rate = 0;
    Serial.println("reset");
    return 1;
  }
  return 1;
}

int changeLEDs(byte* msg, byte* resp) {
  mode = msg[6] + (msg[7] << 8);
  value = msg[4] + (msg[5] << 8);
  R = msg[2];
  G = msg[1];
  B = msg[0];
  Serial.print(mode);
  Serial.print("\t");
  Serial.print(value);
  Serial.print("\t");
  Serial.print(R);
  Serial.print("\t");
  Serial.print(G);
  Serial.print("\t");
  Serial.print(B);
  Serial.println();

  return 0;
}

void setup(void) {
  CAN_add_id(0x601, &changeLEDs);
  CAN_add_id(0x611, &sendEncoder);
  CAN_begin();
  pixels.begin();
  delay(1000);
  Serial.println("Teensy 3.X CAN Encoder");
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
