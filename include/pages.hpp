#include <Arduino.h>

const char MAIN_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-br">
	<head>
		<meta charset="UTF-8" />
		<meta http-equiv="X-UA-Compatible" content="IE=edge" />
		<meta name="viewport" content="width=device-width, initial-scale=1.0" />
		<title>Retificador controlado</title>

		<style>
			body {
				width: 100%;
				height: 100vh;
				margin: 0;
				padding: 0;
				box-sizing: border-box;
				background-color: rgb(115, 243, 200);
				font-family: "Segoe UI", Tahoma, Geneva, Verdana, sans-serif;
			}

			header {
				padding: 0 2rem;
			}

			main {
				display: flex;
				flex-direction: column;
				text-align: center;
			}

			.status-box {
				display: flex;
				justify-content: center;
				align-items: center;
				gap: 0.5rem;
				margin: 1rem 0;
				text-align: center;
			}

			.status {
				width: 10px;
				height: 10px;
				border-radius: 100%;
				background-color: blue;
			}

			.alert-box {
				display: flex;
				justify-content: center;
				align-items: center;
			}

			.alert {
				display: none;
				margin-top: 1rem;
				padding: 0.5rem;
				border-radius: 0.2rem;
				background-color: rgb(224, 55, 55);
				color: white;
				width: fit-content;
			}

			footer {
				text-align: center;
			}
		</style>
	</head>
	<body>
		<header>
			<p>Professor Paulo Cunha</p>
		</header>

		<main>
			<h1>Retificador de onda completa controlado</h1>

			<div class="status-box">
				<div class="status" id="status"></div>
				<span id="status-text">Aguardando conexão.</span>
			</div>

			<form id="form" class="form">
				<input
					type="number"
					name="fraction"
					id="fraction"
					placeholder="Fração da onda desejada:"
					min="1"
					max="10"
					value="2" />

				<button id="change" type="submit">Alterar onda</button>
			</form>

			<div class="alert-box">
				<span id="alert" class="alert">Mudança não enviada ao microcontrolador!</span>
			</div>

			<div>
				<h2>Valor atual da fração: <span id="wave-fraction">?</span></h2>
				<h2>Valor atual da entrada: <span id="wave-output">?</span></h2>
				<h2>Valor atual da saída: <span id="wave-input">?</span></h2>
			</div>
		</main>

		<footer>
			<p>Trabalho realizado por Ana Carleane, João Pedro, Luís Eduardo e Maria Letícia.</p>
		</footer>

		<script>
			const input = document.getElementById("fraction");
			const form = document.getElementById("form");
			const alertBox = document.getElementById("alert");

			form.onsubmit = (ev) => {
				ev.preventDefault();
				alertBox.style.display = "none";

				fraction = Number(input.value);

				// Envia troca de fração para API
				fetch(`/change?fraction=${fraction}`)
					.then((res) => {
						if (res.ok) return;

						alertBox.style.display = "flex";
						alertBox.innerText = "Mudança não enviada ao microcontrolador!";
						console.error("Não foi possível contactar a API.");
					})
					.catch((err) => {
						alertBox.style.display = "flex";
						alertBox.innerText = "Mudança não enviada ao microcontrolador!";
						console.error("Não foi possível contactar a API:", err);
					});
			};
		</script>

		<script>
			let fraction = 0;
			let inputVal = 0;
			let output = 0;
			let dataInterval;

			const statusEl = document.getElementById("status");
			const statusText = document.getElementById("status-text");

			console.log("Host: " + window.location.host);
			const socket = new WebSocket(`ws://${window.location.host}/ws`);
			console.log("Tentando conectar-se ao websocket...");

			socket.onopen = (event) => {
				statusEl.style.backgroundColor = "green";
				statusText.innerText = "Conectado.";

				console.log("Conectado ao websocket:", event);

				dataInterval = setInterval(() => {
					if (!socket.readyState) return;
					socket.send("data");
				}, 100);
			};

			socket.onerror = (event) => {
				statusEl.style.backgroundColor = "red";
				statusText.innerText = "Erro na conexão.";

				console.log("Erro no websocket:", event);
				clearInterval(dataInterval);
			};

			socket.onmessage = (event) => {
				const data = event.data;
				console.log("Mensagem recebida:", data);

				fraction = Number(data.split(":")[0]) || fraction;
				inputVal = Number(data.split(":")[1]) == 1 ? 1 : 0;
				output = Number(data.split(":")[2]) == 1 ? 1 : 0;

				document.getElementById("wave-fraction").innerText = fraction;
				document.getElementById("wave-input").innerText = inputVal;
				document.getElementById("wave-output").innerText = output;
			};
		</script>
	</body>
</html>
)rawliteral";