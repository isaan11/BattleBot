#include <Arduino.h>
#include <WiFi.h>
#include <ESP32Servo.h>

// FIREBASE LIBRARY
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h" // Provide the token generation process info.
#include "addons/RTDBHelper.h"	// Provide the RTDB payload printing info and other helper functions.

// firebase setup
#define API_KEY "AIzaSyCkUAmtVKA7bo052Lc9x4JQktUG61vrLpU"
#define DATABASE_URL "https://battlebot-3a88d-default-rtdb.asia-southeast1.firebasedatabase.app/"
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;

// wifi setup
#define WIFI_SSID "@wifi.id"
#define WIFI_PASSWORD "password"

unsigned long startMillis;
unsigned long curMillis;
String data;

// motor setup
const int rWheel1 = 21;
const int rWheel2 = 19;

const int lWheel1 = 23;
const int lWheel2 = 22;

const int rMotor = 25;
const int lMotor = 26;

// servo setup
const int tangan1 = 13;
const int tangan2 = 12;
Servo rHand;
Servo lHand;

int majumundur, kanankiri, angkatturun;
String move;

void getString(FirebaseData &);
void checkStream();
void maju();
void mundur();
void kanan();
void kiri();
void hentikan();
void naik();
void turun();

void setup(){
	Serial.begin(115200);

	pinMode(LED_BUILTIN, OUTPUT);
	pinMode(rWheel1, OUTPUT);
	pinMode(rWheel2, OUTPUT);
	pinMode(lWheel1, OUTPUT);
	pinMode(lWheel2, OUTPUT);
	pinMode(tangan1, OUTPUT);
	pinMode(tangan2, OUTPUT);

	rHand.attach(tangan1);
	lHand.attach(tangan2);
	rHand.write(90);
	lHand.write(0);

	// wifi connect
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
	Serial.print("Sedang Connect ke Wi-Fi");
	while (WiFi.status() != WL_CONNECTED){
		digitalWrite(LED_BUILTIN, HIGH);
		Serial.print(".");
		delay(250);
		digitalWrite(LED_BUILTIN, LOW);
		delay(250);
	}
	digitalWrite(LED_BUILTIN, HIGH);
	Serial.println();
	Serial.print("Terkoneksi dengan IP: ");
	Serial.println(WiFi.localIP());
	Serial.println();

	// firebase config
	Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
	config.api_key = API_KEY;

	if (Firebase.signUp(&config, &auth, "", "")){
		Serial.println("Firebase Sudah Jalan");
		digitalWrite(LED_BUILTIN, LOW);
		signupOK = true;
	}
	else{
		String firebaseErrorMessage = config.signer.signupError.message.c_str();
		Serial.printf("%s\n", firebaseErrorMessage);
	}

	// start firebase
	config.database_url = DATABASE_URL;
	config.token_status_callback = tokenStatusCallback;

	Firebase.begin(&config, &auth);
	Firebase.reconnectWiFi(true);

	fbdo.setBSSLBufferSize(1024, 1024);
	fbdo.setResponseSize(1024);

	if (!Firebase.RTDB.beginStream(&fbdo, "/")){
		Serial.println(fbdo.errorReason());
	}

	startMillis = 0;

	// fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);
}

void loop(){
	checkStream();
	move.replace("\\", "");
	move.replace("\"", "");

	int index1 = move.indexOf('#');
	int index2 = move.indexOf('#', index1 + 1);

	String data1, data2, data3;
	data1 = move.substring(0, index1);
	data2 = move.substring(index1 + 1, index2);
	data3 = move.substring(index2 + 1);

	majumundur = data1.toInt();
	kanankiri = data2.toInt();
	angkatturun = data3.toInt();

	if (majumundur == 1){
		maju();
	}
	else if (majumundur == 2){
		mundur();
	}
	else
		hentikan();

	if (kanankiri == 1){
		kanan();
	}
	else if (kanankiri == 2){
		kiri();
	}

	if (angkatturun == 1){
		naik();
	}
	else if (angkatturun == 2){
		turun();
	}
}

void maju(){
	analogWrite(rMotor, 100);
	analogWrite(lMotor, 100);

	digitalWrite(rWheel1, HIGH);
	digitalWrite(rWheel2, LOW);

	digitalWrite(lWheel1, HIGH);
	digitalWrite(lWheel2, LOW);
}

void mundur(){
	analogWrite(rMotor, 50);
	analogWrite(lMotor, 50);

	digitalWrite(rWheel1, LOW);
	digitalWrite(rWheel2, HIGH);

	digitalWrite(lWheel1, LOW);
	digitalWrite(lWheel2, HIGH);
}

void kanan(){
	analogWrite(rMotor, 70);
	analogWrite(lMotor, 70);

	digitalWrite(rWheel1, LOW);
	digitalWrite(rWheel2, LOW);

	digitalWrite(lWheel1, HIGH);
	digitalWrite(lWheel2, LOW);
}

void kiri(){
	analogWrite(rMotor, 70);
	analogWrite(lMotor, 70);

	digitalWrite(rWheel1, HIGH);
	digitalWrite(rWheel2, LOW);

	digitalWrite(lWheel1, LOW);
	digitalWrite(lWheel2, LOW);
}

void hentikan(){
	digitalWrite(rWheel1, LOW);
	digitalWrite(rWheel2, LOW);

	digitalWrite(lWheel1, LOW);
	digitalWrite(lWheel2, LOW);
}

void naik(){
	rHand.write(0);
	lHand.write(90);
}

void turun(){
	rHand.write(90);
	lHand.write(0);
}

void checkStream(){
	if (!Firebase.RTDB.readStream(&fbdo)){
		Serial.println(fbdo.errorReason());
	}
	if (fbdo.streamTimeout()){
		Serial.println("Timed out, resuming...");
		Serial.println();
	}
	if (fbdo.streamAvailable()){
		getString(fbdo);
	}
}

void getString(FirebaseData &data){
	if (fbdo.dataTypeEnum() == firebase_rtdb_data_type_string){
		move = fbdo.to<String>();
	}
}