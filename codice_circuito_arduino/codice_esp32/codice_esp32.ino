//Codice relativo alla scheda Esp32
//Cremonini Davide VR471360 UNIVR 2023
#include <stdlib.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

//Credenziali wifi
#define WIFI_SSID "private"
#define WIFI_PASSWORD "private"
//Firebase
#define API_KEY "private"
#define DATABASE_URL "private"
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false; //variabile per checkare la connessione

//definisco pin per seriale
#define RXp2 16
#define TXp2 17

//inizializzazione variabili
String str_from_arduino=""; //stringa dove salvo il contenuto di ciò che viene letto dalla seriale
String in_prec=""; //stringa per salvare l'input precedente e non eseguire il loop più volte se l'input è lo stesso
String id=""; //stringa dove salvo id letto da db
String id_a=""; //stringa dove salvo l'id proveniente da arduino una volta decomposta la stringa in input
String user=""; //stringa dove salvo la key una volta decomposta la stringa in input
char mode=' '; //carattere dove salvo la modalità una volta decomposta la stringa in input
bool help_status = false; //variabile realiva allo stato della richiesta nel database
bool attesa_req=false; //variabile che indica se il sistema si trova in stato di attesa per una richiesta di aiuto
String path=""; //stringa che rappresenta il percorso relativo a un dato del database che modifico a necessità


//funzione per controllare se l'id che leggo da arduino corrisponde all'id che leggo nel db, di uno specifico utente
int check_id(){
  Serial.println("Leggo da Arduino: "+id_a);

  path="/Utenti/"+user+"/";
  if(Firebase.RTDB.getString(&fbdo, path)){
    if(fbdo.dataType()=="string"){
      id=fbdo.stringData();
      Serial.println("Leggo da DB: "+id); //print a video per debug
      if(id==id_a){
        return 1;
      }else{
        return 0;
      }  
    }
  }else{
    Serial.println("Failed! " + fbdo.errorReason());
    return 0;
  }
}

//funzione per settare a true la richiesta di aiuto nel db quando viene letto da arduino
void send_help_req(){
  if(Firebase.RTDB.setBool(&fbdo, "/Richiesta_aiuto/Stato/", true)){
    Serial.println("Richiesta di aiuto inviata con successo!");
  }else{
    Serial.println("Errore! " + fbdo.errorReason());
  }
}

//funzione per settare a false la richiesta di aiuto nel db quando viene letto da arduino
void reset_help_req(){
  if(Firebase.RTDB.setBool(&fbdo, "/Richiesta_aiuto/Stato/", false)){
    Serial.println("Stato richiesta di aiuto resettato correttamente!");
  }else{
    Serial.println("Errore! " + fbdo.errorReason());
  }
}

//funzione per salvare il current time in ingresso di un utente
void salva_time_in(){
  path="/Utenti_in/"+user+"/";
  if(Firebase.RTDB.setInt(&fbdo, path, millis())){
    Serial.println("Tempo ingresso inserito correttamente!");
  }else{
    Serial.println("Errore! " + fbdo.errorReason());
  }
}

//funzione per salvare il current time in uscita di un utente
void salva_time_out(){
  path="/Utenti_out/"+user+"/";
  if(Firebase.RTDB.setInt(&fbdo, path, millis())){
    Serial.println("Tempo ingresso inserito correttamente!");
  }else{
    Serial.println("Errore! " + fbdo.errorReason());
  }
}

//funzione per salvare il tempo totale di presenza all'interno del parcheggio di un utente
void salva_time_db(){
  //inizializzo queste 3 variabili temporanee che mi servono solo momentaneamente per eseguire un calcolo e salvare un dato
  int time_in;
  int time_out;
  int time_prec;

  //prendo il tempo di ingresso
  path="/Utenti_in/"+user+"/";
  if(Firebase.RTDB.getInt(&fbdo, path)){
    if(fbdo.dataType()=="int"){  
      time_in=fbdo.intData();
      //Serial.println("Leggo da DB: "+time_in);

      //resetto il tempo di ingresso
      if(Firebase.RTDB.setInt(&fbdo, path, 0)){
        Serial.println("Tempo ingresso resettato correttamente!");
      }else{
        Serial.println("Errore! " + fbdo.errorReason());
      }

      //prendo il tempo di uscita
      path="/Utenti_out/"+user+"/";
      if(Firebase.RTDB.getInt(&fbdo, path)){
        if(fbdo.dataType()=="int"){  
          time_out=fbdo.intData();
          //Serial.println("Leggo da DB: "+time_out);

          //resetto il tempo di uscita
          if(Firebase.RTDB.setInt(&fbdo, path, 0)){
            Serial.println("Tempo uscita resettato correttamente!");
          }else{
            Serial.println("Errore! " + fbdo.errorReason());
          }
          
          //prendo il tempo precedente salvato
          path="/Utenti_time/"+user+"/";
          if(Firebase.RTDB.getInt(&fbdo, path)){
            if(fbdo.dataType()=="int"){  
              time_prec=fbdo.intData();
              //Serial.println("Leggo da DB: "+time_prec);

              //inserisco la differenza dentro-fuori nel db
              if(Firebase.RTDB.setInt(&fbdo, path, time_prec+((time_out-time_in)/1000))){ //divisio 1000 così salvo i secondi per testing rapido, altrimenti con 60000 prendo i secondi o 3600000 prendo le ore
                Serial.println("Tempo tot inserito correttamente!");
              }else{
                Serial.println("Errore (pt1) ! " + fbdo.errorReason());
              }
            }else{
              Serial.println("Errore (pt2) ! " + fbdo.errorReason());
            }
          }else{
            Serial.println("Errore (pt3) ! " + fbdo.errorReason());
          }
        }else{
          Serial.println("Errore (pt4) ! " + fbdo.errorReason());
        }
      }else{
        Serial.println("Errore (pt5) ! " + fbdo.errorReason());
      }
    }else{
      Serial.println("Errore (pt6) ! " + fbdo.errorReason());
    }
  }else{
    Serial.println("Errore (pt7) ! " + fbdo.errorReason());
  }
}

//funzione per controllare lo stato della richiesta di aiuto dal db
int check_help_resolved(){
  Serial.println("Controllo help status");  //print a video per debug
  if(Firebase.RTDB.getBool(&fbdo, "/Richiesta_aiuto/Stato/")){
    if(fbdo.dataType()=="boolean"){
      help_status=fbdo.boolData();
      Serial.print("Leggo da DB: "); //print a video per debug
      Serial.println(help_status); //print a video per debug

      if(help_status){ //se true significa che non è ancora stata risolta
        return 0;
      }else{ //se false significa che è stata risolta e returno 1 al main
        return 1;
      }  
    }
  }else{
    Serial.println("Errore! " + fbdo.errorReason());
    return 0;
  }
}


void setup() {
  //inizializzo seriale per comunicare con arduino
  Serial2.begin(9600, SERIAL_8N1, RXp2, TXp2);

  //inizializzo seriale per debug
  Serial.begin(115200);
  
  //Parte di setup dedicata alla connessione wifi dell'esp
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("\n\n");
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  //Parte di setup dedicata al collegamento con il DB Firebase
  /* Assign the api key (required) */
  config.api_key = API_KEY;
  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;
  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("Connesso e Pronto\n");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

}


void loop() {

  //ad ogni ciclo controllo se il sistema è in attesa di sblocco
  if(attesa_req){
    if(check_help_resolved()==1){ //controllo se è stato sbloccato
      Serial2.print("4"); //informo arduino dell'aiuto concluso

      attesa_req=false; //resetto lo stato di attesa
      Serial.print("Attesa terminata");
    }
  }

  //ad ogni ciclo salvo ciò che leggo da seriale
  str_from_arduino=Serial2.readString();
  //Serial.println("leggo from arduino: '"+str_from_arduino+"'");
  str_from_arduino.trim(); //rimuovo eventuali \r e \n finali 

  //effettuo un controllo su ciò che ho letto così evito di entrare se è uguale o vuoto
  if(str_from_arduino!=in_prec && str_from_arduino!=""){

    //salvo ciò che ho letto per non rientrare nell'if
    in_prec=str_from_arduino;

    //scompongo la stringa formattata
    mode=str_from_arduino.charAt(0);
    user=str_from_arduino.substring(1,4);
    id_a=str_from_arduino.substring(4);

    //Serial.println("Id letto: "+id_a+"no invio");//test stampa PRE-post trim
    //id_a.trim();
    //Serial.println("Id letto: "+id_a+"no invio (cambia?)");//test pre-POST trim
    //Serial.print("MODE letta: ");
    //Serial.print(mode);
    Serial.print("\n\n");

    //qui parte il controllo sulla modalità letta da seriale selezionata dall'utente
    if(mode=='h'){ //significa che da arduino arriva una richiesta di aiuto
      
      Serial.println("Richiesta di aiuto ricevuta da Arduino!\n");
      send_help_req(); //modifico il valore nel db
      attesa_req=true; //porto a true lo stato di attesa per effettuare il controllo sul db controllando se viene risolta

    }else if(mode=='r'){ //significa che da arduino la richiesta è stata chiusa

      reset_help_req(); //modifico il valore nel db
      attesa_req=false; //resetto lo stato di attesa
    }else if(mode=='i'){ //significa che un utente vuole entrare
      if(check_id()){ //controllo se l'id corrisponde
        
        salva_time_in(); //salvo current time ingresso
        
        Serial.println("Corrispondono! Benvenuto"); 
        Serial2.print("1"); //scrivo ad arduino di aprire la sbarra 1
      }else{
        Serial.println("Non corrispondono!");
        Serial2.print("3"); //segnalo ad arduino errore
      }
    }else if(mode=='o'){ //significa che un utente vuole uscire
      if(check_id()){ //controllo se l'id corrisponde

        salva_time_out(); //salvo current time uscita

        Serial.println("Corrispondono! Arrivederci");
        Serial2.print("2"); //scrivo ad arduino di aprire la sbarra 2

        salva_time_db(); //aggiorno il tempo totale in cui questo utente è stato all'interno del parcheggio
      }else{
        Serial.println("Non corrispondono!");
        Serial2.print("3"); //segnalo ad arduino errore
      }
    }else{
      //altro
      Serial.println("Errore di lettura seriale :(\nCiò che ho letto: ["+str_from_arduino+"]");
      Serial2.print("3"); //segnalo ad arduino errore
    }
    Serial.print("\n\n");

  }
  
}
