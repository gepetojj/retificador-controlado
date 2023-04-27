#include <Arduino.h>
#include <WiFi.h>

#include "ESPAsyncWebServer.h"
#include "ESPmDNS.h"
#include "pages.hpp"

#define IN_SIGNAL 22
#define OUT_SIGNAL 4

AsyncWebServer server(80);

// Valor do último tick da onda de entrada
float_t previousWaveValue;

// Fração desejada da onda de entrada (2 = 1/2; 4 = 1/4; etc)
int8_t waveFraction = 2;

// Cálculo do tempo necessário para a fração da onda ser concluída
float_t period = (1 / waveFraction) / 60;

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
	if (previousWaveValue == LOW && digitalRead(IN_SIGNAL) == HIGH)
	{
		delay(period * 1000); // Multiplicação necessária para transformar em ms
		digitalWrite(OUT_SIGNAL, HIGH);
	}

	if (previousWaveValue == HIGH && digitalRead(IN_SIGNAL) == LOW)
	{
		digitalWrite(OUT_SIGNAL, LOW);
	}

	// Armazena o valor da onda (0 || 1)
	previousWaveValue = digitalRead(IN_SIGNAL);
}