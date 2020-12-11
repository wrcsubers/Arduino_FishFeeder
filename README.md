# NodeMCU based Fish Feeder

---

### Check out Fish Feeder Version 2 ---> https://github.com/wrcsubers/Arduino_FishFeeder_v2

---

#### Video of Fish Feeder in 'test mode': https://youtu.be/s49Ig8Wbufc

### Fish Feeder consists of
* ESP8266/NodeMCU Microcontroller
* 28BYJ-48 Stepper Motor with ULN2003 Driver
* Custom built wood stand with height adjustment
* Powered via USB

The first version of this project was an automatic fish feeder that would feed once per day with a capacity for 16 days of feeding.  It uses NTP synchronization to assure the time is accurate.  Also contains a VERY simple built in webpage with a log which can be accessed via VPN when away from home.  

Connect as follows:

![FishFeederWiringDiagram](https://github.com/wrcsubers/Arduino_FishFeeder/blob/main/_Images/Diagram_Wiring.png)

You can access the status page by visiting the IP Address (available by serial monitor) in any browser or by visiting 'FishFeeder.local' in your browser.
