#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP32Ping.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include "modelo_ia.h" 

// Declaração limpa e única do display
TFT_eSPI tft = TFT_eSPI();

// --- CONFIGURAÇÕES DE REDE E ACESSO ---
const char* ssid = "brisa-3738741"; 
const char* password = "uf8gg6cc";
const char* BOTtoken = "8579086813:AAEqyG7aKjyY8aNv-cWDmhVh_y2OsPMsM_A";
const char* CHAT_ID = "1460313553";
const IPAddress remote_ip(192, 168, 0, 1); 

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
Eloquent::ML::Port::RandomForest ia;

void displayMetrics(float rtt, float jitter, float perda, float desvio, const String &status);

const int TAMANHO_JANELA = 10;
int historicoPredicoes[TAMANHO_JANELA];
int indiceJanela = 0;
unsigned long ultimoAlertaTelegram = 0;
const unsigned long INTERVALO_MSG = 60000; 

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
    client.setInsecure(); 
    
    for(int i=0; i<TAMANHO_JANELA; i++) historicoPredicoes[i] = 0;
    
    Serial.println("\n--- MONITOR DE BORDA: FINGERPRINTING ATIVO ---");
    bot.sendMessage(CHAT_ID, "🛡️ Monitoramento Iniciado.\nPor Luis Henrike and Marcelino Camilo", "");
    
    #ifdef TFT_BL
        pinMode(TFT_BL, OUTPUT);
        digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);
    #endif

    tft.init();
    
    // Rotação 0 ou 2 para modo Retrato vertical com a tela preta preenchendo 100%
    tft.setRotation(1); 
    tft.fillScreen(TFT_BLACK);
    
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
}

void loop() {
    float rtt_soma = 0, rtt_antigo = 0, jitter_soma = 0;
    float rtt_min = 9999.0, rtt_max = 0.0;
    int sucessos = 0;
    int sinalWifi = WiFi.RSSI();

    for(int i = 0; i < 5; i++) {
        if(Ping.ping(remote_ip, 1)) {
            float rtt_atual = Ping.averageTime();
            rtt_soma += rtt_atual;
            
            if (rtt_atual < rtt_min) rtt_min = rtt_atual;
            if (rtt_atual > rtt_max) rtt_max = rtt_atual;

            if (sucessos > 0) {
                jitter_soma += abs(rtt_atual - rtt_antigo);
            }
            rtt_antigo = rtt_atual;
            sucessos++;
        }
    }

    float rtt_medio = (sucessos > 0) ? (rtt_soma / sucessos) : 1000.0;
    float jitter = (sucessos > 1) ? (jitter_soma / (sucessos - 1)) : 0.0;
    float perda = ((5.0 - sucessos) / 5.0) * 100.0;
    float desvio_assinatura = (sucessos > 0) ? (rtt_max - rtt_min) : 0.0;

    float entrada_ia[] = { rtt_medio, jitter, perda };
    int preda_atual = ia.predict(entrada_ia);

    historicoPredicoes[indiceJanela] = preda_atual;
    indiceJanela = (indiceJanela + 1) % TAMANHO_JANELA;

    int contagemAtaque = 0;
    for(int i=0; i<TAMANHO_JANELA; i++) {
        contagemAtaque += historicoPredicoes[i];
    }

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
            statusRede = "TRAFEGO SUSPEITO";
        }
    }

    Serial.printf("RTT: %.2fms | JIT: %.2fms | LOSS: %.0f%% | DEV: %.2fms | STATUS: %s (%d/10)\n", 
                  rtt_medio, jitter, perda, desvio_assinatura, statusRede.c_str(), contagemAtaque);
                  
    displayMetrics(rtt_medio, jitter, perda, desvio_assinatura, statusRede);
    
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

void displayMetrics(float rtt, float jitter, float perda, float desvio, const String &status) {
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    
    tft.setCursor(5, 0, 2);
    tft.printf("RTT: %-6.1f ms   ", rtt); 
    
    tft.setCursor(5, 30, 2); 
    tft.printf("JIT: %-6.1f ms   ", jitter);
    
    tft.setCursor(5, 60, 2);
    tft.printf("LOSS: %-3.0f %%    ", perda);
    
    tft.setCursor(5, 90, 2);
    tft.printf("DEV: %-6.1f ms   ", desvio);

    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(5, 130, 1);
    tft.print("STATUS:                 "); 
    
    tft.setCursor(5, 155, 1);
    
    if (status == "SEGURO") {
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
    } else if (status.indexOf("ATAQUE") != -1 || status.indexOf("FALHA") != -1) {
        tft.setTextColor(TFT_RED, TFT_BLACK);
    } else {
        tft.setTextColor(TFT_ORANGE, TFT_BLACK); 
    }
    
    tft.print(status + "                 "); 
}