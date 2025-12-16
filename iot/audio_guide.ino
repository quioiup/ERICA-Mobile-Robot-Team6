#define ENABLE_USER_AUTH
#define ENABLE_DATABASE

#include <FirebaseClient.h>
#include "ExampleFunctions.h"

#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

#define API_KEY ""
#define USER_EMAIL ""
#define USER_PASSWORD ""
#define DATABASE_URL ""

SSL_CLIENT ssl_client;
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client);

UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD, 3000);
FirebaseApp app;
RealtimeDatabase Database;

bool firebase_ready = false;

SoftwareSerial dfSerial(5, 4);
DFRobotDFPlayerMini dfPlayer;

#define LEFT_EN 12
#define RIGHT_EN 13

int last_code = 0;

void setup() {
    Serial.begin(115200);
    dfSerial.begin(9600);

    pinMode(LEFT_EN, OUTPUT);
    pinMode(RIGHT_EN, OUTPUT);
    digitalWrite(LEFT_EN, LOW);
    digitalWrite(RIGHT_EN, LOW);

    Serial.println("\nBooting...");

    Serial.println("Connecting to WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(300);
    }
    Serial.println("\nWiFi connected!");

    set_ssl_client_insecure_and_buffer(ssl_client);

    initializeApp(aClient, app, getAuth(user_auth), auth_debug_print, "authTask");
    app.getApp<RealtimeDatabase>(Database);
    Database.url(DATABASE_URL);

    Serial.println("Firebase init complete!");

    Serial.println("Initializing DFPlayer...");
    delay(2500);

    if (!dfPlayer.begin(dfSerial)) {
        Serial.println("DFPlayer init failed");
        while (true);
    }

    dfPlayer.volume(25);
    Serial.println("DFPlayer Ready!");
}

void loop() {
    app.loop();

    if (app.ready() && !firebase_ready) {
        firebase_ready = true;
        Serial.println("Firebase Ready!");
    }

    if (!firebase_ready)
        return;

    int code = Database.get<int>(aClient, "/events/speaker");

    if (aClient.lastError().code() != 0) {
        Serial.print("Read ERROR: ");
        Serial.println(aClient.lastError().message().c_str());
        delay(1000);
        return;
    }

    Serial.print("speaker code = ");
    Serial.println(code);

    if (code != last_code) {
        if (code == 1) {
            digitalWrite(LEFT_EN, HIGH);
            digitalWrite(RIGHT_EN, LOW);
            dfPlayer.play(1);
            Serial.println("PLAY LEFT (track 1)");
        } else if (code == 2) {
            digitalWrite(LEFT_EN, LOW);
            digitalWrite(RIGHT_EN, HIGH);
            dfPlayer.play(2);
            Serial.println("PLAY RIGHT (track 2)");
        } else {
            digitalWrite(LEFT_EN, LOW);
            digitalWrite(RIGHT_EN, LOW);
            dfPlayer.stop();
            Serial.println("STOP (invalid code)");
        }
        last_code = code;
    }

    delay(300);
}
