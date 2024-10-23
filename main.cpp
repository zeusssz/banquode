#include <Servo.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

const int LMotorPin = 9;
const int RMotorPin = 10;
Servo LMotor;
Servo RMotor;

int baseSpeed = 1500;  // NTRL throttle signal (thanks google)
int differential = 200; // TODO: math out the irl thing

TinyGPSPlus gps;
SoftwareSerial gpsSerial(4, 3); // RX, TX pins for GPS comms

// latlong
struct Waypoint {
  double lat;
  double lon;
};

// get actual points
Waypoint waypoints[] = {
  {00.0001, -00.0002}, // Point A
  {00.0003, -00.0004}, // Point B
  {00.0005, -00.0006}, // Point C
  {0.0007, -0.0007}, // Point D
};
int totalWaypoints = sizeof(waypoints) / sizeof(Waypoint);
int currentWaypoint = 0;

bool rageflight_mode = false;

void rageflight(double currentLat, double currentLon) {
  double latDiff = waypoints[currentWaypoint].lat - currentLat;
  double lonDiff = waypoints[currentWaypoint].lon - currentLon;

  int LMotorSpeed = baseSpeed;
  int RMotorSpeed = baseSpeed;

  if (latDiff > 0) {
    RMotorSpeed += 50; // turn R
  } else {
    LMotorSpeed += 50; // turn L
  }

  if (lonDiff > 0) {
    RMotorSpeed -= 50; // move L
  } else {
    LMotorSpeed -= 50; // move R
  }

  LMotor.writeMicroseconds(LMotorSpeed);
  RMotor.writeMicroseconds(RMotorSpeed);

  // BIRD (check if it reached the current waypoint)
  if (abs(latDiff) < 0.0001 && abs(lonDiff) < 0.0001) {
    currentWaypoint = (currentWaypoint + 1) % totalWaypoints; // cycler
  }
}

void antwalk() {
  int LMotorSpeed = baseSpeed - differential;  // slower L motor
  int RMotorSpeed = baseSpeed + differential; // faster R motor

  LMotor.writeMicroseconds(LMotorSpeed);
  RMotor.writeMicroseconds(RMotorSpeed);
}

void setup() {
  LMotor.attach(LMotorPin);
  RMotor.attach(RMotorPin);

  // initialise motors
  LMotor.writeMicroseconds(baseSpeed);
  RMotor.writeMicroseconds(baseSpeed);

  delay(2000); // motor initialization

  gpsSerial.begin(9600); // initialise GPS comms
}

void loop() {
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  if (rageflight_mode) {
    if (gps.location.isValid()) {
      double currentLat = gps.location.lat();
      double currentLon = gps.location.lng();
      rageflight(currentLat, currentLon);
    }
  } else {
    antwalk();
  }

  delay(20);
}
