#include "os_rfid.h"

#include "defines.h"
#include "lcd.h"
#include "os_core.h"
#include "os_memory.h"
#include "util.h"

#include <string.h>

//! If set to 1 a received Tag will be written as HEX-String to the LCD
#define DISPLAY_RFID_TAG_AFTER_RECEIVE 0

//----------------------------------------------------------------------------
// RFID Constants
//----------------------------------------------------------------------------

//! Frequency of the RFID tag in Hz
#define F_RFID_OSC ?

//! Ratio of RFID Tag Frequency to RFID data rate
#define RFID_SCALER ?

//! Pre-scaler used for the timer
#define TIMER_SCALER ?

//! Number of timer ticks for a long edge
#define TICKSRATIO ?

//! Noise threshold for the RFID Reader
#define NOISE_THRESHOLD (TICKSRATIO / 4)

//! Length of Bit Record Buffer
#define RECORD_NUM (3 * 64)

//! Pin MOD
#define MOD PD5

//! Pin DEMOD_OUT
#define DEMOD_OUT PD6

//! Pin SHD
#define SHD PD7

//----------------------------------------------------------------------------
// Globals
//----------------------------------------------------------------------------

typedef ? RFID_Tag;
typedef ? ManchesterDecode;

typedef struct {
    uint16_t time;
    uint16_t ovfCount;
} Edge;

typedef struct {
    volatile RFID_Tag tagBuffer;
    uint64_t lastReceivedTag;
    volatile Edge edge[2];
    MemAddr signalTimes;
    uint8_t signalStartEdge : 1;
    uint8_t lastManchesterSkipped : 1;
    volatile uint8_t head;
    volatile uint8_t tail;
} RFID;
RFID rfid;

//----------------------------------------------------------------------------
// ISR
//----------------------------------------------------------------------------

/*!
 *  This ISR counts the Timer 1 Overflows to enable exact Measurements of Signal Level Lengths
 */
ISR(TIMER1_OVF_vect) {
    rfid.edge[0].ovfCount++;
    rfid.edge[1].ovfCount++;
}

/*!
 *  This ISR Analyzes the Signals from the RFID reader board
 *  and saves the Signal levels together with their durations
 *  in the global RFID_recTime and RFID_recLevel arrays
 */
ISR(TIMER1_CAPT_vect) {
    static uint16_t sp;
    sp = SP;
    SP = BOTTOM_OF_ISR_STACK;
    unsigned char counts;

    const uint8_t e = (PIND >> PD6) & 1;
    rfid.edge[e].time = ICR1;
    counts = rfid.edge[e].time - rfid.edge[!e].time + rfid.edge[!e].ovfCount * (1ul << 16);
    rfid.edge[e].ovfCount = 0;

    if (counts > NOISE_THRESHOLD) {
        intSRAM->write(rfid.signalTimes + rfid.head, counts);

        if (!rfid.head) {
            rfid.signalStartEdge = e;
        }
        rfid.head = (rfid.head + 1) % RECORD_NUM;
        TCCR1B = (TCCR1B & ~(1 << ICES1)) | ((!e) << ICES1);
    }

    SP = sp;
}

//----------------------------------------------------------------------------
// Function definitions
//----------------------------------------------------------------------------

/*!
 *  This function initializes the RFID Driver.
 *  The ports are set, then the RFID Chip on the reader board is initialized
 *  by a sequence of signals. In the end, Timer 1 is set.
 *  Afterwards, the global variables are set to their initialization values
 *  This also initializes our Memory as NULL pointers.
 */
void rfid_init() {
    memset(&rfid, 0, sizeof(RFID));
    rfid.signalTimes = os_malloc(intHeap, RECORD_NUM);

    // Ports
    PORTD = 0xA0; // PD7=SHD, PD6=DEMOD-OUT, PD5=MOD
    DDRD = 0xFF;
    DDRD |= (1 << SHD) | (1 << MOD);
    DDRD &= ~(1 << DEMOD_OUT);
    PORTD |= (1 << DEMOD_OUT); // Pull up (input)

    // Boot up the RFID reader chip
    PORTD |= (1 << SHD); // EM4095 shot down
    PORTD |= (1 << MOD); // EM4095 max output
    delayMs(500);        // Delay 500ms

    PORTD &= ~(1 << SHD); // EM4095 ready
    PORTD &= ~(1 << MOD); // EM4095 min output
    delayMs(500);         // Delay 500ms

    // Timer
    // Enable overflow and input capture interrupts
    TIMSK1 = 0x21;

    // Noise canceler, 256 pre-scaler, rising edge
    TCCR1B = 0x84;

    lcd_writeProgString(PSTR("RFID go! "));
}

/*!
 *  This function decodes the Manchester code that is used to transmit
 *  Data bytes from the RFID tag to our RFID reader.
 *  To do so, it analyzes signal level times and edges in the signal sent by the tag.
 *  \param level state of the edge under analysis (rising or falling)
 *  \param now_rec The duration of the current edge
 *  \returns 1 if we found a '1' bit, 0 if we found a '0' bit and a value > 1 if we could not identify a valid bit.
 */
ManchesterDecode rfid_decodeManchester(uint8_t level, uint16_t now_rec) {
#error IMPLEMENT STH. HERE
    return 0;
}

/*!
 *  If a header was previously found, this function does the data packet
 *  and column parity checks. If the checks were successful, the Tag data
 *  is returned after cleaning up.
 *  \returns 0 if the parity checks failed, and the Tag data if they succeeded.
 */
uint64_t rfid_getData() {
#error IMPLEMENT STH. HERE
    return 0;
}

/*!
 *  This function performs the whole process of receiving a RFID Tags Data.
 *  The function only terminates when a complete ID was received.
 *  \returns The actually received Tag ID.
 */
uint64_t rfid_receive(void) {
#error IMPLEMENT STH. HERE
    return 0;
}