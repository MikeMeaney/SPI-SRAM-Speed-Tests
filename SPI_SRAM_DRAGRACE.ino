
//SPI SRAM DRAG RACE
//Mike Meaney 2014

#include <SPI.h>


//SPI SRAM Stuff -----------------------------------------------------------
#define SS_PIN 2
SPISettings SRAMsettings(2000000, MSBFIRST, SPI_MODE0);  //SPI SRAM works only in MODE0, BLE is MODE1. <--The Problem
                                                         //Gaol to put both on same SPI bus.
//As per the Microchip Data Sheet
const uint8_t WRITE_BYTE = B00000010; // This must be sent to trigger SRAM IC to store data
const uint8_t READ_BYTE =  B00000011; // This must be sent to trigger SRAM IC to recite data
	//Status Register constatants
const uint8_t SEQ_MODE =   B01000001; 
const uint8_t BYTE_MODE =  B00000001;
const uint8_t RDSR = B00000101;
const uint8_t WSRS = B00000001;

//In a uinverse var, var away...
uint32_t startOfTest = 0;
uint32_t endOfTest = 0;
uint32_t testResult = 0;

void setup() {

	Serial.begin(115200);
	SPI.begin();
	while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  	}

	//Feg it. Testing char array into String



  Serial.println("Coded Wear SPI SRAM Drag Race");
  Serial.println();
  //Initate SPI SRAM (32KB of SRAM FUN!)
  //SS on the SPI SRAM must be set as an output
  pinMode(SS_PIN, OUTPUT);

  //Set SRAM to byte mode
  Serial.println(" Setting SPI SRAM to opperate in BYTE_MODE ");
  Serial.println();
  extSRAM_setStatusRegister(BYTE_MODE);
  uint8_t  statReg = extSRAM_readStatusRegister();
  Serial.print(" SRAM Status Register is: " );
  Serial.println( statReg, BIN );
  Serial.println(" Starting Byte Write Test  ");
  Serial.println();


  //Test and Time Byte mode write
  startOfTest = millis();
  for(uint16_t w = 0; w < 0x7FFF; w++){
  	//uint8_t testByte = int(random(0, 255));
  	extSRAM_writeByte(w, w);
  }
  endOfTest = millis();
  testResult = endOfTest - startOfTest;
  Serial.println(" Ending Byte Write Test  ");
  Serial.print("Result: ");
  Serial.println(testResult);
  Serial.println("------------");
   
  //Test and Time Byte mode read
  Serial.println(" Starting Byte Read Test  ");
  startOfTest = millis();
  
  for(uint16_t w = 0; w < 0x7FFF; w++){
  	uint8_t theByte = extSRAM_readByte(w);
  	//Serial.println(theByte);
  }

  endOfTest = millis();
  testResult = endOfTest - startOfTest;
  Serial.println(" Ending Byte Read Test  ");
  Serial.print("Result: ");
  Serial.println(testResult);
  Serial.println("--------------------");

  //Set SRAM to sequential mode
  Serial.println(" Setting SPI SRAM to opperate in SEQ_MODE ");
  Serial.println();
  extSRAM_setStatusRegister(SEQ_MODE);
  statReg = extSRAM_readStatusRegister();
  Serial.print(" SRAM Status Register is: " );
  Serial.println( statReg, BIN );

  //Test and Time Sequential mode write
  Serial.println(" Starting Sequential Write Test  ");
  startOfTest = millis();
  //Start by intializing transaction
  SPI.beginTransaction(SRAMsettings);
  digitalWrite(SS_PIN, LOW);
  //Send it the instruction
  SPI.transfer(WRITE_BYTE);
  //Send it the starting address
  SPI.transfer(0);
  SPI.transfer(0); // Send zero twice to get 16 clock cycles

  //Send over the data
  for(uint16_t w = 0; w < 0x7FFF; w++){
  	//uint8_t testByte = int(random(0, 255)); // The random data
  	SPI.transfer(w);
  }
  digitalWrite(SS_PIN, HIGH);
  SPI.endTransaction();

  endOfTest = millis();
  testResult = endOfTest - startOfTest;
  Serial.println(" Ending Sequential Write Test  ");
  Serial.print("Result: ");
  Serial.println(testResult);
  Serial.println("------------");
  
  //Test and Time Seuential mode read
  Serial.println(" Starting Sequential Read Test  ");
  startOfTest = millis();
  //Start by intializing transaction
  SPI.beginTransaction(SRAMsettings);
  digitalWrite(SS_PIN, LOW);
  //Send it the instruction
  SPI.transfer(READ_BYTE);
  //Send it the starting address
  SPI.transfer(0);
  SPI.transfer(0); // Send zero twice to get 16 clock cycles

  //Send over the data
  for(uint16_t w = 0; w < 0x7FFF; w++){
  	uint8_t theByte = SPI.transfer(0); //Keep clocking out 0's as long as there is data to send
  	//Serial.println(theByte);
  }
  digitalWrite(SS_PIN, HIGH);
  SPI.endTransaction();

  endOfTest = millis();
  testResult = endOfTest - startOfTest;
  Serial.println(" Ending Sequential Read Test  ");
  Serial.print("Result: ");
  Serial.println(testResult);
  Serial.println("------------");

  Serial.println(" Starting Sequential Varible Byte Read Test  ");
  startOfTest = millis();
  uint16_t dataLength = 1024;
  uint8_t theData[dataLength];
  extSRAM_sequentialRead(0x00, theData, dataLength);
  endOfTest = millis();
  Serial.println(" Ending Sequential Varible Byte Read Test  ");
  for(uint16_t i =0; i< dataLength; i++){
  	Serial.print(i);
    Serial.print(" | ");
    Serial.println(theData[i]);
  }
  Serial.print(" Result: ");
  Serial.println( endOfTest - startOfTest );
  //end test
}

void loop() {

}

// SPI SRAM Functions

//Set the status register with provided config
void extSRAM_setStatusRegister(uint8_t statusReg){

 //Begin SPI Transaction
  SPI.beginTransaction(SRAMsettings);
  digitalWrite(SS_PIN, LOW); // Pull CS Low to start
  uint8_t read_byte;
  SPI.transfer(WSRS);
  SPI.transfer(statusReg);
  digitalWrite(SS_PIN, HIGH); //Pull CS high to end
  SPI.endTransaction();
  //SPI Transaction Ended
}
//Read from the status register
uint8_t extSRAM_readStatusRegister(){
   uint8_t theStatusRegister;

 //Begin SPI Transaction
  SPI.beginTransaction(SRAMsettings);
  digitalWrite(SS_PIN, LOW); // Pull CS Low to start
  uint8_t read_byte;
  SPI.transfer(RDSR);
  theStatusRegister = SPI.transfer(0);
  digitalWrite(SS_PIN, HIGH); //Pull CS high to end
  SPI.endTransaction();
  //SPI Transaction Ended

  return theStatusRegister;
}

uint8_t extSRAM_sequentialRead(uint16_t address, uint8_t dataTarget[] ,uint16_t length) {
	SPI.beginTransaction(SRAMsettings);
	digitalWrite(SS_PIN, LOW);
	SPI.transfer(READ_BYTE);
	SPI.transfer((char)(address >> 8));
	SPI.transfer((char)(address));

	for(uint16_t r = 0; r < length; r++){
		//This is where data gets written to dataTarget
		dataTarget[r] = SPI.transfer(0);
	}
	digitalWrite(SS_PIN, HIGH);
	SPI.endTransaction();

	return 0;
}


uint8_t extSRAM_sequentialWrite(uint16_t address, uint8_t dataTarget[] ,uint16_t length) {
  SPI.beginTransaction(SRAMsettings);
  digitalWrite(SS_PIN, LOW);
  SPI.transfer(WRITE_BYTE);
  SPI.transfer((char)(address >> 8));
  SPI.transfer((char)(address));

  for(uint16_t r = 0; r < length; r++){
    //This is where data gets written to dataTarget
    SPI.transfer(dataTarget[r]);
  }
  digitalWrite(SS_PIN, HIGH);
  SPI.endTransaction();

  return 0;
}
//Read a byte at an address and return it's value (BYTE_MODE)
uint8_t extSRAM_readByte(uint16_t address){
 
 //Begin SPI Transaction
  SPI.beginTransaction(SRAMsettings);
  digitalWrite(SS_PIN, LOW); // Pull CS Low to start
  uint8_t read_byte;
  SPI.transfer(READ_BYTE);
  SPI.transfer((char)(address >> 8));
  SPI.transfer((char)(address));
  read_byte = SPI.transfer(0);  
  digitalWrite(SS_PIN, HIGH); //Pull CS high to end
  SPI.endTransaction();
  //SPI Transaction Ended
  
 return read_byte;
}

//Write a byte to an address. return the written value (BYTE_MODE)
uint8_t extSRAM_writeByte(uint16_t address, uint8_t data){ 
  //Begin SPI Transaction
  SPI.beginTransaction(SRAMsettings);
  digitalWrite(SS_PIN, LOW); // Pull CS Low to start
  SPI.transfer(WRITE_BYTE);
  SPI.transfer((char)(address>> 8));
  SPI.transfer((char)(address));
  SPI.transfer(data);
  digitalWrite(SS_PIN, HIGH); //Pull CS high to end
  SPI.endTransaction();
  //SPI Transaction ended

  //Return a read of the newly written value
  return extSRAM_readByte(address);
}