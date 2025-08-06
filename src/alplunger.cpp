/*
  Read the Legends Pinball Plunger's weird i2c-ish protocol
*/

#include <arduino.h>

// Clock in ten bits per packet
#define CLOCK_BITS 11
#define DATA_MASK 0x1FF
#define RESTART_PULSE_LENGTH 1000

#ifdef LEONARDO
    #define I2C_PORT PIND
    #define SDA_MASK 0x2
    #define SCL_MASK 0x1
    // using D9 for leonardo/beetle 
    #define PWM_PIN 9
#endif
#ifdef TINY88
    #define I2C_PORT PINC
    #define SDA_MASK 0x10
    #define SCL_MASK 0x20
    #define PWM_PIN 9
#endif
#ifdef UNO 
    // this is for the uno r3 and nano
    #define I2C_PORT PINC
    #define SDA_MASK 0x10
    #define SCL_MASK 0x20
    #define PWM_PIN 5
#endif

// Plunger's the first value
#define PLUNGER_VALUE_INDEX 0
#define VALUES_PER_PACKET 4

// #define SERIAL_DEBUGGING 

// These types are shorts in case the unknown bits turn out to be
// useful. If not, they could be converted to bytes
struct PlungerData
{
    short plunger_value;
    short unknown1;
    short unknown2;
    short unknown3;
};

PlungerData current_plunger_data;
PlungerData inbound_plunger_data;
short data_index = 0;

short *data = (short *)&inbound_plunger_data;

void setup()
{
#ifdef SERIAL_DEBUGGING
    Serial.begin(19200);
    while (!Serial);
    Serial.println("starting");
#endif
}

void loop()
{    
    int previous_scl = SCL_MASK;
    int scl = 0;
    int sda = 1;
    short input = 0;
    short bit = 0;

    // the Arduino way is to do all this in Setup()
    // but after signaling the reset we should move quickly
    // to start the read loop and we have to muck with pinModes anyway
    pinMode(PWM_PIN, OUTPUT); 
    pinMode(SCL, OUTPUT);

    // Restart the bitstream so we dont' start in the middle of packets
    digitalWrite(SCL, LOW);
    delay(RESTART_PULSE_LENGTH);
    pinMode(SCL, INPUT);
    pinMode(SDA, INPUT);

    for (;;)
    {
        scl = I2C_PORT & SCL_MASK;

        // if SCL has changed to low, clock a bit
        if (scl < previous_scl)
        {
            sda = (I2C_PORT & SDA_MASK) == SDA_MASK;
            input |= (sda << bit);

            bit++;

            if (bit == CLOCK_BITS)
            {
                data[data_index++] = ((input & DATA_MASK) >> 1);
                bit = 0;
                input = 0;

                if (data_index % VALUES_PER_PACKET == PLUNGER_VALUE_INDEX)
                {
                    if (current_plunger_data.plunger_value != inbound_plunger_data.plunger_value)
                    {
                        #ifdef SERIAL_DEBUGGING
                        Serial.println(inbound_plunger_data.plunger_value & 0xFF);
                        //char buffer[64];
                        //sprintf(buffer, "%04X %04X %04X", inbound_plunger_data.unknown1, inbound_plunger_data.unknown2, inbound_plunger_data.unknown3);
                        //Serial.println(buffer);
                        #endif

                        // Values range from 0 to 65. Map it so that we always return at least 1,
                        // then multiply by 4 and clamp at 255.
                        short plunger_adapted = inbound_plunger_data.plunger_value << 2;
                        if (!plunger_adapted)
                        {
                            plunger_adapted = 1;
                        }

                        if (plunger_adapted > 255) 
                        {
                            plunger_adapted = 255; 
                        }

                        analogWrite(PWM_PIN, plunger_adapted);
                        current_plunger_data.plunger_value = inbound_plunger_data.plunger_value;
                        memset(&inbound_plunger_data, 0, sizeof(inbound_plunger_data));
                    }
                    data_index = 0;
                }
            }
        }

        previous_scl = scl;
    }
}