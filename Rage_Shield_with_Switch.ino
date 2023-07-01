#include <FastLED.h>
#include <ezButton.h>


#define LED_PIN_MAIN     6   // Microcontroller Pin
#define LED_COUNT_MAIN  284   // Total LED count
#define LED_PIN_RING     5   // Microcontroller Pin
#define LED_COUNT_RING  36   // Total LED count
#define LED_PIN_SWITCH     7   // Microcontroller Pin

struct ledSegment {
  int start;
  int end;
  bool growing;
  int lower;
  int upper;
  int current;
  int intensity = 250;
  int number;
};

ezButton toggle_switch(LED_PIN_SWITCH); //State HIGH is off and LOW is on.

CRGB leds_main[LED_COUNT_MAIN];
CRGB leds_ring[LED_COUNT_RING];

  int hue = 0;       //Color of lights based on HSU
  int sat = 255;     //intensity of Lights, dim is controlled via visibility so unimportant for function
  int segments[] = {0, 25, 61, 99, 142, 185, 223, 259, 284}; //Start segments plus an extra for the final end   
  int mins[] = {5, 10, 12, 15, 15, 12, 10, 5}; //Base level of lights that will be static, current config gives 20 by 8 square
  int maxs[] = {0, 6, 6, 8, 8, 6, 6, 0};       //Upper level of lights that wont be used for animation
  ledSegment segment_data[8];
  int move_speed = 25; //Divides into 250
  int ring_speed = 10; //Divides into 200
  int ring_bright = 150;
  int ring_checkpoints[] = {0, 16, 28, 36}; //Starts and ends of each ring row
  int ring_current = 1;
  int switch_state = HIGH;
  bool switch_complete = false;

void setup() {
  // put your setup code here, to run once:
  toggle_switch.setDebounceTime(50);
  for (int i = 0; i < sizeof(segment_data)/sizeof(segment_data[0]); i++) {
      segment_data[i].start = segments[i];
      segment_data[i].end = segments[i+1];
      segment_data[i].lower = mins[i];
      segment_data[i].upper = maxs[i];
      segment_data[i].number = i;
      switch (i) {
      case 0:
      case 2:
      case 5:
      case 7:
          segment_data[i].current = 4;
          segment_data[i].growing = true;
          break;
      case 1:
      case 6:
          segment_data[i].current = 20;
          segment_data[i].growing = false;
          break;
      default:
          segment_data[i].current = 12;
          segment_data[i].growing = true;
          break;
      }
  }
  FastLED.addLeds<NEOPIXEL, LED_PIN_MAIN>(leds_main, LED_COUNT_MAIN);
  FastLED.addLeds<NEOPIXEL, LED_PIN_RING>(leds_ring, LED_COUNT_RING);
  FastLED.clear();
  InitialRingColor();
  for (int i = 0; i < sizeof(segment_data)/sizeof(segment_data[0]); i++) {
      InitialColorMain(segment_data[i]);
  }
}

void loop() {
 
  toggle_switch.loop();
  switch_state = toggle_switch.getState();
  if (switch_state == LOW && switch_complete == false) {
    SwitchMode();
    }
    else if (switch_state == LOW && switch_complete == true) {
      switch_state = toggle_switch.getState();
    }
    else if (switch_state == HIGH && switch_complete == true) {
      SwitchOff();
    }
    else {
  for (int i = 0; i < sizeof(segment_data)/sizeof(segment_data[0]); i++) {
      ColorChangeMain(i);
  }
  ColorChangeRing();
  FastLED.show();
  delay(1);
  }
}

void InitialColorMain (ledSegment segment) {
  for (int i = 0; i < segment.lower + segment.current; i++) {
    if (segment.number % 2 == 1) {
    leds_main[segment.start + i - 1] = CHSV(hue, 255, 250);
    }
  else {
    leds_main[segment.end - i] = CHSV(hue, 255, 250);
    }
  }
}

void ColorChangeMain (int i) {
  if (segment_data[i].growing) {
    if (segment_data[i].intensity == 250) {
      if (segment_data[i].current == 20) {
        segment_data[i].growing = false;
      }
      else {
        segment_data[i].current += 1;
        segment_data[i].intensity = 0;
      }
    }
    else {
      segment_data[i].intensity += move_speed;
    }
  }
  else {
    if (segment_data[i].intensity == 0) {
        if (segment_data[i].current == 1) {
          segment_data[i].growing = true;
        }
        else {
          segment_data[i].current -= 1;
          segment_data[i].intensity = 250;
        }
    }
    else {
        segment_data[i].intensity -= move_speed;
    }
  }
  if (segment_data[i].number % 2 == 1) {
    leds_main[segment_data[i].start + segment_data[i].lower + segment_data[i].current - 1] = CHSV(hue, 255, segment_data[i].intensity);
    }
  else {
    leds_main[segment_data[i].end - segment_data[i].lower - segment_data[i].current] = CHSV(hue, 255, segment_data[i].intensity);
    }
  
}

void InitialRingColor () {
  leds_ring[0] = CHSV(hue, 255, 250);
  leds_ring[1] = CHSV(hue, 255, 150);
  leds_ring[ring_checkpoints[1]-1] = CHSV(hue, 255, 150);
}

void ColorChangeRing () {
  if (ring_bright == 240) {
    ring_bright = 120;
    ring_current = (ring_current + 1) % 16;
  }
  switch (ring_current) {
    case 0:
    leds_ring[ring_current] = CHSV(hue, 255, ring_bright);
    leds_ring[1] = CHSV(hue, 255, ring_bright - 120);
    leds_ring[15] = CHSV(hue, 255, 360 - ring_bright);
    leds_ring[14] = CHSV(hue, 255, 240 - ring_bright);
    break;
    case 1:
    leds_ring[ring_current] = CHSV(hue, 255, ring_bright);
    leds_ring[2] = CHSV(hue, 255, ring_bright - 120);
    leds_ring[0] = CHSV(hue, 255, 360 - ring_bright);
    leds_ring[15] = CHSV(hue, 255, 240 - ring_bright);
    break;
    case 15:
    leds_ring[ring_current] = CHSV(hue, 255, ring_bright);
    leds_ring[0] = CHSV(hue, 255, ring_bright - 120);
    leds_ring[14] = CHSV(hue, 255, 360 - ring_bright);
    leds_ring[13] = CHSV(hue, 255, 240 - ring_bright);
    break;
    default:
    leds_ring[ring_current] = CHSV(hue, 255, ring_bright);
    leds_ring[(ring_current + 1) % 16] = CHSV(hue, 255, ring_bright - 120);
    leds_ring[(ring_current - 1) % 16] = CHSV(hue, 255, 360 - ring_bright);
    leds_ring[(ring_current - 2) % 16] = CHSV(hue, 255, 240 - ring_bright);
    break;
  }
  ring_bright += ring_speed;
}

void SwitchMode () {

  int segment_number = sizeof(segment_data)/sizeof(segment_data[0]);

  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < LED_COUNT_MAIN; j++) {
        leds_main[j] = CHSV(hue, 255, i*25);
        
    }
    for (int j = 0; j < LED_COUNT_RING; j++) {
        leds_ring[j] = CHSV(hue, 255, i*25);
    }
    FastLED.show();
    delay(10);
  }
  switch_complete = true;
}

void SwitchOff () {

  int segment_number = sizeof(segment_data)/sizeof(segment_data[0]);

  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < LED_COUNT_MAIN; j++) {
        leds_main[j] = CHSV(hue, 255, 250 - i*25);
        
    }
    for (int j = 0; j < LED_COUNT_RING; j++) {
        leds_ring[j] = CHSV(hue, 255, 250 - i*25);
    }
    FastLED.show();
  }
  setup();
  switch_complete = false;
}
