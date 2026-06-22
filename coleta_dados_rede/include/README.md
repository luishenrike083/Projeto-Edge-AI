# Detecção de Anomalias em Redes IoT via Edge AI (Random Forest)

Este projeto foi desenvolvido seguindo as diretrizes da **Temática 1 – Uso de Inteligência Artificial em Tempo Real para Predição de Eventos em Redes**. A solução utiliza um microcontrolador **ESP32-S3 (M5Stack Cardputer)** para detectar ataques de negação de serviço (DoS/DDoS) em tempo real, utilizando métricas multivariadas de rede.

## 📋 Resumo do Projeto

A solução proposta aborda a segurança em redes IoT movendo a inteligência do servidor para a **Borda (Edge)**. O dispositivo monitora a saúde da rede e utiliza um modelo de **Random Forest** (treinado em Python e convertido para C++) para classificar o estado da rede entre "Seguro" ou "Ataque Detectado" com base em variações de latência e integridade de pacotes.

---

## 🛠️ Tecnologias e Ferramentas

* **Hardware:** M5Stack Cardputer (ESP32-S3).
* **Firmware:** C++ / Arduino Framework (VS Code + PlatformIO).
* **IA/ML:** Python (Scikit-Learn, Pandas) no Google Colab.
* **Conversão:** `micromlgen` (Portabilidade de modelos ML para C++).
* **Emulação de Ataques:** Kali Linux (Ferramenta `hping3`).

---

## 📊 Metodologia e Dataset

### Métrica de Análise
O sensor coleta três métricas fundamentais a cada ciclo de inferência:
1.  **RTT (Round-Trip Time):** Latência média em milissegundos.
2.  **Jitter:** Variação estatística da latência entre pings sucessivos.
3.  **Packet Loss:** Percentual de pacotes perdidos em uma rajada de 5 pings.

### O Dataset (`dataset.csv`)
O modelo foi treinado com um dataset balanceado contendo:
* **Classe 0 (Normal):** Tráfego estável, RTT < 20ms, 0% perda.
* **Classe 1 (Ataque):** Inundação de pacotes (SYN/ICMP Flood), RTT alto ou >20% de perda.

---

## 🚀 Como Replicar este Projeto

### 1. Preparação do Hardware
1. Conecte o **ESP32-S3** ao computador.
2. No arquivo `src/main.cpp`, configure o seu **SSID**, **Senha do Wi-Fi** e o **IP do Alvo** (seu PC).

### 2. Configuração do Ambiente (PlatformIO)
Certifique-se de que o arquivo `platformio.ini` contém as flags de USB para o chip S3:
```ini
build_flags = 
    -D ARDUINO_USB_CDC_ON_BOOT=1
    -D ARDUINO_USB_MODE=1