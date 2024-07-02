#include <WiFi.h>
#include <FirebaseESP32.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

#define FIREBASE_HOST ""
#define FIREBASE_AUTH ""

// Daftar Variabel
bool statusRelay;
const int relayPin = 21;
String relayOnTime = "";
unsigned long serverTimestamp = 0;
String currentTime = "";
float totalWaterFlow;

// Konfigurasi Firebase
FirebaseData firebaseData;
FirebaseConfig config;
FirebaseAuth auth;
FirebaseJson json;

void setup()
{
  Serial.begin(9600);

  // Menghubungkan ke WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi....");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Connected to WiFi. IP: ");
  Serial.println(WiFi.localIP());

  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  pinMode(relayPin, OUTPUT);

  digitalWrite(relayPin, LOW);

  json.set(".sv", "timestamp");

  if (Firebase.ready())
  {
    if (Firebase.set(firebaseData, "/monitoring/timestamp", json))
    {
      Serial.println("Timestamp initialized in Firebase");
    }
    else
    {
      Serial.print("Error initializing timestamp: ");
      Serial.println(firebaseData.errorReason());
    }
  }
}

void loop()
{
  handleRelayControl();
  handleFirebaseData();
  delay(1000);
}

void handleFirebaseData()
{
  if (Firebase.get(firebaseData, "/monitoring/status_pompa"))
  {
    statusRelay = firebaseData.boolData(); // Use boolData() instead of stringData()
    Serial.print("Status Pompa dari Firebase: ");
    Serial.println(statusRelay);
  }
  else
  {
    Serial.print("Error getting status_pompa: ");
    Serial.println(firebaseData.errorReason());
  }
  if (Firebase.get(firebaseData, "/monitoring/jadwal_pompa"))
  {
    relayOnTime = firebaseData.stringData();
    Serial.print("Jadwal Aktif Pompa: ");
    Serial.println(relayOnTime);
  }
  else
  {
    Serial.print("Error getting jadwal_pompa: ");
    Serial.println(firebaseData.errorReason());
  }
  if (Firebase.get(firebaseData, "/monitoring/total_water_flow"))
  {
    totalWaterFlow = firebaseData.floatData();
    Serial.print("Total Water Flow: ");
    Serial.println(totalWaterFlow);
  }
  else
  {
    Serial.print("Error getting jadwal_pompa: ");
    Serial.println(firebaseData.errorReason());
  }
}

void handleRelayControl()
{
  if (relayOnTime == currentTime || statusRelay == true)
  {
    digitalWrite(relayPin, HIGH);
    statusRelay = true;
    //    Firebase.setInt(firebaseData, "/monitoring/status_pompa", statusRelay);
    Serial.println("Status Pompa updated successfully");
  }
  else if (statusRelay == false)
  {
    digitalWrite(relayPin, LOW);
  }
  if (statusRelay == true && totalWaterFlow >= 10)
  {
    digitalWrite(relayPin, LOW);
    statusRelay = false;
    Firebase.setInt(firebaseData, "/monitoring/status_pompa", statusRelay);
    Serial.println("Relay OFF because totalMilliliters reached 10.");
  }

  Serial.print("Status Aktif Pompa: ");
  Serial.println(statusRelay);
}
