
//  added: Retraction of Filament

//  added: millis instead delay() 
//    https://learn.adafruit.com/multi-tasking-the-arduino-part-1/using-millis-for-timing


//  added: 4-button keypad
//    https://it.aliexpress.com/item/8pcs-Push-Button-Switch-4-Keyboard-Module-key-Board-Keypad-for-Arduino-DUE-Breadboard-Leonardo-ZERO/32281455411.html

//  added: 4-digit Display
//    https://it.aliexpress.com/item/4-Four-Digital-Display-With-Adjustable-Brightness-LED-Module-Clock-Point-Accessories-Blocks-for-arduino-DIY/32315750039.html

//  added: potentiometer to regulate 'fast' (instead of SPEED that is a constant)


//#define DEBUG 1   //  comment out this line to not show debug print lines
                    //  !!! program works but comments makes some problems
                    //      (todo: shorten console comments)

                    
// Define key pin
int K1 = 13;
int K2 = 12;
int buttonState = 0; // variable for reading the pushbutton status
int buttonVal = 0;

const  int     maxSpeed = 200;  //  max extruder speed
const  int     minSpeed =  20;  //  minimum extruder speed
const  int maxRetractionSpeed = maxSpeed;

  int    speedStep =   5;  //  extruder speed increment
  int         once = false; // set initially FALSE
  int        inPin =   2; // (D2) Digital 2 - choose the input pin (from KUKA + low level voltage regulator)
  int        state = LOW; // variable for reading the pin status
  int          pot =   0;
  int         fast =   0; //  "speed" token was already taken !
  float      kappa =  50;
// const int SPEED = 200;
const int INCREMENT=   1;  // Extrude Quantity
const int   POTPIN =  A0; // read Potentiometer value from Analog pin 0

unsigned int val = INCREMENT;  //  how much extrude?
boolean previousRetraction = false;

//  http://www.techmonkeybusiness.com/tm1637-4-digit-display-example-sketch.html

#include <TM1637Display.h>

const int CLK = 9; //Set the CLK pin connection to the display
const int DIO = 8; //Set the DIO pin connection to the display

TM1637Display display(CLK, DIO);  //set up the 4-Digit Display.


long previousMillis = 0;        // will store last time LED was updated



void setup() {
  Serial.begin(115200);   // must be the same as Marlin firmware on extruder
  
  pinMode(inPin, INPUT);    // declare pushbutton as input
    
  Serial.println("G92 E0");  // http://reprap.org/wiki/Gcode (reset Extruder Position)

  display.setBrightness(0x0a);  //set the diplay to maximum brightness

//Activate key pin internal pull-up resistors
  pinMode(K1, INPUT_PULLUP);
  pinMode(K2, INPUT_PULLUP);

}


void isButtonPressed(){
   
  buttonState = digitalRead(K1);
  if(!buttonState){
      #ifdef DEBUG
      Serial.print("k1" );
      delay(1);        // delay in between reads for stability
      #endif
    buttonVal+=speedStep;
  }
  do
  {
  buttonState = digitalRead(K1);
  }
  while(!buttonState);//Wait button release
   
  buttonState = digitalRead(K2);
  if(!buttonState){
      #ifdef DEBUG
      Serial.print("k2" );
      delay(1);        // delay in between reads for stability
      #endif
      buttonVal-=speedStep;
  }
  do
  {
  buttonState = digitalRead(K2);
  }
  while(!buttonState);//Wait button release
   
}


void readPot(){
      // read the input on analog potentiometer pin
      pot = analogRead(POTPIN);

      #ifdef DEBUG
      // print out the value you read:
      Serial.print(pot);
      Serial.print("\t-\t");
      delay(1);        // delay in between reads for stability
      #endif
      
      // scale potentiometer value to regulate the Extruder
      fast = map(pot, 200, 700, minSpeed, maxSpeed);   

      #ifdef DEBUG
      Serial.print(fast);
      Serial.print("\t_  ");
      delay(1);        // delay in between reads for stability
      #endif

      isButtonPressed();  //  -----------------------------------------------------
      
      fast+=buttonVal;
      if(fast > maxSpeed) {
        fast = maxSpeed;
        buttonVal-=speedStep;    //  altrimenti il valore continua ad incrementarsi
      }
      if(fast < minSpeed) {
        fast = minSpeed;
        buttonVal+=speedStep;
      }
      
      #ifdef DEBUG
      Serial.print(fast);
      Serial.print("\n");
      delay(1);        // delay in between reads for stability
      #endif
      display.showNumberDec(fast);            // Display the Extrusion Speed;

}


void loop(){

  state = digitalRead(inPin);  // read KUKA pin 

  if (state == HIGH) {         // check if the input is HIGH
      readPot();

  // check to see if it's time to send another extrude code; that is, if the 
  // difference between the current time and last time extruded
  // is bigger than the interval at which you want to 
  
  unsigned long currentMillis = millis();
       #ifdef DEBUG
      Serial.print("\tK= ");
      Serial.print(kappa);
      Serial.print("\tCurr ");
      Serial.print(currentMillis);
      Serial.print("\tPrev ");
      Serial.print(previousMillis);
      
      Serial.println("\t-\t");
      delay(1);        // delay in between reads for stability
      #endif

      //  float regulator;
      float kappa = 60000 * INCREMENT/fast; //

   if(currentMillis - previousMillis > kappa) {
     // save the last time sent extrude gcode 
     previousMillis = currentMillis;   
 
      #ifdef DEBUG
      Serial.print("\tkappa= ");
      Serial.print(kappa);
      Serial.print("\t-\t");
      delay(1);        // delay in between reads for stability
      #endif


//  - - - - - - - - - - ( RESTORE from RETRACTION ) - - - - -         

      if(previousRetraction == true) {
        previousRetraction = false;
        val+=INCREMENT*2;   // see: retraction

        Serial.print("G0 E");
        Serial.println(val); 
      }
//  - - - - - - - - - - - - - - - - - - - - - - -     


//  - - - - - - - - - - - - - - - - - - - - - - -         
      Serial.print("G0");  // send to Marlin: listen


      Serial.print(" F");  // send to Marlin: Extruder velocity
      Serial.print(fast);  
    
      Serial.print(" E");  // send to Marlin: Extrude mm. (entrance filament)
      Serial.println(val); 
//  - - - - - - - - - - - - - - - - - - - - - - -         
     #ifdef DEBUG
     Serial.print("- ");  // TEST the delay: as soon the delay as reached print a "-"
     #endif

      val+=INCREMENT;
      once = true;
      
     #ifdef DEBUG
//     delay(500);
     #endif

    }
//      delay(kappa);   // SPEED 1400 - delay  700
                       //         200 -       6200 
  } else if (once == true) {  //  if (once == true) --> restart from initial condition
      once = false;
      previousRetraction = true;
      
//  - - - - - - - - - - ( RETRACTION ) - - - - -         
      val-=INCREMENT*4;   // decrease "val" by n times +1 because previously increased
                          // ToDo: move previous increment on top from the second round

      if(val < 0) val = 0;
      Serial.print("G0");
      Serial.print(" F");
      Serial.print(maxRetractionSpeed);
      Serial.print(" E");
      Serial.println(val); 
//  - - - - - - - - - - - - - - - - - - - - - - -     
    

// tolle: o torna indietro tanto quanto ha stampato fino ad ora
//    Serial.println("G92 E0");  // http://reprap.org/wiki/Gcode (reset Extruder Position)
//    val = INCREMENT;

      delay(50);        // delay in between reads for stability

  }
  readPot();

}
 
