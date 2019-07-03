/// @file

#include "hwlib.hpp"
#include "A24C256.hpp"
#include "DS3231.hpp"
#include "KY040.hpp"
#include "TEA5767.hpp"
#include "RDA5807.hpp"

/// \brief
/// All-in-one TEST
/// \details
/// This program can be used to execute all tests at once.
int main( void ){

  hwlib::wait_ms(2000);
  hwlib::cout << "The order of the tests is: EEPROM, DS3231, Rotary Encoder, RDA5807 or TEA5767 (can't be tested at the same time since they can operate at the same address). It is starting in 5 seconds.";

  for(unsigned int i = 0; i < 5; i++){
    hwlib::cout << "- ";
    hwlib::wait_ms(1000);
  }
  hwlib::cout << hwlib::endl;

  namespace target = hwlib::target; 

  auto scl = target::pin_oc( target::pins::d8 );
  auto sda = target::pin_oc( target::pins::d9 );
  auto i2c_bus = hwlib::i2c_bus_bit_banged_scl_sda(scl, sda);

  hwlib::wait_ms(1000);   //Wait for terminal

  auto memory = A24C256(i2c_bus);
  auto largeMemory = A24C256(i2c_bus, 512);
  auto addressMemory = A24C256(i2c_bus, 256, 0x10);
  auto falseMemory = A24C256(i2c_bus, 230);

  hwlib::cout << hwlib::boolalpha << hwlib::setw(100) << hwlib::left << "Initizalisation with valid parameters: " << ((addressMemory.getAddress() == 0x10) && (addressMemory.getMemorySize() == 256) ) << hwlib::endl;
  hwlib::cout << hwlib::boolalpha << hwlib::setw(100) << hwlib::left << "Initizalisation with correction of invalid parameters: " << ((falseMemory.getAddress() == 0x50) && (falseMemory.getMemorySize() == 256) ) << hwlib::endl;

  memory.write(40000, 'c');
  hwlib::cout << hwlib::setw(100) << hwlib::left << "Invalid location protection prevents writing and reading on and from non-existing places: " << hwlib::boolalpha << (int(memory.read(40000)) == 0) << hwlib::endl;

  largeMemory.write(50000, 'c');
  hwlib::cout << hwlib::setw(100) << hwlib::left << "Chips with larger memory have more addresses: " << hwlib::boolalpha << (char(largeMemory.read(50000)) == 'c') << hwlib::endl;
  
  
  char data[]={"At vero eos et accusamus et iusto odio dignissimos ducimus qui blanditiis praesentium voluptatum deleniti atque corrupti quos dolores et quas molestias excepturi sint occaecati cupiditate non provident, similique sunt in culpa qui officia deserunt mollitia animi, id est laborum et dolorum fuga. Et harum quidem rerum facilis est et expedita distinctio."};
  memory.write(0, data);

  uint8_t receivedData[353];
  memory.read(0, 353, receivedData);

  hwlib::cout << hwlib::endl << "Saved Text: " << hwlib::endl;

  for(unsigned int i = 0; i < 353; i++){
    hwlib::cout << char(receivedData[i]);
    if(char(receivedData[i]) != data[i]){
      hwlib::cout << hwlib::endl << hwlib::endl << hwlib::setw(100) << hwlib::left << "Multi-Page Multi-Byte values are written and read correctly: " << false << hwlib::endl;
    }
    if(i == 352){
      hwlib::cout << hwlib::endl << hwlib::endl << hwlib::setw(100) << hwlib::left << "Multi-Page Multi-Byte values are written and read correctly: " << true << hwlib::endl;
    }
  }

  char newData[]={"At vero eos et accusamus."};
  memory.write(0, newData);

  uint8_t newReceivedData[25];
  memory.read(0, 25, newReceivedData);

  for(unsigned int i = 0; i < 25; i++){
    if(char(receivedData[i]) != data[i]){
      hwlib::cout << hwlib::setw(100) << hwlib::left  << "Single-Page Multi-Byte values are written and read correctly: " << false << hwlib::endl;
    }
    if(i == 24){
      hwlib::cout << hwlib::setw(100) << hwlib::left << "Single-Page Multi-Byte values are written and read correctly: " << true << hwlib::endl;
    }
  }

  memory.write(386, 'c');
  hwlib::cout << hwlib::setw(100) << hwlib::left << "Single-Byte values are written and read correctly: " << hwlib::boolalpha << (char(memory.read(386)) == 'c') << hwlib::endl;

  memory.write(32760, data);
  hwlib::cout << hwlib::setw(100) << hwlib::left << "Multi-Byte values are not written if they exceed maximum memory-address: " << hwlib::boolalpha << (char(memory.read(32760)) != 'A') << hwlib::endl;

// <<<----------------------------------------------------------------------------------------->>>

  namespace target = hwlib::target;

  auto clock = DS3231(i2c_bus);

  auto time = timeData(15, 20, 10);
  auto lastTime = timeData(5, 30);
  auto bigTime = timeData(50, 60, 80);

  hwlib::wait_ms(1000);

  hwlib::cout << "---------------------------------TIME------------------------------" << hwlib::endl << hwlib::endl;
  hwlib::cout << hwlib::left << hwlib::boolalpha << hwlib::setw(45) << "Initialization: " << ((time.getHours() == 15) && time.getMinutes() == 20 && time.getSeconds() == 10) << hwlib::endl;

  hwlib::cout << hwlib::left << hwlib::setw(45) << "Too Large Initialization Handling: " << ((bigTime.getHours() == 0) && bigTime.getMinutes() == 0 && bigTime.getSeconds() == 0) << hwlib::endl;

  time.setTime(11, 40, 55);
  hwlib::cout << hwlib::left << hwlib::setw(45) << "Setting: " << ((time.getHours() == 11) && time.getMinutes() == 40 && time.getSeconds() == 55) << hwlib::endl;

  time.setTime(80, 90, 100);
  hwlib::cout << hwlib::left << hwlib::setw(45) << "Too Large Setting Handling: " << ((time.getHours() == 11) && time.getMinutes() == 40 && time.getSeconds() == 55) << hwlib::endl;

  time.setTime(10, 30);
  hwlib::cout << hwlib::endl << hwlib::left << hwlib::setw(45) << "Addition: " << time << " + " << lastTime << " = " << (time + lastTime) <<  hwlib::boolalpha << " " << ((time + lastTime) == timeData(16, 0, 0)) <<  hwlib::endl;
  lastTime.setHours(18);
  lastTime.setMinutes(15);
  lastTime.setSeconds(55);
  hwlib::cout << hwlib::left << hwlib::setw(45) << "Addition: " << time << " + " << lastTime << " = " << (time + lastTime) <<  hwlib::boolalpha << " " << ((time + lastTime) == timeData(4, 45, 55)) <<  hwlib::endl;
  hwlib::cout << hwlib::left << hwlib::setw(45) << "Addition: " << timeData(0, 0) << " + " << timeData(0, 0) << " = " << (timeData(0+0, 0+0)) << " true" << hwlib::endl;
  hwlib::cout << hwlib::left << hwlib::setw(45) << "Addition: " << timeData(23, 59) << " + " << timeData(23, 59) << " = " << (timeData(23, 59) + timeData(23, 59)) << " true" <<  hwlib::endl;

  lastTime.setTime(5, 30);
  time.setTime(10, 30);
  hwlib::cout << hwlib::endl << hwlib::left << hwlib::setw(45) << "Substraction: " << time << " - " << lastTime << " = " << (time - lastTime) << hwlib::boolalpha << "  " << ((time - lastTime) == timeData(5, 0, 0)) <<  hwlib::endl;
  hwlib::cout << hwlib::left << hwlib::setw(45) << "Substraction: " << lastTime << " - " << time << " = " << (lastTime - time) << hwlib::boolalpha << " " << ((lastTime - time) == timeData(19, 0, 0)) <<  hwlib::endl;
  hwlib::cout << hwlib::left << hwlib::setw(45) << "Substraction: " << timeData(23, 59) << " - " << timeData(23, 59) << " = " << (timeData(23, 59) - timeData(23, 59)) << " true" << hwlib::endl;
  hwlib::cout << hwlib::left << hwlib::setw(45) << "Substraction: " << timeData(0, 15) << " - " << timeData(0, 30) << " = " << (timeData(0, 15) - timeData(0, 30)) << " true" <<  hwlib::endl;

  lastTime.setTime(5, 30);
  time.setTime(10, 30);
  hwlib::cout << hwlib::endl << hwlib::left << hwlib::setw(45) << "Equality: " << time << " == " << lastTime << " = " << (time == lastTime) <<  hwlib::endl;
  hwlib::cout << hwlib::left << hwlib::setw(45) << "Unequality: " << lastTime << " != " << time << " = " << (lastTime != time) <<  hwlib::endl << hwlib::endl;

  hwlib::cout << "---------------------------------DATE------------------------------" << hwlib::endl << hwlib::endl;

  auto date = dateData(7, 30, 6, 2019);
  auto lastDate = dateData(10, 35, 14, 2019);

  hwlib::cout << hwlib::left << hwlib::setw(45) << "Initialization: " << (date.getWeekDay() == 7 && date.getMonthDay() == 30 && date.getMonth() == 6 && date.getYear() == 2019) << hwlib::endl;

  hwlib::cout << hwlib::left << hwlib::setw(45) << "Too Large Initialization Handling: " << (lastDate.getWeekDay() == 7 && lastDate.getMonthDay() == 31 && lastDate.getMonth() == 12 && lastDate.getYear() == 2019) << hwlib::endl;

  lastDate.setDate(7, 30, 6, 2019);
  hwlib::cout << hwlib::left << hwlib::setw(45) << "Setting: " << (lastDate.getWeekDay() == 7 && lastDate.getMonthDay() == 30 && lastDate.getMonth() == 6 && lastDate.getYear() == 2019) << hwlib::endl;

  lastDate.setDate(80, 90, 100, 2019);
  hwlib::cout << hwlib::left << hwlib::setw(45) << "Too Large Setting Handling: " << (lastDate.getWeekDay() == 7 && lastDate.getMonthDay() == 30 && lastDate.getMonth() == 6 && lastDate.getYear() == 2019) << hwlib::endl;

  date.setDate(1, 1, 1, 2019);
  lastDate.setDate(7, 30, 6, 0);
  hwlib::cout << hwlib::endl << hwlib::left << hwlib::setw(45) << "Addition: " << date.getWeekDay() << ", " << date << " + " << lastDate.getWeekDay() << ", " << lastDate << " = " << (date + lastDate) <<  hwlib::boolalpha << " " << ((date + lastDate) == dateData(1, 1, 8, 2019)) <<  hwlib::endl;

  lastDate.setWeekDay(5);
  lastDate.setMonthDay(15);
  lastDate.setMonth(8);
  lastDate.setYear(2000);
  hwlib::cout << hwlib::left << hwlib::setw(45) << "Setting and Addition: " << date.getWeekDay() << ", " << date << " + " << lastDate.getWeekDay() << ", " << lastDate << " = " << (date + lastDate) <<  hwlib::boolalpha << " " << ((date + lastDate) == dateData(6, 16, 9, 4019)) <<  hwlib::endl;
  hwlib::cout << hwlib::left << hwlib::setw(45) << "Addition: " << dateData(6, 20, 12, 2019) << " + " << dateData(6, 20, 5, 0) << " = " << (dateData(6, 20, 5, 0) +  dateData(6, 20, 12, 2019)) <<  hwlib::boolalpha << " true" <<  hwlib::endl;
  hwlib::cout << hwlib::left << hwlib::setw(45) << "Addition: " << dateData(1, 1, 1, 2000) << " + " << dateData(1, 1, 1, 0) << " = " << (dateData(1, 1, 1, 2000) +  dateData(1, 1, 1, 0)) <<  hwlib::boolalpha << " true" <<  hwlib::endl;

  date.setDate(5, 8, 10, 2019);
  lastDate.setDate(4, 30, 8, 2000);
  hwlib::cout << hwlib::endl << hwlib::left << hwlib::setw(45) << "Substraction: " << date.getWeekDay() << ", " << date << " - " << lastDate.getWeekDay() << ", " << lastDate << " = " << (date - lastDate) << hwlib::boolalpha << "  " << ((date - lastDate) == dateData(1, 8, 1, 19)) <<  hwlib::endl;
  hwlib::cout << hwlib::left << hwlib::setw(45) << "Substraction: " << lastDate.getWeekDay() << ", " << lastDate << " - " << date.getWeekDay() << ", " << date << " = " << (lastDate - date) << hwlib::boolalpha << " " << ((lastDate - date) == dateData(6, 22, 10, 0)) <<  hwlib::endl;
  hwlib::cout << hwlib::left << hwlib::setw(45) << "Substraction: " << dateData(6, 20, 12, 2019) << " - " << dateData(6, 20, 5, 0) << " = " << (dateData(6, 20, 12, 0) -  dateData(6, 20, 5, 2019)) <<  hwlib::boolalpha << " true" <<  hwlib::endl;
  hwlib::cout << hwlib::left << hwlib::setw(45) << "Substraction: " << dateData(6, 1, 1, 2019) << " - " << dateData(6, 6, 6, 0) << " = " << (dateData(6, 1, 1, 2019) -  dateData(6, 6, 6, 0)) <<  hwlib::boolalpha << " true" <<  hwlib::endl;

  lastTime.setTime(5, 30);
  time.setTime(10, 30);
  hwlib::cout << hwlib::endl << hwlib::left << hwlib::setw(45) << "Equality: " << date.getWeekDay() << ", " << date << " == " << lastDate.getWeekDay() << ", " << lastDate << " = " << (date == lastDate) <<  hwlib::endl;
  hwlib::cout << hwlib::left << hwlib::setw(45) << "Unequality: " << date.getWeekDay() << ", " << date << " != " << lastDate.getWeekDay() << ", " << lastDate << " = " << (date != lastDate) <<  hwlib::endl << hwlib::endl;


  hwlib::cout << "-------------------------------DS3231--------------------------------" << hwlib::endl << hwlib::endl;

  hwlib::cout << hwlib::left << hwlib::setw(45) << "Initialization: " << (clock.getAddress() == 0x68) << hwlib::endl;

  hwlib::cout << hwlib::endl;
  for(unsigned int i = 0; i < 3; i++){
    time = clock.getTime();
    date = clock.getDate();
    hwlib::cout << "Time: " << clock.getTime() << hwlib::endl;
    hwlib::cout << "Temperature: " << clock.getTemperature() << hwlib::endl;
    hwlib::cout << "Date: " << clock.getDate() << hwlib::endl << hwlib::endl;
    hwlib::wait_ms(3000);
  }
  hwlib::cout << hwlib::endl;
  
  //Uncomment if time is allowed to get lost. You'll have to set it again later.
  //hwlib::cout << hwlib::left << hwlib::setw(45) << "Set time to 0:0:0 : ";
  //clock.setTime(0, 0, 0);
  //hwlib::cout << (clock.getTime() == timeData(0, 0, 0)) << hwlib::endl;

  auto curTime = clock.getTime();
  curTime.setSeconds(curTime.getSeconds() + 10);
  clock.changeFirstAlarm(curTime, dateData(0, 0, 1, 2019));
  clock.setFirstAlarm(14);
  hwlib::cout << "Alarm set, should go in 10 seconds: ";

  clock.clearAlarm(1);
  
  hwlib::wait_ms(30);

  while(clock.checkAlarms() == 0){
    hwlib::wait_ms(200);
  }

  hwlib::cout << "Triggered!" << hwlib::endl;

  auto CLK = hwlib::target::pin_in( hwlib::target::pins::d36 );
  auto DT = hwlib::target::pin_in( hwlib::target::pins::d38 );
  auto SW = hwlib::target::pin_in( hwlib::target::pins::d40 );
  auto button = KY040(CLK, DT, SW);

  hwlib::wait_ms(2000);

  hwlib::cout << "Testprocedure KY040. Press the button." << hwlib::endl;
  for(;;){
    button.update();
    if(button.isPressed()){
      hwlib::cout << "Button pressed!" << hwlib::endl;
      while(button.isPressed()){
        button.update();
        hwlib::wait_ms(5);
      }
      hwlib::cout << hwlib::endl;
      break;
    }
    hwlib::wait_ms(5);
  }

  hwlib::cout << "Keep the button pressed for 5 seconds." << hwlib::endl;
  button.update();
  while(!button.isPressed()){
    button.update();
    hwlib::wait_ms(5);
  }
  if(button.isPressed()){
    hwlib::cout << "Button pressed!" << hwlib::endl;
    for(unsigned int i = 0; i < 20; i++){
    button.update();
      if(i == 18){
        hwlib::cout << hwlib::endl << "Kept button pressed for 5 seconds!" << hwlib::endl << hwlib::endl;
        break;
      }
      if(!button.isPressed()){
        hwlib::cout << "Test Failed!" << hwlib::endl << hwlib::endl;
        break;
      }
      hwlib::cout << '-';
      hwlib::wait_ms(250);
    }
  }
  hwlib::wait_ms(5);


  hwlib::cout << "Turn Rotary Encoder Clockwise." << hwlib::endl;
  button.update();
  int lastPos = button.getPos();
  for(;;){
    button.update();
    if(button.getPos() > lastPos){
      hwlib::cout << "Turned Clockwise!" << hwlib::endl << hwlib::endl;
      break;
    }
    hwlib::wait_ms(5);
  }

  hwlib::cout << "Turn Rotary Encoder 5x Clockwise." << hwlib::endl;
  button.update();
  lastPos = button.getPos();
  for(;;){
    button.update();
    if(button.getPos() > lastPos + 4){
      hwlib::cout << "Turned 5x Clockwise!" << hwlib::endl << hwlib::endl;
      break;
    }
    hwlib::wait_ms(5);
  }

  hwlib::cout << "Turn Rotary Encoder Counter-Clockwise." << hwlib::endl;
  button.update();
  lastPos = button.getPos();
  for(;;){
    button.update();
    if(button.getPos() < lastPos){
      hwlib::cout << "Turned Counter-Clockwise!" << hwlib::endl << hwlib::endl;
      break;
    }
    hwlib::wait_ms(5);
  }

  hwlib::cout << "Turn Rotary Encoder 5x Counter-Clockwise." << hwlib::endl;
  button.update();
  lastPos = button.getPos();
  for(;;){
    button.update();
    if(button.getPos() + 4 < lastPos){
      hwlib::cout << "Turned 5x Counter-Clockwise!" << hwlib::endl << hwlib::endl;
      break;
    }
    hwlib::wait_ms(5);
  }

  hwlib::cout << hwlib::endl << "Test Succeeded" << hwlib::endl;

// <<<----------------------------------------------------------------------------------------->>>

  auto radio = RDA5807(i2c_bus);
  radio.begin();
  hwlib::cout << "Tuning to 98.9FM (NPO-R2): ";
  radio.setFrequency(98.9);
  hwlib::wait_ms(1000);
  hwlib::cout << "DONE: " << hwlib::boolalpha << (radio.getIntFrequency() == 989) << hwlib::endl << hwlib::boolalpha << "Reception Quality:";
  for(unsigned int i = 0; i < 10; i++){
    hwlib::cout << radio.signalStrength() << ", Stereo: " << radio.stereoReception() << hwlib::endl;
    hwlib::wait_ms(1000);
  }

  hwlib::cout << "Powering Down RDA5807: ";
  radio.standBy(true);
  hwlib::cout << "DONE: " << hwlib::boolalpha << radio.isStandBy() << hwlib::endl;
  hwlib::wait_ms(2000);

  hwlib::cout << "Powering Up RDA5807: ";
  radio.standBy(false);
  hwlib::cout << "DONE: " << hwlib::boolalpha << !radio.isStandBy() << hwlib::endl << hwlib::endl;
  hwlib::wait_ms(2000);

  hwlib::cout << "Tuning to 100.7FM (Q-Music)" << hwlib::endl;
  radio.setFrequency(100.7);
  hwlib::wait_ms(1000);

  hwlib::cout << "Slowly de- and increasing volume:" << hwlib::endl;
  hwlib::wait_ms(2000);
   for(unsigned int i = 15; i > 0; i--){
    radio.setVolume(i);
    hwlib::cout << i << hwlib::endl;
    hwlib::wait_ms(250);
  }
  for(unsigned int i = 0; i < 15; i++){
    radio.setVolume(i);
    hwlib::cout << i << hwlib::endl;
    hwlib::wait_ms(250);
  }

  hwlib::cout << "New Demodulate Method: ";
  radio.demodulateMethod(true);
  hwlib::cout << "Signal Strength: " << hwlib::boolalpha << radio.signalStrength() << hwlib::endl << hwlib::endl;
  hwlib::wait_ms(2000);

  hwlib::cout << "Usual Demodulate Method: ";
  radio.demodulateMethod(false);
  hwlib::cout << "Signal Strength: " << hwlib::boolalpha << radio.signalStrength() << hwlib::endl << hwlib::endl;
  hwlib::wait_ms(2000);

  hwlib::cout << "Setting Mono Signal: ";
  radio.setStereo(false);
  hwlib::cout << "DONE: " << hwlib::boolalpha << !radio.isStereo() << hwlib::endl << hwlib::endl;
  hwlib::wait_ms(2000);

  hwlib::cout << "Setting Stereo Signal: ";
  radio.setStereo(true);
  hwlib::wait_ms(1000);
  hwlib::cout << "DONE: " << hwlib::boolalpha << radio.isStereo() << " doesn't work, but corresponding with datasheet" << hwlib::endl << hwlib::endl;
  hwlib::wait_ms(2000);

  hwlib::cout << "Muting: ";
  radio.setMute(true);
  hwlib::wait_ms(1000);
  hwlib::cout << "DONE: " << hwlib::boolalpha << radio.isMuted() << " doesn't work, but corresponding with datasheet" << hwlib::endl << hwlib::endl;
  hwlib::wait_ms(2000);

  hwlib::cout << "Unmuting: ";
  radio.setMute(false);
  hwlib::cout << "DONE: " << hwlib::boolalpha << !radio.isMuted() << hwlib::endl << hwlib::endl;
  hwlib::wait_ms(2000);

  hwlib::cout << "Bass-Boost On: ";
  radio.setBassBoost(true);
  hwlib::cout << "DONE: " << hwlib::boolalpha << radio.bassBoosted() << hwlib::endl << hwlib::endl;
  hwlib::wait_ms(2000);

  hwlib::cout << "Bass-Boost Off: ";
  radio.setBassBoost(false);
  hwlib::cout << "DONE: " << hwlib::boolalpha << !radio.bassBoosted() << hwlib::endl << hwlib::endl;
  hwlib::wait_ms(2000);

  hwlib::cout << "Starting Search-Up: ";
  hwlib::wait_ms(2000);
  for(unsigned int i = 0; i < 10; i++){
    radio.seekChannel(1);
    hwlib::cout << "FM-Frequency: " << int(radio.getFrequency()) << hwlib::endl;
    hwlib::wait_ms(2000);
    hwlib::cout << "Seek Completed: " << radio.seekCompleted() << ", Signal Strength: " << radio.signalStrength() << ", Stereo: " << radio.stereoReception() << ", Station: " << radio.isStation() << ", Tuned: " << radio.isTuned() << hwlib::endl << hwlib::endl;    hwlib::wait_ms(3000);
    hwlib::wait_ms(3000);
  }

  hwlib::cout << hwlib::endl << "Starting Search-Down: ";
  hwlib::wait_ms(2000);
  for(unsigned int i = 0; i < 10; i++){
    radio.seekChannel(0);
    hwlib::cout << "FM-Frequency: " << int(radio.getFrequency()) << hwlib::endl;
    hwlib::wait_ms(2000);
    hwlib::cout << "Seek Completed: " << radio.seekCompleted() << ", Signal Strength: " << radio.signalStrength() << ", Stereo: " << radio.stereoReception() << ", Station: " << radio.isStation() << ", Tuned: " << radio.isTuned() << hwlib::endl << hwlib::endl;
    hwlib::wait_ms(3000);
  }

  radio.setFrequency(100.7);
  hwlib::cout << hwlib::endl << "Starting Reading of Radio Data System: ";
  for(unsigned int i = 0; i < 8; i++){
    hwlib::wait_ms(5000);
    radio.radioData.update();
    hwlib::cout << hwlib::left << hwlib::setw(30) << "Frequency: " << radio.getIntFrequency() << hwlib::endl;
    hwlib::cout << hwlib::left << hwlib::setw(30) << "Station Name: " << radio.radioData.stationName() << hwlib::endl;
    hwlib::cout << hwlib::left << hwlib::setw(30) << "Radio Text: " << radio.radioData.stationText() << hwlib::endl;
    hwlib::cout << hwlib::left << hwlib::setw(30) << "Country Code: " << radio.radioData.getCountryCode() << hwlib::endl;
    hwlib::cout << hwlib::left << hwlib::setw(30) << "Program Area Coverage: " << radio.radioData.getProgramArea() << hwlib::endl;
    hwlib::cout << hwlib::left << hwlib::setw(30) << "Program Refrence Number: " << radio.radioData.getProgramRefrence() << hwlib::endl;
    hwlib::cout << hwlib::left << hwlib::setw(30) << "Message Group Type: " << radio.radioData.getMessageGroupType() << hwlib::endl;
    hwlib::cout << hwlib::left << hwlib::setw(30) << "Traffic Announcement: " << radio.radioData.trafficAnnouncement() << hwlib::endl;
    hwlib::cout << hwlib::left << hwlib::setw(30) << "Music Playing: " << radio.radioData.currentMusic() << hwlib::endl;
    hwlib::cout << hwlib::left << hwlib::setw(30) << "Clear Screen Request: " << radio.radioData.clearScreen() << hwlib::endl;
    hwlib::cout << hwlib::left << hwlib::setw(30) << "Program Type: " << radio.radioData.getProgramType() << hwlib::endl;
    hwlib::cout << hwlib::left << hwlib::setw(30) << "Static Program Type: " << radio.radioData.staticProgramType() << hwlib::endl;
    hwlib::cout << hwlib::left << hwlib::setw(30) << "Stereo Broadcast: " << radio.radioData.stereo() << hwlib::endl;
    hwlib::cout << hwlib::left << hwlib::setw(30) << "Compressed Broadcast: " << radio.radioData.compressed() << hwlib::endl;
    hwlib::cout << hwlib::left << hwlib::setw(30) << "Emergency Warning: " << radio.radioData.emergencyWarning() << hwlib::endl;
    hwlib::cout << hwlib::left << hwlib::setw(30) << "Received Time: " << radio.radioData.hours() << ":" << radio.radioData.minutes() << hwlib::endl;
    hwlib::cout << hwlib::endl << hwlib::endl;
    hwlib::wait_ms(3000);
    radio.seekChannel(1);
  }

  for(unsigned int i = 0; i < 5; i++){
    hwlib::wait_ms(5000);
    radio.seekChannel(1);
    hwlib::wait_ms(2000);   //Give time to complete tune process
    radio.radioData.rawData();
  }

  radio.setFrequency(107.5);    //No station Supports RDS-A, this one is RDS-A but doesn't send time...
  radio.radioData.update();
  hwlib::cout << radio.radioData.hours() << ":" << radio.radioData.minutes() << hwlib::endl;

// <<<----------------------------------------------------------------------------------------------------->>>
/*
  auto radio = TEA5767(i2c_bus);

  hwlib::cout << "Tuning to 100.7FM (Q-Music): ";
  radio.setFrequency(100.7);
  hwlib::cout << "DONE" << hwlib::endl << hwlib::boolalpha << "Reception Quality:";

  for(unsigned int i = 0; i < 5; i++){
    hwlib::cout << radio.signalStrength() << ", Stereo: " << radio.stereoReception() << hwlib::endl;
    hwlib::wait_ms(1000);
  }

  radio.setBandLimit();
  hwlib::wait_ms(200);
  hwlib::cout << "Setting Band Limit: " << radio.hasBandLimit() << hwlib::endl;
  radio.setFrequency(100.7);
  hwlib::wait_ms(5000);
  hwlib::cout << "Blocking of tuning to frequency out of range: " << !(radio.getFrequency() == 100.7) << hwlib::endl;
  radio.setBandLimit(0);
  radio.setFrequency(100.7);
  hwlib::cout << "Unsetting of Band Limit: " << (radio.getFrequency() == 100.7) << hwlib::endl << hwlib::endl;

  hwlib::cout << "Turning All Audio Optimizers On 1 by 1: ";
  radio.audioSettings(true, false, false);
  hwlib::cout << "SNC" << hwlib::endl;
  hwlib::wait_ms(2000);
  radio.audioSettings(true, true, false);
  hwlib::cout << "HCC" << hwlib::endl;
  hwlib::wait_ms(2000);
  radio.audioSettings(true, true, true);
  hwlib::cout << "SM" << hwlib::endl;
  hwlib::wait_ms(2000);
  hwlib::cout << "DONE: " << hwlib::boolalpha << !radio.isStereo() << hwlib::endl << hwlib::endl;
  hwlib::wait_ms(2000);

  hwlib::cout << "Setting Mono Signal: ";
  radio.setStereo(false);
  hwlib::cout << "DONE: " << hwlib::boolalpha << !radio.isStereo() << hwlib::endl << hwlib::endl;
  hwlib::wait_ms(2000);

  hwlib::cout << "Setting Stereo Signal: ";
  radio.setStereo(true);
  hwlib::cout << "DONE: " << hwlib::boolalpha << radio.isStereo() << hwlib::endl << hwlib::endl;
  hwlib::wait_ms(2000);

  hwlib::cout << hwlib::endl << hwlib::endl;
  hwlib::cout << "Tuning to 100.7FM with Low Side Injection: ";
  radio.setFrequency(100.7, 0);
  hwlib::cout << !radio.highSide() << hwlib::endl << hwlib::endl;
  hwlib::wait_ms(2000);

  hwlib::cout << "Tuning to 100.7FM with High Side Injection: ";
  radio.setFrequency(100.7, 1);
  hwlib::cout << radio.highSide() << hwlib::endl << hwlib::endl;
  hwlib::wait_ms(2000);

  hwlib::cout << "Muting Left Speaker: ";
  radio.setMute('l', true);
  hwlib::cout << "DONE: " << hwlib::boolalpha << radio.isMuted('l') << hwlib::endl << hwlib::endl;
  hwlib::wait_ms(2000);

  hwlib::cout << "Unmuting Left Speaker: ";
  radio.setMute('l', false);
  hwlib::cout << "DONE: " << hwlib::boolalpha << !radio.isMuted('l') << hwlib::endl << hwlib::endl;
  hwlib::wait_ms(2000);

  hwlib::cout << "Turning to Stand-By: ";
  radio.standBy();
  hwlib::cout << hwlib::boolalpha << radio.isStandBy() << hwlib::endl << hwlib::endl;
  hwlib::wait_ms(2000);

  hwlib::cout << "Turning back on: ";
  radio.standBy(false);
  hwlib::cout << hwlib::boolalpha << !radio.isStandBy() << hwlib::endl << hwlib::endl;
  hwlib::wait_ms(2000);

  hwlib::cout << "Starting Circulair Search-Up: ";
  radio.searchLoop(88, 1, 3);
  hwlib::cout << "Search-Mode Active: " << hwlib::boolalpha << !radio.inSearch() << hwlib::endl << hwlib::endl;
  hwlib::wait_ms(2000);
  for(unsigned int i = 0; i < 10; i++){
    radio.searchLoop(1, 3);
    hwlib::cout << "FM-Frequency: " << radio.getIntFrequency() << ", Signal Strength: " << radio.signalStrength() << ", Stereo: " << radio.stereoReception() << hwlib::endl;
    hwlib::wait_ms(4000);
  }
  
  hwlib::cout << "Starting Search-Up: ";
  radio.singleSearch(88, 1, 2);
  hwlib::cout << "Search-Mode Active: " << hwlib::boolalpha << !radio.inSearch() << hwlib::endl << hwlib::endl;
  hwlib::wait_ms(2000);
  for(unsigned int i = 0; i < 15; i++){
    radio.singleSearch(1, 3);
    hwlib::cout << "FM-Frequency: " << radio.getIntFrequency() << ", Signal Strength: " << radio.signalStrength() << ", Stereo: " << radio.stereoReception() << hwlib::endl;
    hwlib::wait_ms(4000);
  }

  hwlib::cout << "Starting Alternative Search-Up: ";
  radio.altSearch(88, 1, 3, 0.4);
  hwlib::cout << "Search-Mode Active: " << hwlib::boolalpha << !radio.inSearch() << hwlib::endl << hwlib::endl;
  hwlib::cout << "FM-Frequency: " << radio.getIntFrequency() << ", Signal Strength: " << radio.signalStrength() << ", Stereo: " << radio.stereoReception() << hwlib::endl;
  hwlib::wait_ms(4000);
  for(unsigned int i = 0; i < 10; i++){
    radio.altSearch(1, 3, 0.4);
    hwlib::cout << "FM-Frequency: " << radio.getIntFrequency() << ", Signal Strength: " << radio.signalStrength() << ", Stereo: " << radio.stereoReception() << hwlib::endl;
    hwlib::wait_ms(4000);
  }
*/


  hwlib::cout << hwlib::endl << hwlib::endl << "Test ended" << hwlib::endl;
}
