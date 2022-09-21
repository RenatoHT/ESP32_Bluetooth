// this header is needed for Bluetooth Serial -> works ONLY on ESP32
#include "BluetoothSerial.h" 

// init Class:
BluetoothSerial ESP_BT; 

// Parameters for Bluetooth interface
int incoming;
//est

//Tela OLED
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//Array population parameters
const int columns = 7;     //6 switches + 1 tempo
const int rows = 9;        //max sequences possible to store
int qntTot=0;
int qntAtt=0;
int sequenceArray[rows][columns];


void setup() {
  Serial.begin(9600);
  ESP_BT.begin("WemosLolin32"); //Name of your Bluetooth interface -> will show up on your phone

//Inicializa tela oled
 // Start I2C Communication SDA = 5 and SCL = 4 on Wemos Lolin32 ESP32 with built-in SSD1306 OLED
  Wire.begin(5, 4);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000); // Pause for 2 seconds
 
  // Clear the buffer.
  display.clearDisplay();
  
  // Display on the screen
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 20);
  display.print("Stand By");
  display.display(); 

}

//Array Button ON/OFF -> Ordering and Verifying if command received is complete 
void setOnOff(int num){
  int button = floor(num/10);
  int pos = button-1;
  sequenceArray[qntAtt][pos] = num;
}

//Serial.print for Array *not used*
void printArray(int auxArray[][columns]){
  for (int i = 0; i < qntTot;){
    Serial.println("Sequence " + String(i+1));
    for (int j = 0; j < columns;) {
      Serial.print(String(auxArray[i][j]) + " - " );
      j+=1;
    }
    Serial.println();
    i+=1;    
  }
}

//Reset all values
void resetAll() {

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 20);
  display.print("Stand By");
  display.display(); 
  
  for(int i = 0; i < qntTot; i++) {
    memset(sequenceArray[i], 0, sizeof(sequenceArray[i]));
  }

  qntTot = 0;
  qntAtt = 0;
}

//Execute saved arrays
void executeArray(int array2D[rows][columns]){
  for (int i = 0; i < qntTot;){
    Serial.println("Executing: " + String(i+1));
    
    int seq = i+1;
    ESP_BT.print(seq);

    
    for (int j = 0; (j < (columns-1)); j++) {
      int button = floor(array2D[i][j]/10);    
      int stateOnOff = array2D[i][j]%10;      

      switch (button){
      case 1:
        display.clearDisplay();
        display.setCursor(0, 20);
        display.print(String(stateOnOff));
        display.display(); 
        break;
      
      case 2:
        display.setCursor(20, 20);
        display.print(String(stateOnOff));
        display.display();
        break;

      case 3:
        display.setCursor(40, 20);
        display.print(String(stateOnOff));
        display.display();
        break;
      
      case 4:
        display.setCursor(60, 20);
        display.print(String(stateOnOff));
        display.display();
        break;  
      
      case 5:
        display.setCursor(80, 20);
        display.print(String(stateOnOff));
        display.display();
        break;
      
      case 6:
        display.setCursor(100, 20);
        display.print(String(stateOnOff));
        display.display();
        break;

      default:
        Serial.println("Error, button not assigned");
        display.clearDisplay();
        display.print("ERROR");
        display.display(); 

        break;
      }
    }
    
    //Timer to wait until next array is read/executed
    uint32_t duraMin = array2D[i][columns-1] * 1000L;    //Tempo *1000L = 1 Seg*
    for ( uint32_t timerStart = millis(); millis()-timerStart < duraMin;) {
      
      //If during wait time another command is received
      if (ESP_BT.available()) {
        incoming = ESP_BT.read();

        //Stop excuting and reset
        if(incoming == 0) {
          resetAll();
          ESP_BT.print(90);
          Serial.println("Resetting");
          return;
        }

        //Send "occupied"
        else if (incoming != 0) {
          ESP_BT.print(95);
        }
      }
    }
    Serial.println();

    i+=1;  
   }
  ESP_BT.print(99);  //Send "finished all tasks"
  resetAll();
}


void loop() {
  
  // -------------------- Receive Bluetooth signal ----------------------
  if (ESP_BT.available()) {
   
    incoming = ESP_BT.read(); //Read what we receive 

    if(incoming == 0) {
      resetAll();
      Serial.println("Resetting");
    }

    //Qnt of commands
    else if (incoming < 10) {
      qntTot = incoming;
      Serial.println("Expected commands: " + String(qntTot));
    }
    
    //Buttons
    else if ((incoming >= 10) && (incoming < 100)){
        setOnOff(incoming);
    }

    //Time
    else if (incoming > 100){     
        int minutes = incoming-100;
        sequenceArray[qntAtt][columns-1] = minutes;
        qntAtt+=1;
        if((qntAtt == qntTot) && (qntTot != 0)){
            printArray(sequenceArray);
            executeArray(sequenceArray);
          
        }
    }
  }
}

