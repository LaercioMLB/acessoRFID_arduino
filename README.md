# BEM VINDO AO SENSOR RFID COM LCD NO ARDUINO

## ![BEM VINDO](https://i.imgur.com/yYFkdUL.jpg)

> **OBS:** Com pouco de conhecimento da para alterar o código tranquilamente para adaptar outro sensor.

## _Obs2:_ Uso Academico!!!🎓🎓

[![LaDEV](https://i.imgur.com/mzHsMye.jpg)](https://www.linkedin.com/in/laércio-bubiak-3b974a179/)

---

# Sensor RFID + LCD

📰 Este sistema é capaz de adicionar e remover tags [RFID](https://www.google.com/search?q=tags+rfid&tbm=isch&ved=2ahUKEwiSu4Xl-4XrAhUgCbkGHcuOBqMQ2-cCegQIABAA&oq=tags+rfid&gs_lcp=CgNpbWcQAzICCAAyBAgAEB4yBggAEAgQHjIGCAAQCBAeMgYIABAIEB4yBggAEAgQHjIGCAAQCBAeMgYIABAIEB4yBggAEAgQHjIGCAAQCBAeOgQIIxAnOgQIABBDOgUIABCxAzoHCAAQsQMQQzoHCCMQ6gIQJzoICAAQsQMQgwFQ0PabAViih5wBYI-InAFoA3AAeACAAccBiAG_CpIBBDIuMTCYAQCgAQGqAQtnd3Mtd2l6LWltZ7ABCsABAQ&sclient=img&ei=Z6UrX9LrKaCS5OUPy52amAo&bih=1010&biw=1818&client=opera&hs=Rex&hl=pt-BR) usando uma tag como a principal (TAG MESTRE), tendo a opção de abrir e fechar um **Relé**, usando a própria memoria do Arduino EEPROM. Simples e fácil!

> Uma rapida pesquinha no google você vê os modelos mais usados!! Lembrando que os cartões da atualidade pode ter esta tecnologia como os cartões do Nubank, Santander, Banco Inter entre outros. (Caso for usar para uso pessoal, pode ser uma boa opção!).

##### Bibliotecas usadas 👨‍🔧:

- [EEPROM](https://www.arduino.cc/en/Reference/EEPROM)
- [MFRC522](https://www.arduinolibraries.info/libraries/mfrc522)
- [LiquidCrystal](https://www.arduino.cc/en/Reference/LiquidCrystal)

---

# DIAGRAMA FISICO:

![Diagrama](https://i.imgur.com/Ec4t3ww.png)

# Ordem dos pinos Arduino

| Signal    | MFRC522 Reader/PCD | Ard UNO/101 | Ard Mega | Ard Nano v3 | Ard Leonardo/Micro | Arduino Pro Micro |
| --------- | ------------------ | ----------- | -------- | ----------- | ------------------ | ----------------- |
|           | PIN                | PIN         | PIN      | PIN         | PIN                | PIN               |
| RST/Reset | RST                | 9           | 5        | D9          | RESET/ICSP-5       | RST               |
| SPI SS    | SDA(SS)            | 10          | 53       | D10         | 10                 | 10                |
| SPI MOSI  | MOSI               | 11 / ICSP-4 | 51       | D11         | ICSP-4             | 16                |
| SPI MISO  | MISO               | 12 / ICSP-1 | 50       | D12         | ICSP-1             | 14                |
| SPI SCK   | SCK                | 13 / ICSP-3 | 52       | D13         | ICSP-3             | 15                |

# DIAGRAMA

![Diagrama](https://i.imgur.com/Ztu0RC9.png)

# PERGUNTAS:

- O que é UID do PICC? [link](https://www.youtube.com/watch?v=0rlgIs1EUe0)
- MODULO MFRC522 [link](https://portal.vidadesilicio.com.br/modulo-rfid-rc522-mifare/)

## Referencias:

- [FilipeFlop](https://www.filipeflop.com/blog/controle-acesso-leitor-rfid-arduino/)
