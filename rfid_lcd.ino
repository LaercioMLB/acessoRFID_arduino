  /*IBLIOTECAS USADAS: MFRC522, EEPROM, LIQUIDCRYSTAL*/

#include <EEPROM.h>     // Vai ler e escrever as UIDs do PICC para a EEPROM  REF: https://www.youtube.com/watch?v=0rlgIs1EUe0
#include <SPI.h>        // MODULO RC522 USANDO PROTOCOLO SPI 
#include <MFRC522.h>  // BIBLIOTECA MFRC522
#include <LiquidCrystal.h>   //LCD

bool programMode = false;  // inicializa o modo de programação para falso

uint8_t successRead;    

byte storedCard[4];           // Armazena uma ID lida na EEPROM
byte readCard[4];             // Armazena a identificação digitalizada do módulo RFID
byte masterCard[4];           // Armazena o ID da placa principal lido na EEPROM

//LCD
LiquidCrystal lcd(6, 7, 5, 4, 3, 2);

// Criar a instancia MFRC522.
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

  Serial.println(F("Controle de Acesso v0.1"));  
  ShowReaderDetails(); 
  
  // Verifique se o CARDMASTER está definido, se não permitir que o usuário escolha um cartão principal
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
      successRead = getID();            // Define succesRead para 1 quando passar no leitor, caso contrário vai a 0
      Serial.println("CARREGANDO...."); // Visualizando se precisa definir o MasterCARD
      
      lcd.clear();
      lcd.print("CARREGANDO...");
    }
    while (!successRead);                  // O programa só continua quando der Sucesso
    for ( uint8_t j = 0; j < 4; j++ ) {        // Loop 4 vezes
      EEPROM.write( 2 + j, readCard[j] );  // Escreve a UID do PICC na EEPROM, a partir do enderço 3 da memória
    }
    EEPROM.write(1, 143);                  // Escreveu na EEPROM para definir o MasterCARD
    Serial.println(F("Cartão Mestre definido!"));
    
    lcd.clear();
    lcd.print("Definido!");
    delay(1000);
  }
  Serial.println(F("-------------------"));
  Serial.println(F("UID do Cartão mestre: "));
  
  for ( uint8_t i = 0; i < 4; i++ ) {          // Lê o MasterCARD da EEPROM
    masterCard[i] = EEPROM.read(2 + i);    // Escrever no MasterCARD
    Serial.print(masterCard[i], HEX);
  }
  Serial.println("");
  Serial.println(F("-------------------"));
  Serial.println(F("Está tudo pronto!"));
  Serial.println(F("Esperando PICCs para ser digitalizado"));
}


///////////////////////////////////////// Main Loop ///////////////////////////////////
void loop () {
  do {
    successRead = getID();  // Define succesRead para 1 quando passar no leitor, caso contrário vai a 0

    if (programMode) {
     
      Serial.println("*ESPERANDO PARA LER O PRÓXIMO CARTÃO*");
      lcd.clear();
      lcd.print(" ESPERANDO LER ");
      lcd.setCursor(0,1);
      lcd.print("O PROXIMO CARTAO");
    }
    else {
   
      Serial.println("*MODO NORMAL ATIVADO*");
      lcd.clear();
      lcd.print("MODO NORMAL");
      lcd.setCursor(0,1);
      lcd.print("ATIVADO");
      delay(1000);
    }
  }
  while (!successRead);   //Não continua até dar successRead !!
  if (programMode) {
    if ( isMaster(readCard) ) { //Se tiver o cartão sendo lido no sensor sairá do modo de gravação
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
      if ( findID(readCard) ) { // se conhecer o cartão vai excluir ele
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
      else {                    //Se não reconhecer o cartão vai adicionar ele
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
    if ( isMaster(readCard)) {    // Se a ID do cartão for a do MasterCARD entra no modo gravação
      programMode = true;
      Serial.println(F("Olá Mestre - Entrando MODO GRAVAÇÃO")); 
      lcd.clear();
      lcd.print("Ola Mestre");
      lcd.setCursor(0,1);
      lcd.print("Entrando MODO GRAVACAO");
      delay(1000);
      
      uint8_t count = EEPROM.read(0);   // Le o primeiro byte da EEPROM
      Serial.print(F("Eu tenho "));     // armazena o numero de IDs na EEPROM
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
      if ( findID(readCard) ) { 
        Serial.println(F("Seja bem-vindo!"));
        lcd.clear();
        lcd.print("Seja bem-vindo!");
        delay(1000);
        granted(300);        
      }
      else {     
        Serial.println(F("Cartão não encontrado"));
        lcd.clear();
        lcd.print("Cartao nao encontrado");
        denied();
        delay(1000);
      }
    }
  }
}

/////////////////////////////////////////  Acesso Concedido    ///////////////////////////////////
void granted ( uint16_t setDelay) {
                                                                   
 digitalWrite(8, HIGH); // ativa rele, abre a trava solenoide
 delay(3000);           // espera 3 segundos
 digitalWrite(8, LOW);  // desativa rele, fecha a trava solenoide 
 Serial.println("Acesso concedido");
 lcd.clear();
 lcd.print("Acesso concedido");
 delay(1000);
}

///////////////////////////////////////// Acesso Negago  ///////////////////////////////////
void denied() {
  Serial.println("Acesso negado");
  lcd.clear();
  lcd.print("Acesso negado");
  delay(1000);
}


///////////////////////////////////////// UID do PICC ///////////////////////////////////
uint8_t getID() {
  //Preparando para ler a PICC
  if ( ! mfrc522.PICC_IsNewCardPresent()) { //Se a PICC for nova, irá continuar
    return 0;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {   //Se a PICC ja existir, ele continuará
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
  //Obter a ultima versão do software mfrc522
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
  // Quando 0x00 ou 0xFF é retornado, a comunicação provavelmente falhou
  if ((v == 0x00) || (v == 0xFF)) {
    Serial.println(F("AVISO: Falha de comunicação, o MFRC522 está conectado corretamente?"));
    Serial.println(F("SYSTEM HALTED: Verifique as conexões."));
    while (true); //
    lcd.clear();
    lcd.print("DEU RUIM");
    delay(1000);
  }
}

//////////////////////////////////////// Leia o ID da EEPROM //////////////////////////////
void readID( uint8_t number ) {
  uint8_t start = (number * 4 ) + 2;    // Posição Inicial
  for ( uint8_t i = 0; i < 4; i++ ) {     // Fazer 4 vezes um loop para gerar 4 bytes
    storedCard[i] = EEPROM.read(start + i);   // Atribuir valor lido a EEPROM principal
  }
}

///////////////////////////////////////// Add ID a EEPROM   ///////////////////////////////////
void writeID( byte a[] ) {
  if ( !findID( a ) ) {     // Antes de escrever na EEPROM vai verificar se ja existe este cartão
    uint8_t num = EEPROM.read(0);     
    uint8_t start = ( num * 4 ) + 6;  // Descobre onde vai o proximo slot
    num++;                 
    EEPROM.write( 0, num );     
    for ( uint8_t j = 0; j < 4; j++ ) { 
      EEPROM.write( start + j, a[j] );  
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
    
  }
}

///////////////////////////////////////// Remover ID da EEPROM   ///////////////////////////////////
void deleteID( byte a[] ) {
  if ( !findID( a ) ) {     
    failedWrite();      
    Serial.println(F("ERROR! Há algo de errado com a ID ou EEPROM está com problemas."));
    lcd.clear();
    lcd.print("ERROR402");
    delay(1000);
   
  }
  else {
    uint8_t num = EEPROM.read(0);  
    uint8_t slot;       
    uint8_t start;     
    uint8_t looping;    
    uint8_t j;
    uint8_t count = EEPROM.read(0); 
    slot = findIDSLOT( a );   
    start = (slot * 4) + 2;
    looping = ((num - slot) * 4);
    num--;      
    EEPROM.write( 0, num );  
    for ( j = 0; j < looping; j++ ) {        
      EEPROM.write( start + j, EEPROM.read(start + 4 + j));   
    }
    for ( uint8_t k = 0; k < 4; k++ ) {        
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
  for ( uint8_t k = 0; k < 4; k++ ) {   
    if ( a[k] != b[k] ) {    
       return false;
    }
  }
  return true;  
}

///////////////////////////////////////// Localizar slot   ///////////////////////////////////
uint8_t findIDSLOT( byte find[] ) {
  uint8_t count = EEPROM.read(0);       // Le o primeiro byte da EEPROM
  for ( uint8_t i = 1; i <= count; i++ ) {   
    readID(i);                
    if ( checkTwo( find, storedCard ) ) {   
      
      return i;         
    }
  }
}

///////////////////////////////////////// Localizar ID da EEPROM   ///////////////////////////////////
bool findID( byte find[] ) {
  uint8_t count = EEPROM.read(0);     
  for ( uint8_t i = 1; i < count; i++ ) {    
    readID(i);          
    if ( checkTwo( find, storedCard ) ) {  
      return true;
    }
    else {    
    }
  }
  return false;
}

///////////////////////////////////////// ESCRITO COM SUCESSO NA EEPROM ///////////////////////////////////
void successWrite() {
  Serial.println("Gravação bem-sucedida");
  lcd.clear();
  lcd.print("Gravacao"); 
  lcd.setCursor(0,1);
  lcd.print("bem-sucedida");
  delay(1000);
}

///////////////////////////////////////// FALHOU NA EEPROM //////////////////////////////////
void failedWrite() {
  Serial.println("Algo aconteceu de errado na gravação");
  lcd.clear();
  lcd.print("ERROR275");
  delay(1000);
  //MANUAL DE ERROR
}

///////////////////////////////////////// UID removida com sucesso da EEPROM ///////////////////////////////////
void successDelete() {
 
  Serial.println("UID removida com sucesso da EEPROM");
  lcd.clear();
  lcd.print("UID removida");
  lcd.setCursor(0,1);
  lcd.print("com sucesso");
  delay(1000);

}

////////////////////// Verificar se é o MasterCARD   ///////////////////////////////////
bool isMaster( byte test[] ) {
  return checkTwo(test, masterCard);
}

bool monitorWipeButton(uint32_t interval) {
  uint32_t now = (uint32_t)millis();
  while ((uint32_t)millis() - now < interval)  {
    
    if (((uint32_t)millis() % 500) == 0) {
//      if (digitalRead(wipeB) != LOW)
        return false;
    }
  }
  return true;
}
