/*
 * Copyright 2019 Smartbus main motor controller
 *
 * Licensed under the GNU Affero General Public License, Version 3.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      https://www.gnu.org/licenses/agpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#define IgnitionPin 2 //Ignition relay
#define ForwardPin 5 //Forward logic pin
#define ReversePin 6 //Reverse logic pin
#define WiperPin 9 //Wiper (speed control) pin

/*    Setup    */
#define PWM_MIN 20 //0-255 pwm value for min. speed
#define PWM_MAX 220 //0-255 pwm value for max. speed
/* ----------- */

boolean direction = true; //true = forward
uint8_t speedValue = 0;

/* Communication */
/*
All commands are similar to GCode
Main motor system prefix is: S0 (means system-0)

--------------------
Input:

S0 P[0-100]\n set speed

S0 M0\n	forward moving
S0 M1\n	reverse moving

S0 I0\n	switch off ignition
S0 I1\n	switch on ignition

--------------------
--------------------
Output:

S0 E0\n	reading error
S0 S1\n	successfully
--------------------
*/

void setup()
{
	pinMode(IgnitionPin, OUTPUT);
	pinMode(ForwardPin, OUTPUT);
	pinMode(ReversePin, OUTPUT);
	pinMode(WiperPin, OUTPUT);

	Serial.begin(9600);

}

void loop()
{

	SerialAction();

}

void SerialAction() { //Serial communication
	if (Serial.available() > 0) {
		String incoming = Serial.readStringUntil('\n');

		if (incoming.startsWith("S0")) {

			incoming.remove(0, 2);

			if (incoming.startsWith(" P")) { //Speed setup
				speedValue = SerialCommandConverter(incoming);
				runCheckAndPrint();
			}
			else if (incoming.startsWith(" M")) { //Mode setup
				uint8_t command = SerialCommandConverter(incoming);


				if (command == 0) {
					direction = 1;
					runCheckAndPrint();
				}
				else if (command == 1) {
					direction = 0;
					runCheckAndPrint();
				} else
					Serial.println("S0 E0"); //Reading error

			} else if (incoming.startsWith(" I")) { //Ignition state
				uint8_t command = SerialCommandConverter(incoming);

				if (command == 0) {
					digitalWrite(IgnitionPin, 0);
					Serial.println("S0 S1"); //successfully
				}
				else if (command == 1) {
					digitalWrite(IgnitionPin, 1);
					Serial.println("S0 S1"); //successfully
				}
				else
					Serial.println("S0 E0"); //Reading error
			}
			else
				Serial.println("S0 E0"); //Reading error
		}
	}
}

void runCheckAndPrint() {
	if (runMotor(direction, speedValue))
		Serial.println("S0 S1"); //successfully
	else {
		stopMotor();
		Serial.println("S0 E0"); //Reading error
	}
}

void stopMotor() {
	digitalWrite(WiperPin, 0);
	digitalWrite(ReversePin, 0);
	digitalWrite(ForwardPin, 0);
	speedValue = 0;
}

boolean runMotor(boolean forward, uint8_t speed) { //Motor Action
	if (forward) {
		digitalWrite(ReversePin, 0);
		digitalWrite(ForwardPin, 1);
	}
	else {
		digitalWrite(ForwardPin, 0);
		digitalWrite(ReversePin, 1);
	}

	if (speed <= 100 && speed >= 0) {
		analogWrite(WiperPin, map(speed, 0, 100, PWM_MIN, PWM_MAX));
		return 1;
	}
	else
		return 0;
}

int SerialCommandConverter(String incoming) { //Command converter
	incoming.remove(0, 2);
	return incoming.toInt();
}
