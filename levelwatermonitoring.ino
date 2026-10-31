#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4

// ===== KONFIGURASI =====
const char* WIFI_SSID = "A";
const char* WIFI_PASSWORD = "";
const char* BOT_TOKEN = "8923539699:AAFN7iQqs7Lcw4I7RrXAoLOc2SmmeThChhA";
const String CHAT_ID_DIIZINKAN = "7511940466";
// =======================

const byte PIN_SEDANG = 32;
const byte PIN_TINGGI = 33;
const byte PIN_LED = 26;
const byte PIN_BUZZER = 25;

const byte PIN_DIN = 23;
const byte PIN_CLK = 18;
const byte PIN_CS = 5;

const unsigned long DURASI_BUZZER_WASPADA = 5000;

MD_Parola matrix(HARDWARE_TYPE, PIN_DIN, PIN_CLK, PIN_CS, MAX_DEVICES);

WiFiClientSecure secureClient;
UniversalTelegramBot bot(BOT_TOKEN, secureClient);

enum LevelAir { AMAN, WASPADA, BAHAYA };

volatile LevelAir levelSekarang = AMAN;
volatile bool alarmDiizinkan = true;

unsigned long mulaiWaspada = 0;

const char* namaLevel(LevelAir level) {
  if (level == BAHAYA) return "BAHAYA - SEGERA KURANGI AIR";
  if (level == WASPADA) return "WASPADA - PANTAU LEVEL AIR";
  return "JAGALAH KEBERSIHAN";
}

void tampilkan(LevelAir level) {
  matrix.displayClear();
  matrix.displayScroll(
    namaLevel(level),
    PA_LEFT,
    PA_SCROLL_LEFT,
    50
  );
}

String buatStatus() {
  LevelAir level = levelSekarang;
  bool alarmAktif = alarmDiizinkan;

  String pesan = "Status air: ";
  pesan += namaLevel(level);
  pesan += "\nAlarm: ";
  pesan += alarmAktif ? "AKTIF" : "DIBISUKAN";
  pesan += "\nWi-Fi: TERHUBUNG";

  return pesan;
}

void prosesPesan(int jumlahPesan) {
  for (int i = 0; i < jumlahPesan; i++) {
    String chatId = bot.messages[i].chat_id;
    String perintah = bot.messages[i].text;

    if (perintah == "/start") {
      bot.sendMessage(
        chatId,
        "Bot monitoring air aktif.\n"
        "Chat ID kamu: " + chatId +
        "\nKetik /status untuk melihat kondisi air.",
        ""
      );
      continue;
    }

    // Semua orang boleh melihat status.
    if (perintah == "/status") {
      bot.sendMessage(chatId, buatStatus(), "");
      continue;
    }

    // Hanya pemilik yang boleh mengatur alarm.
    if (perintah == "/alarm_off" || perintah == "/alarm_on") {
      if (chatId != CHAT_ID_DIIZINKAN) {
        bot.sendMessage(
          chatId,
          "Hanya pemilik yang bisa mengatur alarm. "
          "Gunakan /status untuk melihat kondisi air.",
          ""
        );
        continue;
      }

      if (perintah == "/alarm_off") {
        alarmDiizinkan = false;
        bot.sendMessage(
          chatId,
          "Alarm dibisukan. Status air tetap dipantau.",
          ""
        );
      } else {
        alarmDiizinkan = true;
        bot.sendMessage(
          chatId,
          "Alarm aktif kembali.",
          ""
        );
      }
    }
  }
}

void tugasTelegram(void* parameter) {
  for (;;) {
    if (WiFi.status() != WL_CONNECTED) {
      WiFi.reconnect();
      vTaskDelay(pdMS_TO_TICKS(10000));
      continue;
    }

    int jumlahPesan = bot.getUpdates(bot.last_message_received + 1);

    while (jumlahPesan > 0) {
      prosesPesan(jumlahPesan);
      jumlahPesan = bot.getUpdates(bot.last_message_received + 1);
    }

    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_SEDANG, INPUT_PULLUP);
  pinMode(PIN_TINGGI, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  digitalWrite(PIN_LED, LOW);
  digitalWrite(PIN_BUZZER, LOW);

  matrix.begin();
  matrix.setIntensity(3);
  tampilkan(AMAN);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  secureClient.setInsecure();

  xTaskCreatePinnedToCore(
    tugasTelegram,
    "Telegram",
    10000,
    nullptr,
    1,
    nullptr,
    0
  );
}

void loop() {
  // Float switch milikmu mendeteksi air saat HIGH.
  bool airSedang = digitalRead(PIN_SEDANG) == HIGH;
  bool airTinggi = digitalRead(PIN_TINGGI) == HIGH;

  LevelAir levelBaru = AMAN;

  if (airTinggi) {
    levelBaru = BAHAYA;
  } else if (airSedang) {
    levelBaru = WASPADA;
  }

  if (levelBaru != levelSekarang) {
    LevelAir levelSebelumnya = levelSekarang;
    levelSekarang = levelBaru;
    tampilkan(levelBaru);

    // Mulai hitung 5 detik saat baru masuk WASPADA.
    if (levelBaru == WASPADA) {
      mulaiWaspada = millis();
    }

    // Setelah keluar dari BAHAYA, alarm siap untuk
    // kejadian BAHAYA berikutnya.
    if (levelSebelumnya == BAHAYA && levelBaru != BAHAYA) {
      alarmDiizinkan = true;
    }
  }

  if (matrix.displayAnimate()) {
    matrix.displayReset();
  }

  bool buzzerWaspada =
    levelSekarang == WASPADA &&
    alarmDiizinkan &&
    (millis() - mulaiWaspada < DURASI_BUZZER_WASPADA);

  bool alarmBahaya =
    levelSekarang == BAHAYA &&
    alarmDiizinkan;

  // WASPADA: hanya buzzer selama 5 detik.
  // BAHAYA: buzzer dan LED terus menyala sampai /alarm_off.
  digitalWrite(
    PIN_BUZZER,
    (buzzerWaspada || alarmBahaya) ? HIGH : LOW
  );
  digitalWrite(PIN_LED, alarmBahaya ? HIGH : LOW);

  delay(1);
}