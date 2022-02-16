
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel strip = Adafruit_NeoPixel(15, 6, NEO_GRB + NEO_KHZ800);
//music
double rgb = 0;
double r1 = 0, g1 = 255, b1 = 0;
double r2 = 255, g2 = 255, b2 = 0;
double r3 = 255, g3 = 0, b3 = 0;
long last_run = 0;
uint16_t pixel = 0, last_pixel = 0;
//rainbow
float gain = 0, u = 0, w = 0, x = 0, y = 0;
///////
uint8_t last_Brightness = 0;
int data_[4] = {0};
long timer = 0, last_timer = 0;
void setup() {
  strip.begin();
  strip.setBrightness(20);
  strip.show();
  Wire.begin(0x04);                // join i2c bus with address #8
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(115200);           // start serial for output
}

void loop() {
  if (data_[1] != last_Brightness) {
    last_Brightness = data_[1];
    strip.setBrightness(last_Brightness);
    strip.show();

  }

    if (data_[0] == 1) {
      rainbow();
    }
    if (data_[0] == 0) {
      music();
    }

}

void receiveEvent(int howMany)
{
  while (1 < Wire.available()) // loop through all but the last
  {
    for (int i = 0; i <= 2; i++) {
      data_[i] = Wire.read();    // receive byte as an integer
      delay(100);

    }
    // print the character
  }
  Serial.print("button switch: ");
  Serial.print(data_[0]);
  Serial.print("    :");
  Serial.print("Brightness :");
  Serial.print(data_[1]);
  Serial.print("    :");
  Serial.print("music signal :");
  Serial.println(data_[2]);


}
