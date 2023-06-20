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
int8_t waveFraction = 2;

void setup()
{
	Serial.begin(115200);

	Serial.println("Aguardando 2 segundos...");
	delay(2000);

	int frequency = getCpuFrequencyMhz();
	Serial.print("Frequencia da CPU: ");
	Serial.print(frequency);
	Serial.println(" MHz");

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
	server.addHandler(&ws);
}

// Valor do último tick da onda de entrada
float_t previousWaveValue;

// Valor atual da onda de entrada
int currentWaveValue;

void loop()
{
	ws.cleanupClients();

	// Cálculo do tempo necessário para a fração da onda ser concluída
	float_t period = (1 / waveFraction) / 60;

	currentWaveValue = digitalRead(IN_SIGNAL);

	ws.printfAll("%i:%i", waveFraction, currentWaveValue);

	// Valor mudou de 0 para 1
	if (previousWaveValue == LOW && currentWaveValue == HIGH)
	{
		delay(period * 1000); // Multiplicação necessária para transformar em ms
		digitalWrite(OUT_SIGNAL, HIGH);
		ws.textAll("--> 1");
	}

	if (previousWaveValue == HIGH && currentWaveValue == LOW)
	{
		digitalWrite(OUT_SIGNAL, LOW);
		ws.textAll("--> 0");
	}

	// Armazena o valor da onda (0 || 1)
	previousWaveValue = currentWaveValue;
}