
#include <Arduino.h>
#include <YA_FSM.h>

const byte BTN_CALL = 2;
const byte LED_GREEN = 3;
const byte LED_YELLOW = 4;
const byte LED_RED = 5;


// Create new FSM (num States, num Transition)
YA_FSM stateMachine(4, 4);

// Input Alias. We use Input as trigging condition in defined transition
enum Input {StartGreen, StartYellow, StartRed};

// State Alias
enum State {RED, GREEN, YELLOW, CALL};

// Helper for print labels instead integer when state change
const char* stateName[] = { "RED", "GREEN", "YELLOW", "CALL"};

// Stores last user input and the current active state
Input input;

// Pedestrian traffic light -> green ligth ON until button pressed
// #define GREEN_TIME  20000    // always ON
#define YELLOW_TIME  4000  		  // 4s
#define RED_TIME 10000  		    // 10s
#define CALL_DELAY 2000			    // 2s


void setup() 
{
	pinMode(BTN_CALL, INPUT_PULLUP);
	pinMode(LED_GREEN, OUTPUT);
	pinMode(LED_YELLOW, OUTPUT);
	pinMode(LED_RED, OUTPUT);

	Serial.begin(115200);
	Serial.println(F("Starting State Machine...\n"));
	setupStateMachine();	

	// Initial state
	Serial.print(F("Active state: "));
	Serial.println(stateName[stateMachine.GetState()]);
}

void loop() 
{

	// Update State Machine	(true is state changed)
	if(stateMachine.Update()){
		Serial.print(F("Active state: "));
		Serial.println(stateName[stateMachine.GetState()]);
	}
	
}



void setLight(bool light[]){
	digitalWrite(LED_GREEN, light[0]);
	digitalWrite(LED_YELLOW, light[1]);
	digitalWrite(LED_RED, light[2]);
}


/////////// STATE MACHINE FUNCTIONS //////////////////


// Define "on entering" callback functions
void onEnteringGreen(){
	Serial.println(F("Green light ON"));
}

void onEnteringYellow(){
	Serial.println(F("Yellow light ON"));
}

void onEnteringRed(){
	Serial.println(F("Red light ON"));
}

// Define "on leaving" callback functions
void onLeavingGreen(){
	Serial.println(F("Green light OFF\n"));
}

void onLeavingYellow(){
	Serial.println(F("Yellow light OFF\n"));
}

void onLeavingRed(){
	Serial.println(F("Red light OFF\n"));
}


// Setup the State Machine
void setupStateMachine()
{
	// Add transitions with related callback functions
	///// ---- Be carefull, total number of transitions MUST be as declared -------  ///////
	stateMachine.AddTransition(RED, GREEN, [](){return input == Input::StartGreen;}	);
	stateMachine.AddTransition(YELLOW, RED, [](){return input == Input::StartRed;}  );	
	stateMachine.AddTransition(CALL, YELLOW, [](){return input == Input::StartYellow;});

	// In this example we check the button state in the callback function
	// Here debounce delay (or others type) is not more needed because we check button only during GREEN state
	// After this, state will change and so, the function will not be called anymore
	stateMachine.AddTransition(GREEN, CALL,
		[](){
			if(digitalRead(BTN_CALL) == LOW){					
				Serial.println(F("Start button pressed"));
				return true;
			}		
			return false;	
		});	

	// Add on enetering and on leaving actions (previuos defined)
	stateMachine.SetOnEntering(GREEN, onEnteringGreen);
	stateMachine.SetOnEntering(YELLOW, onEnteringYellow);
	stateMachine.SetOnEntering(RED, onEnteringRed);

	// On leavine callback functions ( inline)
	stateMachine.SetOnLeaving(GREEN, onLeavingGreen);
	stateMachine.SetOnLeaving(YELLOW, onLeavingYellow);
	stateMachine.SetOnLeaving(RED, onLeavingRed);

	// Add the onState callback function. This will be excuted while the state is active

	// State GREEN active 
	// Pedestrian traffic light -> green ligth ON untile button pressed
	stateMachine.SetOnState(GREEN,	
		[](){ 
			bool _ligth[] = {1, 0, 0 };
			setLight(_ligth);
		});

	// State YELLOW active (with timeout)
	stateMachine.SetOnState(YELLOW, 
		[](){ 
			bool _ligth[] = {0, 1, 0 };
			setLight(_ligth);

			// Check if current state has timeouted, if yes go to next
			bool timeout = stateMachine.GetTimeout( stateMachine.GetState() ); 
			if( timeout) {
				input = Input::StartRed;	
			}
		},
		YELLOW_TIME );

	// State RED active (with timeout)
	stateMachine.SetOnState(RED, 
		[](){ 
			bool _ligth[] = {0, 0, 1 };
			setLight(_ligth);
			
			// Check if current state has timeouted, if yes go to next 
			if( stateMachine.GetTimeout( stateMachine.GetState() ) ) {
				input = Input::StartGreen;
			}
		},
		// This state has timeout
		RED_TIME );

	// State CALL active (with timeout)
	stateMachine.SetOnState(CALL, 
		[](){ 
			// Check if current state has timeouted, if yes go to next
			if( stateMachine.GetTimeout( stateMachine.GetState() ) ) {
				input = Input::StartYellow;
			}
		},
		CALL_DELAY );

	
}
