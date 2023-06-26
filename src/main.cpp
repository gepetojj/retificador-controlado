#include <Arduino.h>
#include <WiFi.h>

#include "ESPAsyncWebServer.h"
#include "ESPmDNS.h"
#include "pages.hpp"

#define IN_SIGNAL 22
#define OUT_SIGNAL 4

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Fração desejada da onda de entrada (2 = 1/2; 4 = 1/4; etc)
u_int8_t waveFraction = 2;
u_int8_t previousWaveValue = 0;
u_int8_t currentWaveValue = 0;
u_int8_t currentOutputValue = 0;

float fullPeriod = 1 * 0.01666;
float halfPeriod = fullPeriod * 0.5;

// Divisão da meia onda pela fração desejada
double fraction = 0;
// Tempo de espera em segundos
double waitSeconds = 0;
// Tempo de espera em microsegundos
float waitMicroseconds = 0;

void onDataHandler(void *arg, uint8_t *data, size_t len)
{
	AwsFrameInfo *info = (AwsFrameInfo *)arg;

	if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
	{
		data[len] = 0;
		if (strcmp((char *)data, "data") == 0)
		{
			Serial.println("-------------------------");
			Serial.print("Fracao da onda: ");
			Serial.println(waveFraction);
			Serial.print("Valor da entrada: ");
			Serial.println(currentWaveValue);
			Serial.print("Valor da saida: ");
			Serial.println(currentOutputValue);
			Serial.print("Tempo de meio periodo: ");
			Serial.println(halfPeriod, 5);
			Serial.print("Meia onda dividida pela fracao: ");
			Serial.println(fraction, 5);
			Serial.print("Tempo de espera em segundos: ");
			Serial.println(waitSeconds, 5);
			Serial.print("Tempo de espera em microsegundos: ");
			Serial.println(waitMicroseconds);
			Serial.println("-------------------------");

			ws.printfAll("%i:%i:%i", waveFraction, currentWaveValue, currentOutputValue);
		}
	}
}

void onEventHandler(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
	switch (type)
	{
	case WS_EVT_DATA:
		onDataHandler(arg, data, len);
		break;

	case WS_EVT_ERROR:
		Serial.println("Houve um erro na comunicacao WebSocket.");
		break;

	default:
		break;
	}
}

void setup()
{
	Serial.begin(115200);

	pinMode(IN_SIGNAL, INPUT);
	pinMode(OUT_SIGNAL, OUTPUT);

	WiFi.mode(WIFI_AP);
	WiFi.softAP("Retificador controlado");

	server.on("/", HTTP_GET, [](AsyncWebServerRequest *req)
			  { req->send_P(200, "text/html", MAIN_PAGE); });

	server.on("/change", HTTP_GET, [](AsyncWebServerRequest *req)
			  {
		if (req->hasParam("fraction")) {
			waveFraction = std::atoi(req->getParam("fraction")->value().c_str());
			req->send(200, "text/plain", "Seus dados foram recebidos.");
		} else {
			req->send(400, "text/plain", "Envie os dados necessarios.");
		} });

	server.begin();
	ws.onEvent(onEventHandler);
	server.addHandler(&ws);
}

void loop()
{
	ws.cleanupClients();

	fraction = halfPeriod / waveFraction;
	waitSeconds = fraction * (waveFraction - 1);
	waitMicroseconds = waitSeconds * 1000000;

	currentWaveValue = digitalRead(IN_SIGNAL);

	// Valor mudou de 0 para 1
	if (previousWaveValue == LOW && currentWaveValue == HIGH)
	{
		delayMicroseconds(waitMicroseconds);
		digitalWrite(OUT_SIGNAL, HIGH);
		currentOutputValue = HIGH;
	}

	if (previousWaveValue == HIGH && currentWaveValue == LOW)
	{
		digitalWrite(OUT_SIGNAL, LOW);
		currentOutputValue = LOW;
	}

	// Armazena o valor da onda (0 || 1)
	previousWaveValue = currentWaveValue;
}