//Codice relativo alla scheda Arduino B
//Cremonini Davide VR471360 UNIVR 2023
#include <stdlib.h>
#include <Servo.h>

//inizizializzazione variabili
//servomotori
Servo servo1;
Servo servo2;

int in=-1; //variabile che prenderà il valore letto da seriale
int ao=-1; //variabile che mi è utile per non stampare più volte lo stesso valore letto ad ogni ciclo

//inizializzazione led 
int led_rosso=4; 

//variabili per sensore prossimita
// const int triggerPort=9;
// const int echoPort=10;
// int auto_near=0;

//funzione per aprire la sbarra dell' ingresso
void ingresso(){
  //apro
  for(int i=0;i<90;i++){
    servo1.write(i);
    delay(20);
  }
  delay(8000);
  //chiudo
  for(int i=90;i>0;i--){
    servo1.write(i);
    delay(50);
  }
}

//funzione per aprire la sbarra dell'uscita
void uscita(){
  //apro
  for(int i=90;i>0;i--){
    servo2.write(i);
    delay(20);
  }
  delay(8000);
  //chiudo
  for(int i=0;i<90;i++){
    servo2.write(i);
    delay(50);
  }
}

//funzione per accendere il led di errore
void errore(){

  digitalWrite(led_rosso,HIGH); //on
  delay(5000);
  digitalWrite(led_rosso,LOW); //off

}


void setup() {
  //assegno i pin ai servomotori
  servo1.attach(8);
  servo2.attach(7);

  //izizializzo pin led 
  pinMode(led_rosso, OUTPUT);

  //inizializzo seriale
  Serial.begin(9600);

  //inizializzo sensore prossimità
  // pinMode(triggerPort,OUTPUT);
  // pinMode(echoPort,INPUT);

  //resetto le posizioni delle due sbarre all'inizio per evitare eventuali aperture anomale
  servo1.write(0);
  servo2.write(90);

}


void loop() {
  //ad ogni ciclo leggo ack da seriale che arriva da esp
  in=Serial.read()-48;

  //debug
  if(ao!=in){ //per non printare continuamente
    Serial.print("Leggo da seriale: ");
    Serial.println(in);

    ao=in; //per non printare continuamente
  }

  //ad ogni ciclo eseguo un controllo sull'input letto e mi comporto in modo diverso a seconda di ciò che accade
  if(in==1){
    ingresso();
  }else if(in==2){
    uscita();
  }else if(in==3){
    errore();
  }

}
