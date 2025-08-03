Information on how to read the Legends Pinball plunger (which reads the accelerometer as well)

The plunger board has pins for 5v (and downconverts for the 3v accelerometer), GND, SCL and SDA.
Despite this, the protocol is only I2C-ish. Clock signal is inverted, there's no start/stop sequence
and it appears there are 11 clocks per packet

Data is transmitted in four packet sequences with 0 being the plunger and 1-3 being unknown.
Speed appears to be standard mode i2c, but there's no address or command bytes, from startup
the chip simply screams four packet sets. If the accelerometer is plugged in, the packets are 4x as
frequent. Pulling SCL low and holding it for >107ms results in a reset so the reader can get clean 
data.