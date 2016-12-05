#include <Encoder.h>
#include <Adafruit_NeoPixel.h>
#include <FlexCAN.h>
#include <TeensyCANBase.h>

Encoder wheelEncoder(5, 6);
Encoder turretEncoder(9, 10);

const int numberPixels = 34;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(numberPixels, 14, NEO_GRB + NEO_KHZ800);

long wheelLastRead = 0;
long wheelPos = -999;
long wheelRate = 0;

long turretLastRead = 0;
long turretPos = -999;
long turretRate = 0;

int mode = 0;
int value = 0;
int R = 0;
int G = 0;
int B = 0;
int led_idx = 0;

int sendWheelEncoder(byte* msg, byte* resp) {
  if (msg[0] == 0) {
    resp[0] = wheelPos & 0xff;
    resp[1] = (wheelPos >> 8) & 0xff;
    resp[2] = (wheelPos >> 16) & 0xff;
    resp[3] = (wheelPos >> 24) & 0xff;

    Serial.println(wheelPos);

    resp[4] = 0; // Mode

    for (int i = 5; i < 8; i++) {
      resp[i] = 0;
    }

    return 0;
  }
  else if (msg[0] == 1) {
    resp[0] = wheelRate & 0xff;
    resp[1] = (wheelRate >> 8) & 0xff;
    resp[2] = (wheelRate >> 16) & 0xff;
    resp[3] = (wheelRate >> 24) & 0xff;

    resp[4] = 1; // Mode

    for (int i = 5; i < 8; i++) {
      resp[i] = 0;
    }

    return 0;
  }
  else if (msg[0] == 0x72 && msg[1] == 0x65 && msg[2] == 0x73 && msg[3] == 0x65 && msg[4] == 0x74 && msg[5] == 0x65 && msg[6] == 0x6e && msg[7] == 0x63) {
    wheelEncoder.write(0);
    wheelPos = 0;
    wheelRate = 0;
    Serial.println("reset");
    return 1;
  }
  return 1;
}

int sendTurretEncoder(byte* msg, byte* resp) {
  if (msg[0] == 0) {
    resp[0] = turretPos & 0xff;
    resp[1] = (turretPos >> 8) & 0xff;
    resp[2] = (turretPos >> 16) & 0xff;
    resp[3] = (turretPos >> 24) & 0xff;

    resp[4] = 0; // Mode

    for (int i = 5; i < 8; i++) {
      resp[i] = 0;
    }

    return 0;
  }
  else if (msg[0] == 1) {
    resp[0] = turretRate & 0xff;
    resp[1] = (turretRate >> 8) & 0xff;
    resp[2] = (turretRate >> 16) & 0xff;
    resp[3] = (turretRate >> 24) & 0xff;

    resp[4] = 1; // Mode

    for (int i = 5; i < 8; i++) {
      resp[i] = 0;
    }

    return 0;
  }
  else if (msg[0] == 0x72 && msg[1] == 0x65 && msg[2] == 0x73 && msg[3] == 0x65 && msg[4] == 0x74 && msg[5] == 0x65 && msg[6] == 0x6e && msg[7] == 0x63) {
    turretEncoder.write(0);
    turretPos = 0;
    turretRate = 0;
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
  CAN_add_id(0x602, &changeLEDs);
  CAN_add_id(0x606, &sendTurretEncoder);
  CAN_add_id(0x612, &sendWheelEncoder);
  CAN_begin();
  pixels.begin();
  delay(1000);
  Serial.println("Teensy 3.X CAN Encoder");
}

void loop(void) {
  CAN_update();

  update_pixels();
  pixels.show();

  long newPos = wheelEncoder.read();
  if (newPos != wheelPos) {
    wheelRate = ((double) 1000000.0 * (newPos - wheelPos)) / ((double) (micros() - wheelLastRead));
    wheelPos = newPos;
    wheelLastRead = micros();
  }
  else {
    if ((micros() - wheelLastRead) > 1000) {
      wheelRate = 0;
    }
  }

  newPos = turretEncoder.read();
  if (newPos != turretPos) {
    turretRate = ((double) 1000000.0 * (newPos - turretPos)) / ((double) (micros() - turretLastRead));
    turretPos = newPos;
    turretLastRead = micros();
  }
  else {
    if ((micros() - turretLastRead) > 1000) {
      turretRate = 0;
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
