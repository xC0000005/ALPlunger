Information on how to read the Legends Pinball plunger (which reads the accelerometer as well)

The plunger board has pins for 5v (and downconverts for the 3v accelerometer), GND, SCL and SDA.
Despite this, the protocol is only I2C-ish. Clock signal is inverted, there's no start/stop sequence
and it appears there are 11 clocks per packet. There's a start bit, a parity bit (even), a bit that's always 0 and 8 data bits. The resulting plunger values are from 0 (all the way in) to 65 (all the way out);

Data is transmitted in four packet sequences with 0 being the plunger and 1-3 being the data from the accelerometer, which should probably be left plugged in, since values are still sent even if it's disconnected, but the plunger waits longer to update position.

Speed appears to be standard mode i2c, but there's no address or command bytes, from startup
the chip simply screams four packet sets. If the accelerometer is plugged in, the packets are 4x as
frequent. Pulling SCL low and holding it for >107ms results in a reset so the reader can get clean 
data.

Because there is no signal for when data is ready and the bus master basically sends it constantly, the best way to interface with this is likely either a bluepill exporting the data as HID (a joystick) or, if you're using a control board with support for potentimeter plungers, use the analogWrite to mimic the plunger.

Accelerometer:
The acelerometer is at address 0x1C and looks to be a MMA8451, though one board I have is a knockoff that uses the same set of register commands. Once it's initialized the plunger reads register 1, six bytes, which makes sense if it's the standard XH, XL, YH, YL, ZH, ZL.
The address auto-increments on reads, so it's expected there wouldn't be register sets between reads. This means the unknown bytes
are likely (X, Y, Z) in order, though we'd need to tamper with the bus to be sure.

If the accelerometer is unplugged, the last three bytes (on the two board sets I have) are:
00FE 0000 0000
Plugging in the accelerometer (even without re-starting) immediately begins publishing a set of values that vary by about +/- 2. Tapping the accelerometer yields immediate changes but of course happens in all three axis. It would probably be possible to build a multi-master wiring in which the arduino changes the configuration values for axis and thus reveals which byte correlates to what axis. 

It seems clear that the plunger IC, in addition to reading the data, is performing averaging/ranging on the data and presenting it back. This may explain why a relatively functional accelerometer seems to give a rough experience (or it could be the pinball code.)

J9 / ribbon to main board
Pin order is from silk screen on main board
---------------    --------------
| 19 17 15 13 11  9  7  5  3  1 |
| 20 18 16 14 12 10  8  6  4  2 | 
---------------------------------

 1 VCC J1 4
 2 I2C SDA J1 3
 3 I2C CLK J1 2
 4 GND
 5 LFLIPPER J3 3
 6 LMAGNA J3 2
 7 DPAD ? J4 4
 8 DPAD ? J4 5
 9 DPAD ? J4 2
10 DPAD ? J4 3
11 FKEY REWIND J5 5
12 FKEY FNUDGE J5 4
13 FKEY MENU J5 3
14 FKEY START J5 2
15 RLIPPER J2 3
16 RFMAGNA J2 2
17 GND
18 SOLENOID J6 2
19 NOT CONNECTED J7 2
20 SOLENOID J8 2
