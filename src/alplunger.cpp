/*
  Read the Legends Pinball Plunger's weird i2c-ish protocol
*/

#include <arduino.h>

// Clock in ten bits per packet
#define CLOCK_BITS 11
#define DATA_MASK 0x1FF
#define RESTART_PULSE_LENGTH 1000

#ifdef UNO 
    // this is for the uno r3 and nano
    #define I2C_PORT PINC
    #define SDA_MASK 0x10
    #define SCL_MASK 0x20
    #define PWM_PIN 5
#endif
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
#ifdef TINY85
    // Pin 0 PWM works.
    // Pin 1 is an LED and should be avoided.
    // Pin 2 worked fine for PWM.
    // Pin 3 is always on 
    // Pin 5 is always on
    #define I2C_PORT PINB
    #define SDA_MASK 0x4
    #define SCL_MASK 0x10
    #define SDA 2
    #define SCL 4
    #define PWM_PIN 0
#endif

// Plunger's the first value
#define PLUNGER_VALUE_INDEX 0
#define VALUES_PER_PACKET 4
#define MINIMUM_DELAY_FOR_UPDATE 100

// output data on mcus that support it
// #define SERIAL_DEBUGGING 

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
    #ifdef LED_SIGNAL
    byte led = 0;
    #endif
    byte data_index = 0;
    int last_update = millis();

    // This is a value that can't happen, ensuring our first read results
    // in a analogWrite.
    short current_plunger_value = 0x2FF;
    short inbound_plunger_value = 0;

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

    int write_counter = 0;
    int led = 0;

    for (;;)
    {
        scl = I2C_PORT & SCL_MASK;

        // if SCL has changed to low, clock a bit
        if (scl < previous_scl)
        {
            write_counter++;
            if (write_counter % 50 == 0) {

            digitalWrite(LED_BUILTIN, led);
            led = ! led;
            }

            sda = (I2C_PORT & SDA_MASK) == SDA_MASK;
            input |= (sda << bit);

            bit++;

            if (bit == CLOCK_BITS)
            {
                if (data_index == PLUNGER_VALUE_INDEX)
                {
                    inbound_plunger_value = ((input & DATA_MASK) >> 1); 
                    if (current_plunger_value != inbound_plunger_value)
                    {
                        int now = millis();
                        if (now - last_update > MINIMUM_DELAY_FOR_UPDATE)
                        {
                            // Values range from 0 to 65. Map it so that we always return at least 1,
                            // then multiply by 4 and clamp at 255.
                            unsigned short plunger_adapted = inbound_plunger_value << 2;
                            if (!plunger_adapted)
                            {
                                plunger_adapted = 1;
                            }

                            if (plunger_adapted > 255) 
                            {
                                plunger_adapted = 255; 
                            }

                            analogWrite(PWM_PIN, plunger_adapted);
                            current_plunger_value = inbound_plunger_value;
                            last_update = millis();
                        }

                        inbound_plunger_value = 0;
                    }
                }

                data_index++;

                if (data_index == VALUES_PER_PACKET) {
                    data_index = 0;
                }

                bit = 0;
                input = 0;
            }
        }

        previous_scl = scl;
    }
}