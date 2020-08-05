               /*
   --------------------------------------------------------------------------------------------------------------------
   BIBLIOTECA MFRC522 EDITADA BY: Laércio Bubiak    [https://www.youtube.com/c/krazzymen]
   --------------------------------------------------------------------------------------------------------------------
   BIBLIOTECAS USADAS: MFRC522, EEPROM.

   Este exemplo mostra um sistema completo de controle de acesso à porta

  "Fluxograma" do sistema fisicamente :
                                     +---------+
  +----------------------------------->READ TAGS+^------------------------------------------+
  |                              +--------------------+                                     |
  |                              |                    |                                     |
  |                              |                    |                                     |
  |                         +----v-----+        +-----v----+                                |
  |                         |MASTER TAG|        |OTHER TAGS|                                |
  |                         +--+-------+        ++-------------+                            |
  |                            |                 |             |                            |
  |                            |                 |             |                            |
  |                      +-----v---+        +----v----+   +----v------+                     |
  |         +------------+READ TAGS+---+    |KNOWN TAG|   |UNKNOWN TAG|                     |
  |         |            +-+-------+   |    +-----------+ +------------------+              |
  |         |              |           |                |                    |              |
  |    +----v-----+   +----v----+   +--v--------+     +-v----------+  +------v----+         |
  |    |MASTER TAG|   |KNOWN TAG|   |UNKNOWN TAG|     |GRANT ACCESS|  |DENY ACCESS|         |
  |    +----------+   +---+-----+   +-----+-----+     +-----+------+  +-----+-----+         |
  |                       |               |                 |               |               |
  |       +----+     +----v------+     +--v---+             |               +--------------->
  +-------+EXIT|     |DELETE FROM|     |ADD TO|             |                               |
          +----+     |  EEPROM   |     |EEPROM|             |                               |
                     +-----------+     +------+             +-------------------------------+


   Use a Master Card which is act as Programmer then you can able to choose card holders who will granted access or not

 * **Easy User Interface**

   Just one RFID tag needed whether Delete or Add Tags. You can choose to use Leds for output or Serial LCD module to inform users.

 * **Stores Information on EEPROM**

   Information stored on non volatile Arduino's EEPROM memory to preserve Users' tag and Master Card. No Information lost
   if power lost. EEPROM has unlimited Read cycle but roughly 100,000 limited Write cycle.

 * **Security**
   To keep it simple we are going to use Tag's Unique IDs. It's simple and not hacker proof.

   @license Released into the public domain.

   Typical pin layout used:
   -----------------------------------------------------------------------------------------
               MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
               Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
   Signal      Pin          Pin           Pin       Pin        Pin              Pin
   -----------------------------------------------------------------------------------------
   RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
   SPI SS      SDA(SS)      10            53        D10        10               10
   SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
   SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
   SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
*/

#include <EEPROM.h>     // We are going to read and write PICC's UIDs from/to EEPROM
#include <SPI.h>        // RC522 Module uses SPI protocol
#include <MFRC522.h>  // Library for Mifare RC522 Devices
#include <LiquidCrystal.h>   //LCD

bool programMode = false;  // inicializa o modo de programação para falso

uint8_t successRead;    // Variable integer to keep if we have Successful Read from Reader

byte storedCard[4];           // Armazena uma ID lida na EEPROM
byte readCard[4];             // Armazena a identificação digitalizada do módulo RFID
byte masterCard[4];           // Armazena o ID da placa principal lido na EEPROM

//LCD
LiquidCrystal lcd(6, 7, 5, 4, 3, 2);

// Create MFRC522 instance.
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);

///////////////////////////////////////// Setup ///////////////////////////////////
void setup() {

  pinMode(8 , OUTPUT); //SAIDA DO RELÉ

  //Configuração do protocolo
  Serial.begin(9600);  // Inicialize as comunicações seriais com o PC
  SPI.begin();           // MFRC522 usa protocolo SPI
  mfrc522.PCD_Init();    // Inicializa o MFRC522

  //Define o número de colunas e linhas do LCD:  
  lcd.begin(16, 2);

  Serial.println(F("Controle de Acesso v0.1"));   // For debugging purposes
  ShowReaderDetails();  // Show details of PCD - MFRC522 Card Reader details
  
  // Verifique se o cartão mestre está definido, se não permitir que o usuário escolha um cartão principal
  // Isso também é útil para apenas redefinir a Master Card
  // Você pode manter outros registros de EEPROM apenas escrever diferente de 143 para o endereço da EEPROM 1
  // O endereço EEPROM 1 deve conter um número mágico que é '143'

  if (EEPROM.read(1) != 143) {
    Serial.println(F("Cartão mestre não definido"));
    Serial.println(F("Aproxime um cartão para defini-lo como mestre"));
    lcd.clear();
    lcd.print("Aproxime para");
    lcd.setCursor(0,1);
    lcd.print("definir o mestre");
    delay(2000);
    
    do {
      successRead = getID();            // sets successRead to 1 when we get read from reader otherwise 0
      Serial.println("CARREGANDO...."); // Visualize Mastercard need to be defined
      
      lcd.clear();
      lcd.print("CARREGANDO...");
    }
    while (!successRead);                  // Program will not go further while you not get a successful read
    for ( uint8_t j = 0; j < 4; j++ ) {        // Loop 4 times
      EEPROM.write( 2 + j, readCard[j] );  // Write scanned PICC's UID to EEPROM, start from address 3
    }
    EEPROM.write(1, 143);                  // Write to EEPROM we defined Master Card.
    Serial.println(F("Cartão Mestre definido!"));
    
    lcd.clear();
    lcd.print("Definido!");
    delay(1000);
  }
  Serial.println(F("-------------------"));
  Serial.println(F("UID do Cartão mestre: "));
  
  for ( uint8_t i = 0; i < 4; i++ ) {          // Read Master Card's UID from EEPROM
    masterCard[i] = EEPROM.read(2 + i);    // Write it to masterCard
    Serial.print(masterCard[i], HEX);
  }
  Serial.println("");
  Serial.println(F("-------------------"));
  Serial.println(F("Está tudo pronto!"));
  Serial.println(F("Esperando PICCs para ser digitalizado"));
 // cycleLeds();    // Everything ready lets give user some feedback by cycling leds
}


///////////////////////////////////////// Main Loop ///////////////////////////////////
void loop () {
  do {
    successRead = getID();  // sets successRead to 1 when we get read from reader otherwise 0
    // When device is in use if wipe button pressed for 10 seconds initialize Master Card wiping
    if (programMode) {
     // cycleLeds();              // Program Mode cycles through Red Green Blue waiting to read a new card
      Serial.println("*ESPERANDO PARA LER O PRÓXIMO CARTÃO*");
      lcd.clear();
      lcd.print(" ESPERANDO LER ");
      lcd.setCursor(0,1);
      lcd.print("O PROXIMO CARTAO");
    }
    else {
    // normalModeOn();     // Normal mode, blue Power LED is on, all others are off
      Serial.println("*MODO NORMAL ATIVADO*");
      lcd.clear();
      lcd.print("MODO NORMAL");
      lcd.setCursor(0,1);
      lcd.print("ATIVADO");
      delay(1000);
    }
  }
  while (!successRead);   //the program will not go further while you are not getting a successful read
  if (programMode) {
    if ( isMaster(readCard) ) { //When in program mode check First If master card scanned again to exit program mode
      Serial.println(F("Cartão Mestre foi escaneado pelo sensor..."));
      Serial.println(F("Saindo MODO GRAVAÇÃO"));
      Serial.println(F("-----------------------------"));
      lcd.clear();
      lcd.print("Cartao mestre");
      lcd.setCursor(0,1);
      lcd.print("detectado");
      delay(1000);
      
      lcd.clear();
      lcd.print("SAINDO");
      lcd.setCursor(0,1);
      lcd.print("MODO GRAVAÇÃO");
      delay(1000);
      programMode = false;
      return;
    }
    else {
      if ( findID(readCard) ) { // If scanned card is known delete it
        Serial.println(F("Eu conheço este CARTÃO, removendo ..."));
        lcd.clear();
        lcd.print("Cartao");
        lcd.setCursor(0,1);
        lcd.print("Reconhecido");
        delay(1000);
        
        lcd.clear();
        lcd.print("REMOVENDO...");
        delay(1000);
        
        deleteID(readCard);
        Serial.println("-----------------------------");
        Serial.println(F("Aproxime um cartão para ADICIONAR ou REMOVER da EEPROM"));
        lcd.clear();
        lcd.print("Aproxime cartao");
        lcd.setCursor(0,1);
        lcd.print("para ADD ou REM");
        delay(1000);
        
      }
      else {                    // If scanned card is not known add it
        Serial.println(F("Eu não conheço este CARTÃO, adicionando ..."));
        lcd.clear();
        lcd.print("ADICIONANDO...");
        delay(1000);
        
        writeID(readCard);
        Serial.println(F("-----------------------------"));
        Serial.println(F("Aproxime um cartão para ADICIONAR ou REMOVER da EEPROM"));
        lcd.clear();
        lcd.print("Aproxime cartao");
        lcd.setCursor(0,1);
        lcd.print("para ADD ou REM");
        delay(1000);
      }
    }
  }
  else {
    if ( isMaster(readCard)) {    // If scanned card's ID matches Master Card's ID - enter program mode
      programMode = true;
      Serial.println(F("Olá Mestre - Entrando MODO GRAVAÇÃO")); 
      lcd.clear();
      lcd.print("Ola Mestre");
      lcd.setCursor(0,1);
      lcd.print("Entrando MODO GRAVACAO");
      delay(1000);
      
      uint8_t count = EEPROM.read(0);   // Read the first Byte of EEPROM that
      Serial.print(F("Eu tenho "));     // stores the number of ID's in EEPROM
      Serial.print(count);
      Serial.print(F(" registros na EEPROM"));
      Serial.println("");
      Serial.println(F("Aproxime um cartão para ADICIONAR ou REMOVER da EEPROM"));
      Serial.println(F("Aproxime o Cartão mestre novamente para sair do MODO GRAVAÇÃO"));
      Serial.println(F("-----------------------------"));
      lcd.clear();
      lcd.print("Aproxime cartao");
      lcd.setCursor(0,1);
      lcd.print("para ADD ou REM");
      delay(1000);
      
      lcd.clear();
      lcd.print("Aproxime MESTRE");
      lcd.setCursor(0,1);
      lcd.print("ExitMODOGRAVACAO");
      delay(1000);
      
    }
    else {
      if ( findID(readCard) ) { // If not, see if the card is in the EEPROM
        Serial.println(F("Seja bem-vindo!"));
        lcd.clear();
        lcd.print("Seja bem-vindo!");
        delay(1000);
        granted(300);         // Open the door lock for 300 ms / ABRE A FECHADURA POR 3seg
      }
      else {      // If not, show that the ID was not valid
        Serial.println(F("Cartão não encontrado"));
        lcd.clear();
        lcd.print("Cartao nao encontrado");
        denied();
        delay(1000);
      }
    }
  }
}

/////////////////////////////////////////  Access Granted    ///////////////////////////////////
void granted ( uint16_t setDelay) {
                                                                   
 digitalWrite(8, HIGH); // ativa rele, abre a trava solenoide
 delay(3000);           // espera 3 segundos
 digitalWrite(8, LOW);  // desativa rele, fecha a trava solenoide 
 Serial.println("Acesso concedido");
 lcd.clear();
 lcd.print("Acesso concedido");
 delay(1000);
}

///////////////////////////////////////// Access Denied  ///////////////////////////////////
void denied() {
  Serial.println("Acesso negado");
  lcd.clear();
  lcd.print("Acesso negado");
  delay(1000);
}


///////////////////////////////////////// Get PICC's UID ///////////////////////////////////
uint8_t getID() {
  // Getting ready for Reading PICCs
  if ( ! mfrc522.PICC_IsNewCardPresent()) { //If a new PICC placed to RFID reader continue
    return 0;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {   //Since a PICC placed get Serial and continue
    return 0;
  }
  
  // Existem MIFARE PICCs que têm cuidado UID de 4 bytes ou 7 bytes se você usar PICC de 7 bytes
  // Acho que devemos assumir todos os PICC, pois eles têm UID de 4 bytes
  // Até que suportemos PICCs de 7 bytes
  Serial.println(F("Lendo ID do cartão:"));
  for ( uint8_t i = 0; i < 4; i++) {  //
    readCard[i] = mfrc522.uid.uidByte[i];
    Serial.print(readCard[i], HEX);
  }
  Serial.println("");
  mfrc522.PICC_HaltA(); // Para de ler
  return 1;
}

void ShowReaderDetails() {
  // Get the MFRC522 software version
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.print(F("MFRC522 Software Version: 0x"));
  Serial.print(v, HEX);
  if (v == 0x91)
    Serial.print(F(" = v1.0"));
  else if (v == 0x92)
    Serial.print(F(" = v2.0"));
  else
    Serial.print(F(" (DESCONHECIDO), é um clone chinês?"));
  Serial.println("");
  // When 0x00 or 0xFF is returned, communication probably failed
  if ((v == 0x00) || (v == 0xFF)) {
    Serial.println(F("AVISO: Falha de comunicação, o MFRC522 está conectado corretamente?"));
    Serial.println(F("SYSTEM HALTED: Verifique as conexões."));
    while (true); // do not go further
    lcd.clear();
    lcd.print("DEU RUIM");
    delay(1000);
  }
}

//////////////////////////////////////// Read an ID from EEPROM //////////////////////////////
void readID( uint8_t number ) {
  uint8_t start = (number * 4 ) + 2;    // Figure out starting position
  for ( uint8_t i = 0; i < 4; i++ ) {     // Loop 4 times to get the 4 Bytes
    storedCard[i] = EEPROM.read(start + i);   // Assign values read from EEPROM to array
  }
}

///////////////////////////////////////// Add ID to EEPROM   ///////////////////////////////////
void writeID( byte a[] ) {
  if ( !findID( a ) ) {     // Before we write to the EEPROM, check to see if we have seen this card before!
    uint8_t num = EEPROM.read(0);     // Get the numer of used spaces, position 0 stores the number of ID cards
    uint8_t start = ( num * 4 ) + 6;  // Figure out where the next slot starts
    num++;                // Increment the counter by one
    EEPROM.write( 0, num );     // Write the new count to the counter
    for ( uint8_t j = 0; j < 4; j++ ) {   // Loop 4 times
      EEPROM.write( start + j, a[j] );  // Write the array values to EEPROM in the right position
    }
    successWrite();
    Serial.println(F("ID ADCIONADA com sucesso"));
    lcd.clear();
    lcd.print("ID ADICIONADA");
    lcd.setCursor(0,1);
    lcd.print("com sucesso");
    delay(1000);
  }
  else {
    failedWrite();
    Serial.println(F("ERROR! Há algo de errado com a ID ou EEPROM está com problemas."));
    lcd.clear();
    lcd.print("ERROR402");
    delay(1000);
    //MANUAL DE ERROR
  }
}

///////////////////////////////////////// Remove ID from EEPROM   ///////////////////////////////////
void deleteID( byte a[] ) {
  if ( !findID( a ) ) {     // Before we delete from the EEPROM, check to see if we have this card!
    failedWrite();      // If not
    Serial.println(F("ERROR! Há algo de errado com a ID ou EEPROM está com problemas."));
    lcd.clear();
    lcd.print("ERROR402");
    delay(1000);
    //MANUAL DE ERROR
  }
  else {
    uint8_t num = EEPROM.read(0);   // Get the numer of used spaces, position 0 stores the number of ID cards
    uint8_t slot;       // Figure out the slot number of the card
    uint8_t start;      // = ( num * 4 ) + 6; // Figure out where the next slot starts
    uint8_t looping;    // The number of times the loop repeats
    uint8_t j;
    uint8_t count = EEPROM.read(0); // Read the first Byte of EEPROM that stores number of cards
    slot = findIDSLOT( a );   // Figure out the slot number of the card to delete
    start = (slot * 4) + 2;
    looping = ((num - slot) * 4);
    num--;      // Decrement the counter by one *
    EEPROM.write( 0, num );   // Write the new count to the counter
    for ( j = 0; j < looping; j++ ) {         // Loop the card shift times
      EEPROM.write( start + j, EEPROM.read(start + 4 + j));   // Shift the array values to 4 places earlier in the EEPROM
    }
    for ( uint8_t k = 0; k < 4; k++ ) {         // Shifting loop
      EEPROM.write( start + j + k, 0);
    }
    successDelete();
    Serial.println(F("ID removida com sucesso da EEPROM"));
    lcd.clear();
    lcd.print("ID removida");
    lcd.setCursor(0,1);
    lcd.print("com sucesso");
    delay(1000);
  }
}

///////////////////////////////////////// Check Bytes   ///////////////////////////////////
bool checkTwo ( byte a[], byte b[] ) {   
  for ( uint8_t k = 0; k < 4; k++ ) {   // Loop 4 times
    if ( a[k] != b[k] ) {     // IF a != b then false, because: one fails, all fail
       return false;
    }
  }
  return true;  
}

///////////////////////////////////////// Find Slot   ///////////////////////////////////
uint8_t findIDSLOT( byte find[] ) {
  uint8_t count = EEPROM.read(0);       // Read the first Byte of EEPROM that
  for ( uint8_t i = 1; i <= count; i++ ) {    // Loop once for each EEPROM entry
    readID(i);                // Read an ID from EEPROM, it is stored in storedCard[4]
    if ( checkTwo( find, storedCard ) ) {   // Check to see if the storedCard read from EEPROM
      // is the same as the find[] ID card passed
      return i;         // The slot number of the card
    }
  }
}

///////////////////////////////////////// Find ID From EEPROM   ///////////////////////////////////
bool findID( byte find[] ) {
  uint8_t count = EEPROM.read(0);     // Read the first Byte of EEPROM that
  for ( uint8_t i = 1; i < count; i++ ) {    // Loop once for each EEPROM entry
    readID(i);          // Read an ID from EEPROM, it is stored in storedCard[4]
    if ( checkTwo( find, storedCard ) ) {   // Check to see if the storedCard read from EEPROM
      return true;
    }
    else {    // If not, return false
    }
  }
  return false;
}

///////////////////////////////////////// Write Success to EEPROM   ///////////////////////////////////
// Flashes the green LED 3 times to indicate a successful write to EEPROM
void successWrite() {
  Serial.println("Gravação bem-sucedida");
  lcd.clear();
  lcd.print("Gravacao"); 
  lcd.setCursor(0,1);
  lcd.print("bem-sucedida");
  delay(1000);
}

///////////////////////////////////////// Write Failed to EEPROM   ///////////////////////////////////
// Flashes the red LED 3 times to indicate a failed write to EEPROM
void failedWrite() {
  Serial.println("Algo aconteceu de errado na gravação");
  lcd.clear();
  lcd.print("ERROR275");
  delay(1000);
  //MANUAL DE ERROR
}

///////////////////////////////////////// Success Remove UID From EEPROM  ///////////////////////////////////
// Flashes the blue LED 3 times to indicate a success delete to EEPROM
void successDelete() {
 
  Serial.println("UID removida com sucesso da EEPROM");
  lcd.clear();
  lcd.print("UID removida");
  lcd.setCursor(0,1);
  lcd.print("com sucesso");
  delay(1000);

}

////////////////////// Check readCard IF is masterCard   ///////////////////////////////////
// Check to see if the ID passed is the master programing card
bool isMaster( byte test[] ) {
  return checkTwo(test, masterCard);
}

bool monitorWipeButton(uint32_t interval) {
  uint32_t now = (uint32_t)millis();
  while ((uint32_t)millis() - now < interval)  {
    // check on every half a second
    if (((uint32_t)millis() % 500) == 0) {
//      if (digitalRead(wipeB) != LOW)
        return false;
    }
  }
  return true;
}
