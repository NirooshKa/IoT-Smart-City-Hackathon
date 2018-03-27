//INITIAL DECLARATIONS
#define echoPin 12
#define trigPin 13
#define ledOne 15
#define ledTwo 0
#define ledThree 16

//VARIABLES
int duration, cm;

//------------------START OF CODE-------------------------------------------------------
void setup() 
{
  Serial.begin(9600);
  
  //SENSOR PINS
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  //STREETLIGHT PINS
  pinMode(ledOne, OUTPUT);
  pinMode(ledTwo, OUTPUT);
  pinMode(ledThree, OUTPUT);
}

void loop()
{
  /*1. The distance between the car and the person/vehicle gets measured
  */
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration= pulseIn(echoPin,HIGH);
  cm=(duration*0.034)/2; //This is the distance away from the sensor in cm
  Serial.print("Distance: ");
  Serial.println(cm);
  delay(100);

  /*2. With the distances and sounds measured from before,
  * We are going to determine which lights to turn on,
  * and which lights to turn off to ensure the car can see
  * its surroundings
  */
  
  //2.1: The last streetlight turns on after you pass the other two
   if (cm >= 3 && cm <7){
      digitalWrite(ledOne, HIGH);
      digitalWrite(ledTwo, LOW);
      digitalWrite(ledThree, LOW);
      delay(3000);
   }

  //2.2: Two streetlights turn on after you pass the first one behind it
   else if (cm >= 7 && cm < 15){
      digitalWrite(ledOne, HIGH); 
      digitalWrite(ledTwo, HIGH);
      digitalWrite(ledThree, LOW);
      delay(3000);
   }

  //2.3: All of the lights turn on right when a car is present within a 5cm radius
   else if (cm >= 15 && cm <19){
      digitalWrite(ledOne, HIGH); 
      digitalWrite(ledTwo, HIGH);
      digitalWrite(ledThree, HIGH);
      delay(3000);
   }

  //2.4: Now we deal with the case when the lights are on, but nobody is there
    digitalWrite(ledOne, LOW);
    digitalWrite(ledTwo, LOW);
    digitalWrite(ledThree, LOW);
}

/*------ PIR MOTION DETECTOR CODE (WITHOUT BJT) -----------
/----DECIDED TO USE A BJT, AND MODIFY IT FOR THE MOTION DETECTION---------

int stat = 14; //status
int sensor = 13; //sensor

void setup(){
  Serial.begin(9600);
  pinMode(sensor, INPUT);
  pinMode(stat, OUTPUT);
  stat = LOW;
}

void loop(){
  long state = digitalRead(sensor);
    if(state == HIGH){
      digitalWrite(stat, HIGH);
      Serial.println("Motion Detected!");
      delay(1000);
      digitalWrite(stat, LOW);
    }
    else{
      digitalWrite(stat, LOW);
      Serial.println("Motion absent");
      delay(10000);
    }
}
*/
