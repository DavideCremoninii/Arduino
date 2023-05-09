//Codice relativo alla scheda Arduino A
//Cremonini Davide VR471360 UNIVR 2023
#include "U8glib.h" 
#include <stdlib.h> 
#include <SoftwareSerial.h> 

//inizializzazione variabili 

//inizializzazione led 
int verde=13; 
int blue=12; 
int giallo=11; 

//inizializzazione pulsanti 
int green_button=5; 
int help_button=4; 

//stati 
int in_status=LOW; //stato di ingresso/uscita 
int help_status=LOW; //stato di help request attiva/non attiva 

//variabili
char mode=' '; //stringa che potrà essere impostata su 3 valori: i(in), o(out), h(help) [+ nuvo valore r(stato di attesa resettato)]
String code=""; //stringa dove salverò il codice letto da gm65 
String name=""; //stringa dove salvo il nome per mostrarlo a video una volta completata l'autenticazione
String output=""; //stringa che invierò in seriale come output di questo arduino 
String output_prec=""; //stringa utile per non inviare all'infinito un output uguale 
int change=0; //flag per sapere se i pulsanti sono stati cambiati o no
int rck=-1; //variabile intera che può assumere più valori in base all'ack ricevuto dall'esp

//stringhe utili per printare sul display
String frase1="";
String frase2="";
String frase3="";
String frase4="";

//variabili per il display oled 
char display_output[20];  
int string_width;  
String str_to_display=""; 
//                        D0 D1 CS  DC RES 
U8GLIB_SSD1306_128X64 u8g(6, 7, 10, 9, 8);   // SPI communication: SCK/D0 = 6, SDA/D1 = 7, RES = 8, DC = 9, CS = 10 for the oled display 

//dichiaro una seriale software per la comunicazione con gm65 
SoftwareSerial ss(3, 2); // RX, TX 

 
//funzione per stampare sul display oled 
void display_print(String f1,String f2,String f3, String f4){ 
  u8g.firstPage(); 
  do {  
    f1.toCharArray(display_output,20); //metto la stringa nell'array di riferimento per la stampa a video 
    u8g.drawStr(64-u8g.getStrWidth(display_output)/2, 10, display_output); //centro la stringa e stampo a video 

    f2.toCharArray(display_output,20); //metto la stringa nell'array di riferimento per la stampa a video 
    u8g.drawStr(64-u8g.getStrWidth(display_output)/2, 25, display_output); //centro la stringa e stampo a video 

    f3.toCharArray(display_output,20); //metto la stringa nell'array di riferimento per la stampa a video 
    u8g.drawStr(64-u8g.getStrWidth(display_output)/2, 40, display_output); //centro la stringa e stampo a video 

    f4.toCharArray(display_output,20); //metto la stringa nell'array di riferimento per la stampa a video 
    u8g.drawStr(64-u8g.getStrWidth(display_output)/2, 55, display_output); //centro la stringa e stampo a video 


  } while ( u8g.nextPage() ); 
 
}
 
//funzione per leggere e salvare i codici scansionati dal sensore gm65 
void lettura_code(){ 
  // while (!(ss.available())) { 
  //     //do nothing 
  // }   
  // Serial.write(ss.read()); 

  if(ss.available()){ 
    //Serial.write(ss.read()); //scan & print (momentaneo, non va bene) 

    String scan=ss.readString();  //leggo ciò che viene scansionato e lo salvo in una stringa 
    scan.trim(); //rimuovo eventuali \r e \n finali 

    code=scan.substring(0,8); //salvo i primi 8 caratteri, utili all'identificazione
    name=scan.substring(8); //il resto è il nome dell'utente
  }   
} 
 
//funzione per inviare stringa in seriale a esp32 
void invio_dati(){ 
  /*prima di scirvere sulla seriale, formatto la stringa nel seguente modo: 
  primo carattere->modalità (i/o/h/r) 
  resto dei caratteri->codice letto
   
  */   
  if((mode!=' ' && code!="") || mode=='h' || mode=='r'){
    
    output=mode+code; //formatto la stringa
    if(output!=output_prec){ //controllo per evitare di spammare dati in seriale sempre uguali
      Serial.println(output); //scrivo su seriale la stringa formattata che dovrà poi essere interpretata da esp32 
      output_prec=output;
  
      //resetto i valori che ho inviato 
      code=""; 
      mode=' ';
    }
  }
   
}

//funzione per portare il sistema in stato iniziale mostrando la legenda dei pulsanti all'utente
void reset(){
  //spengo tutto
  digitalWrite(blue,LOW); 
  digitalWrite(verde,LOW); 
  digitalWrite(giallo,LOW);

  //imposto display
  frase1="Premi un pulsante :";
  frase2="";
  frase3="VERDE: IN/OUT";
  frase4="GIALLO: Help";
  display_print(frase1,frase2,frase3,frase4);

}

//funzione per leggere ack di esp32 da seriale
int leggo_risposta(){

  //variabile temporanea che contiene ciò che leggo dalla seriale dall'esp
  int ck=Serial.read()-48; //-48 perchè viene inviato come stringa 

  if(ck==1){ //se vale 1 sta entrando
    frase1="Identificazione";
    frase2="avvenuta con";
    frase3="successo!";
    frase4="Benvenuto "+name;
    display_print(frase1,frase2,frase3,frase4); 
    delay(5000);
    frase1="Sbarra in funzione";
    frase2="Attendere ...";
    frase4=""; 
    for(int i=10;i>0;i--){
      frase3=String(i)+" secondi rimasti";
      display_print(frase1,frase2,frase3,frase4); 
      delay(1000);
    }
  }else if(ck==2){ //se vale 1 sta uscendo
    frase1="Identificazione";
    frase2="avvenuta con";
    frase3="successo!";
    frase4="Arrivederci "+name;
    display_print(frase1,frase2,frase3,frase4); 
    delay(5000);
    frase1="Sbarra in funzione";
    frase2="Attendere ...";
    frase4=""; 
    for(int i=10;i>0;i--){
      frase3=String(i)+" secondi rimasti";
      display_print(frase1,frase2,frase3,frase4); 
      delay(1000);
    }
  }else if(ck==3){ //non corrisponde oppure errore di invio in seriale
    frase1="Errore scansione!";
    frase2="";
    frase4="secondi"; 
    for(int i=5;i>0;i--){
      frase3="Riprovare tra "+String(i);
      display_print(frase1,frase2,frase3,frase4); 
      delay(1000);
    }
  }else if(ck==4){ //richiesta di aiuto chiusa dal pannello di controllo
    frase1="Richiesta chiusa";
    frase2="";
    frase4="secondi"; 
    for(int i=3;i>0;i--){
      frase3="Ripristino tra "+String(i);
      display_print(frase1,frase2,frase3,frase4); 
      delay(1000);
    }

  }else{
    return 0; //non è stato letto nulla di utile
  }
  return ck; //returno che è stato letto qualcosa di utile
}


void setup() { 
  //inizializzo seriale per comunicare con esp32 
  Serial.begin(9600);
  //inizializzo seriale con gm65 per ricevere codici
  ss.begin(9600);  
   
  //izizializzo pin led 
  pinMode(verde, OUTPUT); 
  pinMode(blue, OUTPUT); 
  pinMode(giallo, OUTPUT); 
 
  //inizializzo pin pulsanti 
  pinMode(green_button,INPUT_PULLUP); //pulsante senza resistenza, arduino provvede alla resistenza 
  pinMode(help_button,INPUT_PULLUP); 
 
  //settaggi per il display oled 
  u8g.setColorIndex(1); 
  u8g.setFont(u8g_font_profont12); 

  //porto subito il display allo stato iniziale dove mostra all'utente la legenda
  reset();
} 


void loop() { 

  //leggo pulsanti ogni ciclo 
  if(digitalRead(help_button)==LOW){ //se pulsante premuto, cambio stato 
    help_status=!help_status;
    delay(200); 
    //change=1; //aggiunto flag per sapere se i pulsanti sono stati cambiati o no
    
    //aggiungo if per resettare e non tornare a in\out
    if(help_status==LOW){
      reset();
      mode='r'; //richiesta chiusa da pulsante che verrà inviata a esp
    }else{
      change=1;
    }

  }else if(digitalRead(green_button)==LOW){ //se pulsante premuto, cambio stato 
    in_status=!in_status;  //cambio stato tra ingresso e uscita
    delay(200);
    change=1; //flag per sapere se i pulsanti sono stati cambiati o no
  }



  
  //se pulsanti sono stati premuti e qualche stato è cambiato, rieseguo i controlli
  if(change==1){

    change=0; //una volta entrato nell'if, reimposto il flag a 0

    /*3 diversi stati del circuito a seconda dei pulsanti premuti e degli stati attivi 
    -ingresso 
    -uscita 
    -richiesta di aiuto 
    */   
    if(help_status==HIGH){ //richiesta aiuto
      
      //modifico stato led 
      digitalWrite(blue,LOW); //spengo tutto il resto 
      digitalWrite(verde,LOW); 
      digitalWrite(giallo,HIGH); //accendo giallo 

      //mostro a video lo stato 
      //display_print("Richiesta aiuto"); 
      frase1="Richiesta di aiuto";
      frase2="inviata, attendere.";
      frase3="Premere nuovamente";
      frase4="per annullare.";

      //modifica char di stato 
      mode='h'; 
   
    }else{
      //no richiesta, gestisco in/out 
   
      digitalWrite(giallo,LOW); //spengo giallo 
   
      //eseguo controllo su secondo stato 
      if(in_status==HIGH){ 
        //se in_status è high allora imposto il circuito in modalità in 
   
        //modifico stato led 
        digitalWrite(blue,LOW); 
        digitalWrite(verde,HIGH); 
        //mostro a video lo stato 
        frase1="Hai selezionato:";
        frase2="INGRESSO";
        frase3="Scansiona codice";
        frase4="per entrare";
        //modifica char di stato 
        mode='i';  
      }else{ 
        //se in_status è low allora imposto il circuito in modalità out 
   
        //modifico stato led 
        digitalWrite(blue,HIGH); 
        digitalWrite(verde,LOW); 
        //mostro a video lo stato 
        frase1="Hai selezionato:";
        frase2="USCITA";
        frase3="Scansiona codice";
        frase4="per uscire";
        //modifica char di stato 
        mode='o'; 
      }
      //fine else dell'help
    }
    //fine if del change
  }
  display_print(frase1,frase2,frase3,frase4); 
  
  //ad ogni ciclo eseguo un controllo sulla richiesta di aiuto(se è attiva o se è stata chiusa)
  if(mode=='h' || mode=='r'){
    invio_dati(); //invio h o r ad esp

  }else{
    lettura_code(); //leggo codice da gm65
    invio_dati(); //invio a esp 32
  
  }

  //ad ogni ciclo eseguo la lettura da seriale dove troverò un eventuale ack da esp
  rck=leggo_risposta();
  if(rck==1 || rck==2 || rck==3){
    reset(); //resetto il sistema e ricomincio il loop
  }else if(rck==4){
    help_status=LOW; //abbasso lo stato di help
    reset(); //e resetto il sistema

    //informo esp(che cambierà anche il db) e lo preparo a una nuova lettura (così permetto una nuova h)
    mode='r';
    invio_dati();
  }
}

