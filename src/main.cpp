#include <Arduino.h>
#include <WiFi.h>

#include "ESPAsyncWebServer.h"
#include "ESPmDNS.h"
#include "pages.hpp"

#define IN_SIGNAL 22
#define OUT_SIGNAL 4

AsyncWebServer server(80);

float value;

// Fração desejada da onda de entrada (2 = 1/2; 4 = 1/4; etc)
unsigned int waveFraction = 2;

// Duração de uma onda completa
const float fullWave = 16.66; // ms

// Duração de uma meia onda
const float halfWave = fullWave / 2; // ms

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

			Serial.print("Fracao recebida: ");
			Serial.println(req->getParam("fraction")->value());
			Serial.print("Fracao transformada: ");
			Serial.println(std::atoi(req->getParam("fraction")->value().c_str()));

			req->send(200, "text/plain", "Seus dados foram recebidos.");
		} else {
			req->send(400, "text/plain", "Envie os dados necessarios.");
		} });

	server.begin();
}

void loop()
{
	// Valor mudou de 0 para 1
	if (value == LOW && digitalRead(IN_SIGNAL) == HIGH)
	{
		delay(halfWave / waveFraction);
		digitalWrite(OUT_SIGNAL, HIGH);
	}

	// Valor mudou de 1 para 0
	if (value == HIGH && digitalRead(IN_SIGNAL) == LOW)
	{
		digitalWrite(OUT_SIGNAL, LOW);
	}

	// Armazena o valor da onda (0 || 1)
	value = digitalRead(IN_SIGNAL);
}