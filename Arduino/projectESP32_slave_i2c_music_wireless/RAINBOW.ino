void rainbow() {
  gain = 1;




  if (u == 0) {
    w = 0;
    x = 255;
    y = y + gain;
    if (y >= 255) {
      u = 1;
    }
  }

  if (u == 1) {
    w = 0;
    y = 255;
    x = x - gain;
    if (x <= 0) {
      u = 2;
    }
  }
  if (u == 2) {
    x = 0;
    y = 255;
    w = w + gain;
    if (w >= 255) {
      u = 3;
    }
  }

  if (u == 3) {
    x = 0;
    w = 255;
    y = y - gain;
    if (y <= 0) {
      u = 4;
    }
  }


  if (u == 4) {
    y = 0;
    w = 255;
    x = x + gain;
    if (x >= 255) {
      u = 5;
    }
  }

  if (u == 5) {
    y = 0;
    x = 255;
    w = w - gain;
    if (w <= 0) {
      u = 6;
    }
  }
  if (u == 6) {
    u = 0;
  }
  delay(10);
if(millis() - last_timer > 50){
  last_timer = millis();
    for ( int v = 0; v <= 15; v++) {
    strip.setPixelColor(v, strip.Color(w, x, y));
  }
    strip.show();
}



}
