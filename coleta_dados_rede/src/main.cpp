#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP32Ping.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include "modelo_ia.h" // Certifique-se que este arquivo está na pasta include

// --- CONFIGURAÇÕES DE REDE E ACESSO ---
const char* ssid = "brisa-1173521"; 
const char* password = "yypgqnac";
const char* BOTtoken = "8579086813:AAEqyG7aKjyY8aNv-cWDmhVh_y2OsPMsM_A";
const char* CHAT_ID = "1460313553";
const IPAddress remote_ip(192, 168, 0, 1); // IP do seu PC Alvo

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
Eloquent::ML::Port::RandomForest ia;

// --- VARIÁVEIS DE CONTROLE E JANELA DESLIZANTE ---
const int TAMANHO_JANELA = 10;
int historicoPredicoes[TAMANHO_JANELA];
int indiceJanela = 0;
unsigned long ultimoAlertaTelegram = 0;
const unsigned long INTERVALO_MSG = 60000; // 1 minuto entre alertas

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    // Conexão Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) { 
        delay(500); 
        Serial.print("."); 
    }
    
    client.setInsecure(); // Necessário para o HTTPS do Telegram
    
    // Inicializa a janela deslizante com zeros (Seguro)
    for(int i=0; i<TAMANHO_JANELA; i++) historicoPredicoes[i] = 0;
    
    Serial.println("\n--- MONITOR DE BORDA: FINGERPRINTING ATIVO ---");
    
    // Mensagem Inicial Personalizada
    String msgBoasVindas = "🛡️ Monitoramento Iniciado.\nPor Luis Henrike and Marcelino Camilo";
    bot.sendMessage(CHAT_ID, msgBoasVindas, "");
}

void loop() {
    float rtt_soma = 0, rtt_antigo = 0, jitter_soma = 0;
    float rtt_min = 9999.0, rtt_max = 0.0;
    int sucessos = 0;
    int sinalWifi = WiFi.RSSI();

    // Fase de Coleta: 5 pings para análise de Impressão Digital
    for(int i = 0; i < 5; i++) {
        if(Ping.ping(remote_ip, 1)) {
            float rtt_atual = Ping.averageTime();
            rtt_soma += rtt_atual;
            
            // Captura de extremos para cálculo do Desvio (DEV)
            if (rtt_atual < rtt_min) rtt_min = rtt_atual;
            if (rtt_atual > rtt_max) rtt_max = rtt_atual;

            if (sucessos > 0) {
                jitter_soma += abs(rtt_atual - rtt_antigo);
            }
            rtt_antigo = rtt_atual;
            sucessos++;
        }
    }

    // Cálculos das Métricas Finais
    float rtt_medio = (sucessos > 0) ? (rtt_soma / sucessos) : 1000.0;
    float jitter = (sucessos > 1) ? (jitter_soma / (sucessos - 1)) : 0.0;
    float perda = ((5.0 - sucessos) / 5.0) * 100.0;
    float desvio_assinatura = (sucessos > 0) ? (rtt_max - rtt_min) : 0.0;

    // Inferência da IA (Edge AI)
    float entrada_ia[] = { rtt_medio, jitter, perda };
    int preda_atual = ia.predict(entrada_ia);

    // Atualização da Janela Deslizante (Filtro de Falso Positivo)
    historicoPredicoes[indiceJanela] = preda_atual;
    indiceJanela = (indiceJanela + 1) % TAMANHO_JANELA;

    int contagemAtaque = 0;
    for(int i=0; i<TAMANHO_JANELA; i++) {
        contagemAtaque += historicoPredicoes[i];
    }

    // --- LÓGICA DE DECISÃO E CLASSIFICAÇÃO ---
    String statusRede = "SEGURO";
    bool alertaReal = false;

    if (contagemAtaque >= 7) { 
        if (sinalWifi < -80) {
            statusRede = "INSTAVEL (SINAL FRACO)";
        } 
        else if (desvio_assinatura < 10.0 && rtt_medio > 80.0) {
            statusRede = "ATAQUE! ASSINATURA CONFIRMADA";
            alertaReal = true;
        } 
        else if (perda > 25.0) {
            statusRede = "FALHA DE CONEXÃO";
            alertaReal = true;
        } 
        else {
            statusRede = "TRÁFEGO SUSPEITO";
        }
    }

    // --- SAÍDA SERIAL PARA O MONITOR PC (COM UNIDADES) ---
    Serial.printf("RTT: %.2fms | JIT: %.2fms | LOSS: %.0f%% | DEV: %.2fms | STATUS: %s (%d/10)\n", 
                  rtt_medio, jitter, perda, desvio_assinatura, statusRede.c_str(), contagemAtaque);

    // --- NOVO LAYOUT DE ALERTA TELEGRAM ---
    if (alertaReal && (millis() - ultimoAlertaTelegram > INTERVALO_MSG)) {
        String msg = "🚨 *DETECÇÃO DE INTRUSÃO (DDoS)*\n\n";
        msg += "📍 *ALVO:* `" + remote_ip.toString() + "`\n";
        msg += "📊 *ESTATÍSTICAS DO EVENTO:*\n";
        msg += "└ RTT Médio: `" + String(rtt_medio, 2) + " ms`\n";
        msg += "└ Jitter: `" + String(jitter, 2) + " ms`\n";
        msg += "└ Perda (Loss): `" + String(perda, 0) + " %`\n";
        msg += "└ Desvio (DEV): `" + String(desvio_assinatura, 2) + " ms`\n\n";
        
        msg += "🔍 *ANÁLISE DE ASSINATURA:*\n";
        if (statusRede.indexOf("FINGERPRINT") != -1) {
            msg += "└ *Tipo:* Flood Persistente (Alta freq.)\n";
            msg += "└ *Origem:* Fluxo de IP Sintético (Script).\n";
        } else {
            msg += "└ *Tipo:* Saturação de Banda/Buffer\n";
            msg += "└ *Origem:* Múltiplas requisições persistentes.\n";
        }
        
        msg += "\n✅ *Status:* Confirmado por Edge AI (" + String(contagemAtaque) + "/10)";
        
        if(bot.sendMessage(CHAT_ID, msg, "Markdown")) {
            ultimoAlertaTelegram = millis();
        }
    }

    delay(1000);
}