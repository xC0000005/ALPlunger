Information on how to read the Legends Pinball plunger (which reads the accelerometer as well)

The plunger board has pins for 5v (and downconverts for the 3v accelerometer), GND, SCL and SDA.
Despite this, the protocol is only I2C-ish. Clock signal is inverted, there's no start/stop sequence
and it appears there are 11 clocks per packet. It looks from traces like the last two are always 1, so
if this is i2c-like, that's a "write" (which it is, the bus master is writing) and an ack (as in, it doesn't care if you ack it, it's confirming it sent it.)

Data is transmitted in four packet sequences with 0 being the plunger and 1-3 being unknown, but very
likely the data from the accelerometer, which should probably be left plugged in, since values are still sent even if it's disconnected, but the plunger waits longer to update position.
Speed appears to be standard mode i2c, but there's no address or command bytes, from startup
the chip simply screams four packet sets. If the accelerometer is plugged in, the packets are 4x as
frequent. Pulling SCL low and holding it for >107ms results in a reset so the reader can get clean 
data.

Because there is no signal for when data is ready and the bus master basically sends it constantly, the best way to interface with this is likely either a bluepill exporting the data as HID (a joystick) or, if you're using a control board with support for potentimeter plungers, use the analogWrite to mimic the plunger.

Accelerometer bytes:
If the accelerometer is unplugged, the last three bytes (on the two board sets I have) are:
00FE 0000 0000
Plugging in the accelerometer (even without re-starting) immediately begins publishing a set of values that vary by about +/- 2. Tapping the accelerometer yields immediate changes but of course happens in all three axis. It would probably be possible to build a multi-master wiring in which the arduino changes the configuration values for axis and thus reveals which byte correlates to what axis. 

It seems clear that the plunger IC, in addition to reading the data, is performing averaging/ranging on the data and presenting it back. This may explain why a relatively functional accelerometer seems to give a rough experience (or it could be the pinball code.)