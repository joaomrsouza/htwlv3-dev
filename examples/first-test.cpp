#include <WiFi.h>
#include <WebServer.h>
#include <heltec_unofficial.h>

#define PAUSE 300
#define FREQUENCY 433.2
#define BANDWIDTH 125.0
#define SPREADING_FACTOR 12
#define TRANSMIT_POWER 21

const char *ssid = "LoRa_ESP32_2";
const char *password = "12345678";
WebServer server(80);

String rxdata;
volatile bool rxFlag = false;
long counter = 0;
uint64_t last_tx = 0;
uint64_t tx_time;
uint64_t minimum_pause;

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

// Função de setup
void setup()
{
  heltec_setup();
  both.println("Inicializando Radio LoRa...");
  RADIOLIB_OR_HALT(radio.begin());
  radio.setDio1Action(rx);
  RADIOLIB_OR_HALT(radio.setFrequency(FREQUENCY));
  RADIOLIB_OR_HALT(radio.setBandwidth(BANDWIDTH));
  RADIOLIB_OR_HALT(radio.setSpreadingFactor(SPREADING_FACTOR));
  RADIOLIB_OR_HALT(radio.setOutputPower(TRANSMIT_POWER));
  RADIOLIB_OR_HALT(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF));

  // Inicialização WiFi
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  both.println("WiFi Inicializado");
  both.printf("IP: %s\n", IP.toString().c_str());

  // Configuração do servidor web
  server.on("/", HTTP_GET, handleRoot);
  server.on("/send", HTTP_POST, handleSend);
  server.on("/receive", HTTP_GET, handleReceive);
  server.begin();

  both.println("Servidor Web Ativo");
}

// Função de loop principal
void loop()
{
  heltec_loop();
  server.handleClient();

  bool tx_legal = millis() > last_tx + minimum_pause;

  if ((PAUSE && tx_legal && millis() - last_tx > (PAUSE * 1000)) || button.isSingleClick())
  {
    if (!tx_legal)
    {
      both.printf("Aguarde %i seg.\n", (int)((minimum_pause - (millis() - last_tx)) / 1000) + 1);
      return;
    }
    both.printf("Transmitindo [%s] ", String(counter).c_str());
    radio.clearDio1Action();
    heltec_led(50);
    tx_time = millis();
    RADIOLIB(radio.transmit(String(counter++).c_str()));
    tx_time = millis() - tx_time;
    heltec_led(0);
    if (_radiolib_status == RADIOLIB_ERR_NONE)
    {
      both.printf("Transmitido OK (%i ms)\n", (int)tx_time);
    }
    else
    {
      both.printf("Falha (%i)\n", _radiolib_status);
    }
    minimum_pause = tx_time * 100;
    last_tx = millis();
    radio.setDio1Action(rx);
    RADIOLIB_OR_HALT(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF));
  }

  if (rxFlag)
  {
    rxFlag = false;
    radio.readData(rxdata);
    if (_radiolib_status == RADIOLIB_ERR_NONE)
    {
      both.printf("Recebido [%s]\n", rxdata.c_str());
      both.printf("RSSI: %.2f dBm\n", radio.getRSSI());
      both.printf("SNR: %.2f dB\n", radio.getSNR());
    }
    RADIOLIB_OR_HALT(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF));
  }
}

// Função de callback para recepção LoRa
void rx()
{
  rxFlag = true;
}

// Manipula a página inicial
void handleRoot()
{
  String page = webPage;
  page.replace("%RECEIVED%", rxdata);
  server.send(200, "text/html", page);
}

// Manipula envio de dados LoRa via web
void handleSend()
{
  String message = server.arg("message");
  both.printf("Enviando: %s\n", message.c_str());
  radio.clearDio1Action();
  heltec_led(50);
  tx_time = millis();
  RADIOLIB(radio.transmit(message.c_str()));
  tx_time = millis() - tx_time;
  heltec_led(0);

  if (_radiolib_status == RADIOLIB_ERR_NONE)
  {
    both.printf("Enviado OK (%i ms)\n", (int)tx_time);
  }
  else
  {
    both.printf("Erro (%i)\n", _radiolib_status);
  }

  minimum_pause = tx_time * 100;
  last_tx = millis();
  radio.setDio1Action(rx);
  RADIOLIB_OR_HALT(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF));

  server.sendHeader("Location", "/");
  server.send(303);
}

// Exibe dados LoRa recebidos na web
void handleReceive()
{
  server.send(200, "text/plain", rxdata);
}