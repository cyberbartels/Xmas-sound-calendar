#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <MFRC522.h>
#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>

#include "pictures.h"
#include "chars.h"
#include "text.h"

//OLED screen
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1  // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C

//MFRC522
#define RST_PIN 9
#define SS_PIN 10

//DFPlayer
#define busyPin 4

#define xmasday 24
#define numMusicTracks 24
//Reference pictures
#define snow_man_0 0
#define snow_man_1 1
#define santa_sledge_0 2
#define santa_sledge_1 3
#define xmas_tree 4
#define tune_0 5
#define tune_1 6
#define insert_card 7
#define lotta 8
#define fox 9

/*
   Which daily track to play
   100 = Random music
     1 = CD 1
     2 = CD 2
   ...
*/
byte dailyTrackMode = 100;  //default to random music

//Whole lot of peripheral devices
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
SoftwareSerial mySoftwareSerial(2, 3);  // RX, TX // DFPlayer Mini
DFRobotDFPlayerMini mp3DFPlayer;

void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(0));  //Some random seed from open input

  /*
     Busy Pin @ MP3 Player
  */
  pinMode(busyPin, INPUT);

  /*
     OLED
  */
  //initialize OLED display with the I2C addr 0x3C
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }
  Serial.println(F("OLED started"));
  showGreeting();

  /*
     MP3 Player
  */
  mySoftwareSerial.begin(9600);
  Serial.println(F("Initializing DFPlayer ... May take 3~5 seconds"));
  delay(1000);
  if (!mp3DFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck connection!"));
    Serial.println(F("2.Please insert SD card!"));
    while (true) {
      delay(0);  // Code to compatible with ESP8266 watch dog.
    }
  }
  Serial.println(F("DFPlayer Mini online."));

  mp3DFPlayer.setTimeOut(500);  //Set serial communictaion time out 500ms
  mp3DFPlayer.volume(10);       //Set volume value. From 0 to 30

  mp3DFPlayer.playFolder(99, 1);  //Play greeting mp3
  delay(2000);                    // Wait so that the mp3 player can start
  do {                            //Wait 'til the little tune finished playing
    Serial.println(F("mp3 still playing..."));
    delay(1000);
  } while (isPlaying());

  /*
    RFID Reader
  */
  SPI.begin();                                   // Init SPI bus
  mfrc522.PCD_Init();                            // Init MFRC522 card
  mfrc522.PCD_DumpVersionToSerial();             // Show details of PCD - MFRC522 Card Reader details
  Serial.println("Read data on a MIFARE PICC");  //shows in serial that it is ready to read

  /*
    Ready to go       
  */
  showText(0, 1); //("Insert", "Card");
  delay(3000);
}

void showGreeting() {
  display.clearDisplay();
  showText(2, 3);//("Starting", "XMAS Box");
  delay(3000);
  displayPic(xmas_tree);
}

void loop() {
  displayPic(insert_card);
 // Serial.println(F("In loop"));
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Show some details of the PICC (that is: the tag/card)
  Serial.print(F("Card UID:"));
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println();
  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));

  // Check for compatibility
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI
      && piccType != MFRC522::PICC_TYPE_MIFARE_1K
      && piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("This sample only works with MIFARE Classic cards."));
    return;
  }

  // Read the code on the card/tag/...
  byte code = readRFIDCard();
  Serial.print(F("Code: "));
  Serial.println(code, DEC);

  // Handle card code: It indicates either a command or a day before XMas
  if (code > xmasday) {
    handleCommand(code);
  } else {
    handleDay(code);
  }
}

void handleCommand(byte code) {
//  Serial.print(F("Code: "));
//  Serial.println(code, DEC);
  switch (code) {
    case 100:
      dailyTrackMode = 100;
      break;
    case 101:
      dailyTrackMode = 1;
      break;
    case 102:
      dailyTrackMode = 2;
      break;
    case 103:
      dailyTrackMode = 3;
      break;
    case 99:
      dailyTrackMode = 100; //reset to random music on Xmas!
      playXMasSpecial();
      break;
    default:
      dailyTrackMode = 100;
      break;
  }
  showTrackMode();
}

void showTrackMode() {
  Serial.print(F("dailyTrackMode: "));
  Serial.println(dailyTrackMode, DEC);
  display.clearDisplay();
  switch (dailyTrackMode) {
    case 100:
      display.println(F("Random Music"));
      break;
    case 1:
      display.println(F("CD 1"));
      break;
    case 2:
      display.println(F("CD 2"));
      break;
    case 3:
      display.println(F("CD 3"));
      break;
    default:
      dailyTrackMode = 100;
      display.println(F("Random Music"));
      break;
  }

  delay(1000);
  mp3DFPlayer.playFolder(98, dailyTrackMode);  //98 = Table of contents
  playCDAnimation();
  display.clearDisplay();
}

void playXMasSpecial() {
  mp3DFPlayer.playFolder(90, 1); //Lukas
  animate(bmp_snowflake, SNOWFLAKE_WIDTH, SNOWFLAKE_HEIGHT, NUM_SNOWFLAKES);
  displayPic(xmas_tree);
  delay(5000);
  playRandomMusic();
}

void handleDay(byte day) {
  loadXMas(day);
  playDailyCountdown(day);
  switch (dailyTrackMode) {
    case 100:
      //Random music
      playRandomMusic();
      break;
    default:
      playDailyCDTrack(day);
      break;
  }
}

void playDailyCountdown(byte day) {
  int loopCounter = 0;
  byte pic_0;
  byte pic_1;
  mp3DFPlayer.playFolder(day, 100);  //countdown mp3
  delay(2000); //let mp3 player start
  loopCounter = 0;
  if (day == xmasday) {
    pic_0 = santa_sledge_0;
    pic_1 = santa_sledge_1;
  } else {
    pic_0 = snow_man_0;
    pic_1 = snow_man_1;
  }
  do {
    Serial.println(F("mp3 still playing..."));
    if (loopCounter % 2 == 0) {
      displayPic(pic_0);
    }

    if (loopCounter % 2 == 1) {
      displayPic(pic_1);
    }
    loopCounter++;
    delay(500);
  } while (isPlaying());
  display.clearDisplay();
}

void playRandomMusic() {
  byte track = random(1, numMusicTracks + 1);
  mp3DFPlayer.playFolder(80, track);  //80 = music mp3
  playCDAnimation();
  display.clearDisplay();
}

void playDailyCDTrack(byte day) {
  mp3DFPlayer.playFolder(day, dailyTrackMode);
  playCDAnimation();
}

void playCDAnimation() {
  switch (dailyTrackMode) {
    case 100:
      animate(bmp_note, NOTE_WIDTH, NOTE_HEIGHT, NUM_NOTES);
      break;
    case 1:
      animate(bmp_triple_exclamation, THREE_EXCL_WIDTH, THREE_EXCL_HEIGHT, NUM_THREE_EXCL);
      break;
    case 2:
      animate(bmp_fox, FOX_WIDTH, FOX_HEIGHT, NUM_FOX);
      break;
    case 3:
      animate(bmp_lotta, LOTTA_WIDTH, LOTTA_HEIGHT, NUM_LOTTA);
      break;
    default:
      dailyTrackMode = 100;
      animate(bmp_note, NOTE_WIDTH, NOTE_HEIGHT, NUM_NOTES);
      break;
  }
}

void loadXMas(byte day) {
  mp3DFPlayer.playFolder(99, 3);  // Church bells

  int halfLoaderHeight = 6;
  int displayMiddle = display.height() / 2;
  int stepWidth = (display.width() / xmasday) - 1;

  showText(2, 4);//("Loading", "XMAS");
  delay(1000);

  //Draw progress bar
  display.drawRect(0, displayMiddle - halfLoaderHeight, display.width(), 2 * halfLoaderHeight, SSD1306_WHITE);
  display.display();

  delay(1000);

  //Progress the progress bar
  for (int i = 0; i < day; i++) {
    int x0 = i * stepWidth + (i + 1);
    //int x1 = (i + 1) * stepWidth + i - 1;
    display.fillRect(x0, displayMiddle - halfLoaderHeight, stepWidth - 1, 2 * halfLoaderHeight, SSD1306_WHITE);
    display.display();

    delay(250 + 40 * (xmasday - day));  //Shorter steps each day....
  }

  mp3DFPlayer.pause();
  do {  //Wait 'til player stopped playing
    Serial.println(F("mp3 still playing..."));
    delay(400);
  } while (isPlaying());

  if (day != xmasday) {
    showText(5, 6);//("***Critical Error***", "NO XMAS");
  } else {
    showText(7, 8);//("Success: XMAS", "LOADED");
  }

  delay(3000);
  display.clearDisplay();
}

byte readRFIDCard() {

  /*
     Create key
  */
  MFRC522::MIFARE_Key key;
  // Prepare the key (used both as key A and as key B)
  // using FFFFFFFFFFFFh which is the default at chip delivery from the factory
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  // Use the second sector,
  // that is: sector #1, covering block #4 up to and including block #7
  byte sector = 1;
  byte blockAddr = 4;
  //  byte dataBlock[]    = {
  //    0x01, 0x02, 0x03, 0x04, //  1,  2,   3,  4,
  //    0x05, 0x06, 0x07, 0x08, //  5,  6,   7,  8,
  //    0x09, 0x0a, 0xff, 0x0b, //  9, 10, 255, 11,
  //    0x0c, 0x0d, 0x0e, 0x0f  // 12, 13, 14, 15
  //  };
  byte trailerBlock = 7;
  MFRC522::StatusCode status;
  byte buffer[18];
  byte size = sizeof(buffer);

  // Authenticate using key A
  Serial.println(F("Authenticating using key A..."));
  status = (MFRC522::StatusCode)mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // DEBUG Show the whole sector as it currently is
  //  Serial.println(F("Current data in sector:"));
  //  mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
  //  Serial.println();

  // Read data from the block
  Serial.print(F("Reading data from block "));
  Serial.print(blockAddr);
  Serial.println(F("..."));
  status = (MFRC522::StatusCode)mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Read() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
  }
  Serial.print(F("Data in block "));
  Serial.print(blockAddr);
  Serial.println(F(":"));
  dump_byte_array(buffer, 16);
  Serial.println();
  Serial.println();

  // Halt PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();

  return buffer[0];
}

bool isPlaying() {
  return !digitalRead(busyPin);
}

void showText(byte upperTextIndex, byte lowerTextIndex) {
  //Textbuffer
  //Must contain up to 20 characters
  char upperBuffer[20];
  char lowerBuffer[20];

  strcpy_P(upperBuffer, (char *)pgm_read_word(&(string_table[upperTextIndex])));
  Serial.println(upperBuffer);

  strcpy_P(lowerBuffer, (char *)pgm_read_word(&(string_table[lowerTextIndex])));
  Serial.println(lowerBuffer);

  byte upperPixels = 6 * strlen(upperBuffer) <= display.width() ? 6 * strlen(upperBuffer) : display.width();    //Assume 6 Pixel per character in font size 1
  byte lowerPixels = 12 * strlen(lowerBuffer) <= display.width() ? 12 * strlen(lowerBuffer) : display.width();  //Assume 12 Pixel per character in font size 1
  byte spacingUpper = (display.width() - upperPixels) / 2;                                                      //left spacing
  byte spacingLower = (display.width() - lowerPixels) / 2;                                                      //left spacing

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(spacingUpper, 5);
  display.println(upperBuffer);
  display.setCursor(spacingLower, 45);
  display.setTextSize(2);
  display.println(lowerBuffer);
  display.display();
}

void displayPic(byte picNumber) {
  display.clearDisplay();

  if (picNumber == santa_sledge_0) {
    // Display bitmap
    display.drawBitmap(0, 2, epd_bitmap_santa_0, 128, 60, WHITE);
    display.display();
  }
   if (picNumber == santa_sledge_1) {
    // Display bitmap
    display.drawBitmap(0, 2, epd_bitmap_santa_1, 128, 60, WHITE);
    display.display();
  }
  if (picNumber == xmas_tree) {
    // Display bitmap
    display.drawBitmap(0, 0, epd_bitmap_christmas_tree, 128, 64, WHITE);
    display.display();
  }
  if (picNumber == snow_man_0) {
    // Display bitmap
    display.drawBitmap(0, 0, epd_bitmap_snowman_0, 128, 64, WHITE);
    display.display();
  }
  if (picNumber == snow_man_1) {
    // Display bitmap
    display.drawBitmap(0, 0, epd_bitmap_snowman_1, 128, 64, WHITE);
    display.display();
  }
  //  if (picNumber == tune_0) {
  //    // Display bitmap
  //    display.drawBitmap(0, 0,  epd_bitmap_tune_0, 128, 64, WHITE);
  //    display.display();
  //  }
  //  if (picNumber == tune_1) {
  //    // Display bitmap
  //    display.drawBitmap(0, 0,  epd_bitmap_tune_1, 128, 64, WHITE);
  //    display.display();
  //  }
  // if (picNumber == lotta) {
  //   // Display bitmap
  //   display.drawBitmap(0, 0, epd_bitmap_lotta, 128, 64, WHITE);
  //   display.display();
  // }
  if (picNumber == insert_card) {
    // Display bitmap
    display.drawBitmap(0, 0, epd_bitmap_insert_card, 128, 64, WHITE);
    display.display();
  }
}

#define XPOS 0  // Indexes into the 'icons' array in function below
#define YPOS 1
#define DELTAY 2

void animate(const uint8_t *bitmap, uint8_t w, uint8_t h, uint8_t num) {
  int8_t f, icons[num][3];

  // Initialize 'snowflake' positions
  for (f = 0; f < num; f++) {
    icons[f][XPOS] = random(1 - w, display.width());
    icons[f][YPOS] = -h;
    icons[f][DELTAY] = random(1, 6);
    Serial.print(F("x: "));
    Serial.print(icons[f][XPOS], DEC);
    Serial.print(F(" y: "));
    Serial.print(icons[f][YPOS], DEC);
    Serial.print(F(" dy: "));
    Serial.println(icons[f][DELTAY], DEC);
  }

  delay(2000); //let mp3 player some time to start

  do {                       // Loop while CD player is playing...
    display.clearDisplay();  // Clear the display buffer

    // Draw each Bitmap:
    for (f = 0; f < num; f++) {
      display.drawBitmap(icons[f][XPOS], icons[f][YPOS], bitmap, w, h, WHITE);
    }

    display.display();  // Show the display buffer on the screen
    delay(200);         // Pause for 1/10 second

    // Then update coordinates of each flake...
    for (f = 0; f < num; f++) {
      icons[f][YPOS] += icons[f][DELTAY];
      // If snowflake is off the bottom of the screen...
      if (icons[f][YPOS] >= display.height()) {
        // Reinitialize to a random position, just off the top
        icons[f][XPOS] = random(1 - w, display.width());
        icons[f][YPOS] = -h;
        icons[f][DELTAY] = random(1, 6);
      }
    }
  } while (isPlaying());

  display.clearDisplay();
}


void printMP3PlayerDetail(uint8_t type, int value) {
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerUSBInserted:
      Serial.println(F("USB Inserted!"));
      break;
    case DFPlayerUSBRemoved:
      Serial.println(F("USB Removed!"));
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

/**
   Helper routine to dump a byte array as hex values to Serial.
*/
void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void dump_byte_array_dec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    //   Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.println(buffer[i], DEC);
    if (buffer[i] == 3)
      Serial.println("Tres");
  }
}
