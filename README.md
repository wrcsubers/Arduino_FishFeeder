# NodeMCU based Fish Feeder

#### Video of Fish Feeder in 'test mode': https://youtu.be/s49Ig8Wbufc

### Fish Feeder consists of
* ESP8266/NodeMCU Microcontroller
* 28BYJ-48 Stepper Motor with ULN2003 Driver
* Custom built wood stand with height adjustment
* Powered via USB

The first version of this project was an automatic fish feeder that would feed once per day with a capacity for 16 days of feeding.  It uses NTP synchronization to assure the time is accurate.  Also contains a VERY simple built in webpage with a log which can be accessed via VPN when away from home.  

#### Future Plans:
* Set parameters via webpage
  * Set number of feedings
  * Set time of feedings
  * Test feeder
  * Nudge feeding disk forward/backward
* Add RGB Lighting for day/night cycles
