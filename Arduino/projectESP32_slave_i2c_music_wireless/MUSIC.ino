void music() {

if(millis() - last_timer > 100){
  last_timer = millis();

   rgb = data_[2]/2;
  if (rgb > 0) {
    strip.setPixelColor(0, strip.Color(0, 255, 0));
  }
  if (rgb > 1) {
    strip.setPixelColor(1, strip.Color(35, 255, 0));
  }
  if (rgb > 2) {
    strip.setPixelColor(2, strip.Color(70, 255, 0));
  }
  if (rgb > 3) {
    strip.setPixelColor(3, strip.Color(105, 255, 0));
  }
  if (rgb > 4) {
    strip.setPixelColor(4, strip.Color(140, 255, 0));
  }
  if (rgb > 5) {
    strip.setPixelColor(5, strip.Color(175, 255, 0));
  }
  if (rgb > 6) {
    strip.setPixelColor(6, strip.Color(210, 255, 0));
  }
  if (rgb > 7) {
    strip.setPixelColor(7, strip.Color(255, 255, 0));
  }
  if (rgb > 8) {
    strip.setPixelColor(8, strip.Color(255, 210, 0));
  }
  if (rgb > 9) {
    strip.setPixelColor(9, strip.Color(255, 175, 0));
  }
  if (rgb > 10) {
    strip.setPixelColor(10, strip.Color(255, 140, 0));
  }
  if (rgb > 11) {
    strip.setPixelColor(11, strip.Color(255, 70, 0));
  }
  if (rgb > 12) {
    strip.setPixelColor(12, strip.Color(255, 50, 0));
  }
  if (rgb > 13) {
    strip.setPixelColor(13, strip.Color(255, 20, 0));
  }
  if (rgb > 14) {
    strip.setPixelColor(14, strip.Color(255, 10, 0));
  }
  if (rgb > 15) {
    strip.setPixelColor(15, strip.Color(255, 0, 0));
  }


  if (rgb < 0) {
    strip.setPixelColor(0, strip.Color(0, 0, 0));
  }
  if (rgb < 1) {
    strip.setPixelColor(1, strip.Color(0, 0, 0));
  }
  if (rgb < 2) {
    strip.setPixelColor(2, strip.Color(0, 0, 0));
  }
  if (rgb < 3) {
    strip.setPixelColor(3, strip.Color(0, 0, 0));
  }
  if (rgb < 4) {
    strip.setPixelColor(4, strip.Color(0, 0, 0));
  }
  if (rgb < 5) {
    strip.setPixelColor(5, strip.Color(0, 0, 0));
  }
  if (rgb < 6) {
    strip.setPixelColor(6, strip.Color(0, 0, 0));
  }
  if (rgb < 7) {
    strip.setPixelColor(7, strip.Color(0, 0, 0));
  }
  if (rgb < 8) {
    strip.setPixelColor(8, strip.Color(0, 0, 0));
  }
  if (rgb < 9) {
    strip.setPixelColor(9, strip.Color(0, 0, 0));
  }
  if (rgb < 10) {
    strip.setPixelColor(10, strip.Color(0, 0, 0));
  }
  if (rgb < 11) {
    strip.setPixelColor(11, strip.Color(0, 0, 0));
  }
  if (rgb < 12) {
    strip.setPixelColor(12, strip.Color(0, 0, 0));
  }
  if (rgb < 13) {
    strip.setPixelColor(13, strip.Color(0, 0, 0));
  }
  if (rgb < 14) {
    strip.setPixelColor(14, strip.Color(0, 0, 0));
  }
    if (rgb < 15) {
    strip.setPixelColor(15, strip.Color(0, 0, 0));
  }

  strip.show();

}

 
}
