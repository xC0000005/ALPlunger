/*
  Read the Legends Pinball Plunger's weird i2c-ish protocol
*/

#include <arduino.h>

// Clock in ten bits per packet
#define CLOCK_BITS 11
#define SDA_MASK 0x10
#define SCL_MASK 0x20
#define DATA_MASK 0xFF
#define RESTART_PULSE_LENGTH 1000
#define VALUES_PER_PACKET 4

// on a tiny, this isn't available
#define USE_SERIAL_COMS 

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

#define SAMPLE_SIZE 512
short data[SAMPLE_SIZE] = {};

void setup()
{
#ifdef USE_SERIAL_COMS
    Serial.begin(19200);
    Serial.println();
#endif
}

void loop()
{
    int previous_scl = SCL_MASK;
    int scl = 0;
    int sda = 1;
    short input = 0;
    short bit = 0;
    int packet_counter = 0;

    // the Arduino way is to do all this in Setup()
    // but after signaling the reset we should move quickly
    // to start the read loop and we have to muck with pinModes anyway
    pinMode(SCL, OUTPUT);

    // Restart the bitstream so we dont' start in the middle of packets
    digitalWrite(SCL, LOW);
    delay(RESTART_PULSE_LENGTH);
    pinMode(SCL, INPUT);
    pinMode(SDA, INPUT);

    for (;;)
    {
        scl = PINC & SCL_MASK;

        // if SCL has changed to low, clock a bit
        if (previous_scl < scl)
        {
            sda = (PINC & SDA_MASK) == SDA_MASK;
            input |= (sda << bit);

            bit++;

            if (bit == CLOCK_BITS)
            {
                data[data_index++] = input & DATA_MASK;
                bit = 0;
                input = 0;

                if (data_index % VALUES_PER_PACKET == 0)
                {
                    if (current_plunger_data.plunger_value != inbound_plunger_data.plunger_value)
                    {
                        Serial.println(inbound_plunger_data.plunger_value);
                        current_plunger_data.plunger_value = inbound_plunger_data.plunger_value;
                        memset(&inbound_plunger_data, 0, sizeof(inbound_plunger_data));
                    }

                    packet_counter = 0;
                }
            }
        }

        previous_scl = scl;

        if (data_index == SAMPLE_SIZE)
        {
            break;
        }
    }

    #ifdef USE_SERIAL_COMS
    {
        char message_buffer[256];

        // Display
        for (int i = 0; i < SAMPLE_SIZE; i += 4)
        {
            sprintf(message_buffer, "%04X %04X %04X %04X\r\n", data[i], data[i + 1], data[i + 2], data[i + 3]);
            Serial.print(message_buffer);
        }
    }

    delay(10000);
    #endif
}