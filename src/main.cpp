#include <Arduino.h>

const int relayPin = D1;    // Define the pin connected to the relay
const int onBoardLED = D4;  // GPIO2 pin is D4 in Arduino IDE

const int BUFFER_SIZE = 256; // Adjust buffer size as needed

char receivedChars[BUFFER_SIZE]; // Array to hold the received characters
boolean newData = false;

int parseValue(String input, String keyword) {
  int value = -1;
  int keywordIndex = input.indexOf(keyword);
  
  if (keywordIndex != -1) {
    int equalsIndex = input.indexOf('=', keywordIndex);
    int unitIndex = input.indexOf("ms", equalsIndex);
    if (unitIndex == -1) {
      unitIndex = input.indexOf("us", equalsIndex);
      if (unitIndex == -1) {
        unitIndex = input.indexOf("s", equalsIndex);
      }
    }

    if (equalsIndex != -1 && unitIndex != -1) {
      int valueStartIndex = equalsIndex + 1;
      String valueString = input.substring(valueStartIndex, unitIndex);
      value = valueString.toInt();
      if (input.substring(unitIndex, unitIndex + 2) == "ms") {
        value *= 1000; // Convert milliseconds to microseconds
      } else if (input.substring(unitIndex, unitIndex + 1) == "s") {
        value *= 1000000; // Convert seconds to microseconds
      }
    }
  }
  
  return value;
}

int convertToMS(int duration) {
  // Ensure the duration is in ms
  return duration / 1000;
}

void recvWithEndMarker() {
  static byte ndx = 0;
  char rc;

  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();

    if (rc != '\n' && rc != '\r') {
      receivedChars[ndx] = rc;
      ndx++;
      if (ndx >= BUFFER_SIZE) {
        ndx = BUFFER_SIZE - 1;
      }
    } else {
      receivedChars[ndx] = '\0'; // Null-terminate the string
      ndx = 0;
      newData = true;
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(relayPin, OUTPUT);
  pinMode(onBoardLED, OUTPUT);
  Serial.println("Setup started...");
}

void loop() {
  recvWithEndMarker();

  if (newData) {
    String input = String(receivedChars); // Read a line from serial
    newData = false;
    Serial.print("Received:");
    Serial.println(input);
    input.toLowerCase(); // Convert input to lowercase

    int startIndex = 0;
    int delimiterIndex; // Variable to store the delimiter index
    String segment;
    do {
      delimiterIndex = input.indexOf(",", startIndex);
      if (delimiterIndex != -1) {
          segment = input.substring(startIndex, delimiterIndex);
          startIndex = delimiterIndex + 1; // Move start index past the delimiter for the next iteration
      } else {
          // Last segment or no delimiter found
          segment = input.substring(startIndex);
      }

      int highDuration = parseValue(segment, "high");
      int lowDuration = parseValue(segment, "low");
      
      if (highDuration >= 0) {
        digitalWrite(relayPin, HIGH); // Set relay ON
        digitalWrite(onBoardLED, HIGH); // Turn on the onboard LED
        Serial.print("HIGH:"); Serial.println(convertToMS(highDuration));
        delay(convertToMS(highDuration));
      } else if (lowDuration >= 0) {
        digitalWrite(relayPin, LOW); // Set relay OFF
        digitalWrite(onBoardLED, LOW); // Turn off the onboard LED
        Serial.print("LOW:"); Serial.println(convertToMS(lowDuration));
        delay(convertToMS(lowDuration));
      }
    } while (delimiterIndex != -1);
    Serial.println("Done");
  }
  yield();
}
