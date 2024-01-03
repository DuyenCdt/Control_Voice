//Microphone - INMP441
#include "MicAudio.h"
#include "CloudSpeechClient.h"
#define button 22  //speech
// #define I2S_WS 26 //PIN_I2S_LRC 25 -ws
// #define I2S_SD 33 //PIN_I2S_DIN 33 - in
// #define I2S_SCK 27  //PIN_I2S_BCLK 32 - bck
CloudSpeechClient *cloudSpeechClient;

//Speaker - MAX98357A
#include "Audio.h"
#define I2S_DOUT 25
#define I2S_BCLK 27
#define I2S_LRC 26
Audio *speakerAudio;
bool isSpeaker = false;

// SD Card
#define SD_CS 5
#define SPI_MOSI 23
#define SPI_MISO 19
#define SPI_SCK 18

//ChatGPT
#include <HTTPClient.h>
#include <ArduinoJson.h>
const char *chatgpt_token = "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";  //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 //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
String micro = "";
String res = "";
//String Answer = "";

#define led 13
int i = 0;

//const char*
const char *mySpeech = "Xin chào tôi là Thaco";  //, hoặc CÔNG TY CỔ PHẦN TẬP ĐOÀN TRƯỜNG HẢI (TRUONG HAI GROUP) tiền thân là Công ty CP Ô tô Trường Hải (THACO) được thành lập vào ngày 29/04/1997, tại Đồng Nai. Người sáng lập là ông Trần Bá Dương, hiện là Chủ tịch Hội đồng Quản trị";
//String textSpeaker1 = "Xin chào tôi là Thaco, hoặc CÔNG TY CỔ PHẦN TẬP ĐOÀN TRƯỜNG HẢI (TRUONG HAI GROUP) tiền thân là Công ty CP Ô tô Trường Hải (THACO) được thành lập vào ngày 29/04/1997, tại Đồng Nai. Người sáng lập là ông Trần Bá Dương, hiện là Chủ tịch Hội đồng Quản trị";
//Trải qua 25 năm hình thành và phát triển, từ một công ty chuyên nhập khẩu xe cũ, cung cấp vật tư phụ tùng sửa chữa ô tô, THACO đã phát triển vượt bậc, đưa doanh nghiệp trở thành tập đoàn công nghiệp đa ngành gồm: THACO AUTO (Ô tô), THACO AGRI (Nông Lâm nghiệp); THACO INDUSTRIES (Cơ khí và Công nghiệp hỗ trợ), THADICO (Đầu tư xây dựng), THILOGI (Logistics) và THISO (Thương mại dịch vụ), trong đó các ngành bổ trợ cho nhau và có tính tích hợp cao.";

void setup() {


  DisConnectWifi();
  delete cloudSpeechClient;
  pinMode(button, INPUT);
  pinMode(led, OUTPUT);

  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  Serial.begin(115200);
  if (!SD.begin(SD_CS)) {
    Serial.println("Error talking to SD card!");
    while (true)
      ;  // end program
  }
  Serial.println("Talked to SD card!");
  //Serial.begin(115200);
  delay(1000);
  ConnectWifi();
  cloudSpeechClient = new CloudSpeechClient(USE_APIKEY);  //khai bao ket noi chatgpt

  // audio.connecttospeech("Xin chào tôi là Thaco", "vi");  // Google TTS
}

void loop() {

  if (digitalRead(button) == 0) {  //nhan nut de noi

    //////////////// INMP441 //////////////////////////////////////////////////////////////////////////////////////
    delete speakerAudio;
    isSpeaker = false;

    Serial.println("\r\nRecord start!\r\n");
    MicAudio *micAudio = new MicAudio(ICS43434);
    micAudio->Record();
    Serial.println("Recording Completed. Now Processing...");
    const char *result = cloudSpeechClient->Transcribe(micAudio);
    res = String(result);  //kết quả câu hỏi

    int len = res.length();
    res = res.substring(0, len);
    res = "\"" + res + "\"";
    //micro = res;

    Serial.print("Ask your Question : ");
    Serial.println(res);
    //////////////// INMP441 //////////////////////////////////////////////////////////////////////////////////////

    //////////////// Kết nối ChatGPT //////////////////////////////////////////////////////////////////////////////////////
    HTTPClient https;
    if (https.begin("https://api.openai.com/v1/chat/completions")) {  // HTTPS

      https.addHeader("Content-Type", "application/json");
      String token_key = String("Bearer ") + chatgpt_token;
      https.addHeader("Authorization", token_key);
      String payload = String("{\"model\":\"gpt-3.5-turbo\",\"messages\":[{\"role\":\"user\",\"content\":") + res + String("}],\"temperature\":0.7}");

      int httpCode = https.POST(payload);

      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = https.getString();

        DynamicJsonDocument doc(1024);


        deserializeJson(doc, payload);
        String Answer = doc["choices"][0]["message"]["content"];
        Answer = Answer.substring(0);
        Serial.print("Answer : ");
        Serial.println(Answer);
        // mySpeech = Answer.c_str();
        // Serial.print("mySpeech : ");
        // Serial.println(mySpeech);

      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }
      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }

    //////////////// Kết nối ChatGPT //////////////////////////////////////////////////////////////////////////////////////

    //////////////// Lọc câu hỏi //////////////////////////////////////////////////////////////////////////////////////
    //mySpeech = "Xin chào tôi là Thaco, hoặc CÔNG TY CỔ PHẦN TẬP ĐOÀN TRƯỜNG HẢI (TRUONG HAI GROUP) tiền thân là Công ty CP Ô tô Trường Hải (THACO) được thành lập vào ngày 29/04/1997, tại Đồng Nai";

    // String tatden = "tắt đèn";
    // String TatDen2 = "Tắt Đèn";

    // if (res == "bật đèn") {
    //   //digitalWrite(led, HIGH);
    //   Serial.println("đã bật đèn");
    // } else if (res.equalsIgnoreCase("Tắt Đèn")) {  //String(result) == "tắt đèn"
    //   //digitalWrite(led, LOW);
    //   Serial.println("đã tắt đèn");
    // }
    //////////////// Lọc câu hỏi //////////////////////////////////////////////////////////////////////////////////////
    res = "";
    delete micAudio;
    i = 1;
  }
  if (digitalRead(button) == 1) {
    delay(1);

    if (i == 0) {
      Serial.println("Press button");
      i = 2;
    }

    //////////////// MAX98357A //////////////////////////////////////////////////////////////////////////////////////
    if (i == 1) {
      Serial.println("\r\nSpeaker!\r\n");
      speakerAudio = new Audio(false, 3, I2S_NUM_0);
      speakerAudio->setVolume(15);
      speakerAudio->setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
      //const char* mySpeech = "Xin chào tôi là Thaco";
      if (micro.equalsIgnoreCase("gửi anh xa nhớ"))
        speakerAudio->connecttoFS(SD, "/test.mp3");  // SD
      else
        speakerAudio->connecttospeech(mySpeech, "vi");  // Google TTS
      //speakerAudio->connecttospeech("Xin chào tôi là Thaco", "vi");  // Google TTS

      isSpeaker = true;

      i = 0;
    }
    if (i == 2 && isSpeaker == true) {
      speakerAudio->loop();
      //audio.loop();
    }
    //////////////// MAX98357A //////////////////////////////////////////////////////////////////////////////////////
  }
}

void audio_info(const char *info) {
  Serial.print("info   ");
  Serial.println(info);
}
