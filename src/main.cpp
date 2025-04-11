#include <sstream>

#include "htwlv3.h"

// Página HTML para interface web
String webPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 LoRa Chat</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
  <h1>ESP32 LoRa Communication</h1>
  <form action="/send" method="post">
    <label for="message">Mensagem para enviar:</label><br>
    <textarea id="message" name="message" rows="4" cols="30"></textarea><br><br>
    <input type="submit" value="Enviar">
  </form>
  <h2>Recebido:</h2>
  <div id="received">%RECEIVED%</div>
  <script>
    setInterval(function() {
      fetch('/receive').then(response => response.text()).then(data => {
        document.getElementById("received").innerHTML = data;
      });
    }, 2000);
  </script>
</body>
</html>
)rawliteral";

// #define HTWLV3_SERVER_SSID "LoRa_ESP32"
// #define HTWLV3_SERVER_SSID "LoRa_ESP32_2"
// #define HTWLV3_SERVER_PASSWORD "12345678"

// String rxdata;

void sender(int count);
void receiver();
void onReceive(LoraDataPacket packet);
void onSendDone();
void onSendTimeout();
// void handleRoot();
// void handleSend();
// void handleReceive();

#define STATE_SEND 0
#define STATE_RECEIVE 1
#define STATE_WAIT 2

int state = STATE_SEND;
int count = 0;

void setup()
{
  Board.begin(true, true, false, true);

  Board.lora->setOnReceive(onReceive);
  Board.lora->setOnSendDone(onSendDone);
  Board.lora->setOnSendTimeout(onSendTimeout);

  // Board.server->on("/", HTTP_GET, handleRoot);
  // Board.server->on("/send", HTTP_POST, handleSend);
  // Board.server->on("/receive", HTTP_GET, handleReceive);

  Board.println("setup");
}

void loop()
{
  switch (state)
  {
  case STATE_SEND:
    sender(count);
    state = STATE_WAIT;
    break;
  case STATE_RECEIVE:
    receiver();
    state = STATE_WAIT;
    break;
  case STATE_WAIT:
    Board.process();
    break;
  }
}

void sender(int count)
{
  delay(1000);
  std::ostringstream data;

  data << count + 1;

  const char *message = data.str().c_str();

  Board.print("Sending: ");
  Board.println(message);

  Board.lora->sendPacket(message);
}

void onSendDone()
{
  state = STATE_RECEIVE;
}

void onSendTimeout()
{
  state = STATE_SEND;
}

void receiver()
{
  Board.lora->listenToPacket();
}

void onReceive(LoraDataPacket packet)
{
  Board.print("Rec Data: ");
  Board.println(packet.data);

  count = atoi(packet.data);
  state = STATE_SEND;
  // std::string rxdata(packet.data);
}

// void handleRoot()
// {
//   String page = webPage;
//   page.replace("%RECEIVED%", rxdata);
//   Board.server->send(200, "text/html", page);
// }

// void handleSend()
// {
//   String message = Board.server->arg("message");

//   Board.display->clearDisplay();
//   Board.display->setCursor(0, 0);

//   Board.print("Enviando: ");
//   Board.println(message.c_str());

//   Board.lora->sendPacket(message.c_str());

//   Board.println("Enviado!");

//   Board.server->sendHeader("Location", "/");
//   Board.server->send(303);
// }

// // Exibe dados LoRa recebidos na web
// void handleReceive()
// {
//   Board.server->send(200, "text/plain", rxdata);
// }