#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>

#define F_CPU 16000000
#include <util/delay.h>
#include "usbdrv/usbdrv.h"

static uchar reportBuffer[2];
static uchar idleRate;

const PROGMEM char usbHidReportDescriptor[35] = {   /* USB report descriptor */
  0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
  0x09, 0x06,                    // USAGE (Keyboard)
  0xa1, 0x01,                    // COLLECTION (Application)
  0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
  0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)
  0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)
  0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
  0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
  0x75, 0x01,                    //   REPORT_SIZE (1)
  0x95, 0x08,                    //   REPORT_COUNT (8)
  0x81, 0x02,                    //   INPUT (Data,Var,Abs)
  0x95, 0x01,                    //   REPORT_COUNT (1)
  0x75, 0x08,                    //   REPORT_SIZE (8)
  0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
  0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))
  0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)
  0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
  0xc0                           // END_COLLECTION
};

static const uchar  keyReport[4][2] PROGMEM = {
  {0, 0},                     /* no key pressed */
  {0, 4},
  {1<<1, 0},
  {1<<1, 4},
};

static uchar  keyPressed(int shiftstate) {
  if (!(PINB & 1<<PB1)) {
    if (shiftstate) {
      return 3;
    } else {
      return 1;
    }
  }
  return 0;
}

static int shiftPressed(void) {
  if (!(PINB & 1<<PB0)) {
    PORTC |= 1;
    return 1;
  }
  PORTC &= ~1;
  return 0;
}

static void buildReport(uchar key) {
  *(int *)reportBuffer = pgm_read_word(keyReport[key]);
}

usbMsgLen_t usbFunctionSetup(uchar data[8]) {
  usbRequest_t *rq = (void *)data;

  usbMsgPtr = reportBuffer;
  if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {
    switch(rq->bRequest) {
    case USBRQ_HID_GET_REPORT:
      buildReport(keyPressed(0));
      return sizeof(reportBuffer);
    case USBRQ_HID_GET_IDLE:
      usbMsgPtr = &idleRate;
      return 1;
    case USBRQ_HID_SET_IDLE:
      idleRate = rq->wValue.bytes[1];
    }
  }
  return 0; 
}

#define STATE_SEND_KEY 0
#define STATE_RELEASE_KEY 1

int main() {
  uchar state;

  PORTB = 0xff;  // activate all pull-ups for PIN B
  DDRB = 0;       // setup port B as input

  DDRC = 1;
  PORTC &= ~1;

  usbInit();
  usbDeviceConnect();
  sei();

  while(1) {
    usbPoll();
    state = STATE_SEND_KEY;
    if (usbInterruptIsReady()) {
      switch(state) {
      case STATE_SEND_KEY:
        if (shiftPressed()) {
          buildReport(keyPressed(1));
          break;
        }
        buildReport(keyPressed(0));
        state = STATE_RELEASE_KEY;
        break;
      case STATE_RELEASE_KEY:
        buildReport(0);
        state = STATE_SEND_KEY;
        break;
      default:
        state = STATE_SEND_KEY;
      }
      usbSetInterrupt(reportBuffer, sizeof(reportBuffer));
    }
  }
  
  return 0;
}
