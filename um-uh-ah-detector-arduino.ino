//MAIK CODE
//General-----------------

int uhm_count = 0;

//Button

const int BUTTON_PIN = 12;
int currentState;
int lastState = HIGH;
bool pressed = false;

//Poti
int potPin = 6;
int potiVal = 0;

//Seven Segment Display
#include "SevSegShift.h"

#define SHIFT_PIN_SHCP 10
#define SHIFT_PIN_STCP 9
#define SHIFT_PIN_DS   6

/* Instantiate a seven segment controller object with:
  - segment pins controlled via 1 shift register and
  - digit pins connected to the Arduino directly
*/
SevSegShift sevseg(SHIFT_PIN_DS, SHIFT_PIN_SHCP, SHIFT_PIN_STCP, 1, true);


//Servo
#include <Servo.h>

bool run_servo = false;

class BellServo
{
    Servo servo;              // the servo
    int pos;              // current servo position
    int increment;        // increment to move for each interval
    unsigned long  updateInterval;      // interval between updates
    unsigned long lastUpdate; // last update of position
    bool cycle_ended = false;
    unsigned long cycleInterval = 1000;
    unsigned long lastCycleUpdate = 0;

  public:
    BellServo(unsigned long interval)
    {
      updateInterval = interval;
      increment = 1; //20
    }

    void Attach(int pin)
    {
      servo.attach(pin);
    }

    void Detach()
    {
      servo.detach();
    }

    void Update()
    {
      if (run_servo && potiVal > 600) {

        if ((millis() - lastUpdate) > updateInterval) {
          lastUpdate = millis();
          servo.write(80);
        } else {
          servo.write(0);
        }

        if ((millis() - lastCycleUpdate) > cycleInterval) {
          lastCycleUpdate = millis();
          run_servo = false;
        }
        /*
        if ((millis() - lastUpdate) > updateInterval) {
          lastUpdate = millis();
          pos += increment;
          servo.write(0);
          //Serial.println(pos);
          if (pos >= 0) // end of sweep
          {
            // reverse direction
            increment = -increment;
            cycle_ended = true;
            servo.write(80);
          }
          if (pos <= 0 && cycle_ended == true) // end of sweep
          {
            // reverse direction
            increment = -increment;
            run_servo = false;
            cycle_ended = false;
          }
        }*/
        
      } else {
        lastCycleUpdate = millis();
      }
    }
};

BellServo bellservo(1000);

//--------------------------

// LEDs
#define PIN_LED     (13u)
#define LED_BUILTIN PIN_LED
#define LEDR        (22u)
#define LEDG        (23u)
#define LEDB        (24u)
#define LED_PWR     (25u)

// If your target is limited in memory remove this macro to save 10K RAM
#define EIDSP_QUANTIZE_FILTERBANK   0

/**
 * Define the number of slices per model window. E.g. a model window of 1000 ms
 * with slices per model window set to 4. Results in a slice size of 250 ms.
 * For more info: https://docs.edgeimpulse.com/docs/continuous-audio-sampling
 */
#define EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW 3

/* Includes ---------------------------------------------------------------- */
#include <PDM.h>
#include <um-uh-ah-detector-3-center_curated_fillers_02_inferencing.h>

/** Audio buffers, pointers and selectors */
typedef struct {
    signed short *buffers[2];
    unsigned char buf_select;
    unsigned char buf_ready;
    unsigned int buf_count;
    unsigned int n_samples;
} inference_t;

static inference_t inference;
static bool record_ready = false;
static signed short *sampleBuffer;
static bool debug_nn = false; // Set this to true to see e.g. features generated from the raw signal
static int print_results = -(EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW);

/**
 * @brief      Arduino setup function
 */
void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);

    //---------MAIK----------------
    setupSevsegDisplay();
    setupServo();
    setupButton();
    //-----------------------------

    Serial.println("Edge Impulse Inferencing Demo");

    // summary of inferencing settings (from model_metadata.h)
    ei_printf("Inferencing settings:\n");
    ei_printf("\tInterval: %.2f ms.\n", (float)EI_CLASSIFIER_INTERVAL_MS);
    ei_printf("\tFrame size: %d\n", EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
    ei_printf("\tSample length: %d ms.\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT / 16);
    ei_printf("\tNo. of classes: %d\n", sizeof(ei_classifier_inferencing_categories) /
                                            sizeof(ei_classifier_inferencing_categories[0]));

    run_classifier_init();
    if (microphone_inference_start(EI_CLASSIFIER_SLICE_SIZE) == false) {
        ei_printf("ERR: Failed to setup audio sampling\r\n");
        return;
    }
}

/**
 * @brief      Arduino main function. Runs the inferencing loop.
 */
void loop()
{
    //-------------MAIK-------------
    displayLoop();
    servoLoop();
    buttonLoop();

    potiVal = analogRead(potPin);
    //------------------------------

    
    bool m = microphone_inference_record();
    if (!m) {
        ei_printf("ERR: Failed to record audio...\n");
        return;
    }

    signal_t signal;
    signal.total_length = EI_CLASSIFIER_SLICE_SIZE;
    signal.get_data = &microphone_audio_signal_get_data;
    ei_impulse_result_t result = {0};

    EI_IMPULSE_ERROR r = run_classifier_continuous(&signal, &result, debug_nn);
    if (r != EI_IMPULSE_OK) {
        ei_printf("ERR: Failed to run classifier (%d)\n", r);
        return;
    }

    if (++print_results >= (EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW)) {
      
        // flash LED if filler is detected
        float filler_classification = result.classification[0].value;
        if (filler_classification > 0.6) {
          Serial.println("FILLER DETECTED");
          run_servo = true;
          digitalWrite(LEDR, LOW);
          digitalWrite(LEDG, LOW);
          digitalWrite(LEDB, LOW);
        } else {
          digitalWrite(LEDR, HIGH);
          digitalWrite(LEDG, HIGH);
          digitalWrite(LEDB, HIGH);
        }
        
        
        // print the predictions
        ei_printf("Predictions ");
        ei_printf("( DSP: %d ms., Classification: %d ms., Anomaly: %d ms. )",
            result.timing.dsp, result.timing.classification, result.timing.anomaly);
        ei_printf(": \n");
        for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
            ei_printf("    %s: %.5f\n", result.classification[ix].label,
                      result.classification[ix].value);
        }
        
#if EI_CLASSIFIER_HAS_ANOMALY == 1
        ei_printf("    anomaly score: %.3f\n", result.anomaly);
#endif

        print_results = 0;
    }
}

/**
 * @brief      Printf function uses vsnprintf and output using Arduino Serial
 *
 * @param[in]  format     Variable argument list
 */
void ei_printf(const char *format, ...) {
    static char print_buf[1024] = { 0 };

    va_list args;
    va_start(args, format);
    int r = vsnprintf(print_buf, sizeof(print_buf), format, args);
    va_end(args);

    if (r > 0) {
        Serial.write(print_buf);
    }
}

/**
 * @brief      PDM buffer full callback
 *             Get data and call audio thread callback
 */
static void pdm_data_ready_inference_callback(void)
{
    int bytesAvailable = PDM.available();

    // read into the sample buffer
    int bytesRead = PDM.read((char *)&sampleBuffer[0], bytesAvailable);

    if (record_ready == true) {
        for (int i = 0; i<bytesRead>> 1; i++) {
            inference.buffers[inference.buf_select][inference.buf_count++] = sampleBuffer[i];

            if (inference.buf_count >= inference.n_samples) {
                inference.buf_select ^= 1;
                inference.buf_count = 0;
                inference.buf_ready = 1;
            }
        }
    }
}

/**
 * @brief      Init inferencing struct and setup/start PDM
 *
 * @param[in]  n_samples  The n samples
 *
 * @return     { description_of_the_return_value }
 */
static bool microphone_inference_start(uint32_t n_samples)
{
    inference.buffers[0] = (signed short *)malloc(n_samples * sizeof(signed short));

    if (inference.buffers[0] == NULL) {
        return false;
    }

    inference.buffers[1] = (signed short *)malloc(n_samples * sizeof(signed short));

    if (inference.buffers[0] == NULL) {
        free(inference.buffers[0]);
        return false;
    }

    sampleBuffer = (signed short *)malloc((n_samples >> 1) * sizeof(signed short));

    if (sampleBuffer == NULL) {
        free(inference.buffers[0]);
        free(inference.buffers[1]);
        return false;
    }

    inference.buf_select = 0;
    inference.buf_count = 0;
    inference.n_samples = n_samples;
    inference.buf_ready = 0;

    // configure the data receive callback
    PDM.onReceive(&pdm_data_ready_inference_callback);

    // optionally set the gain, defaults to 20
    PDM.setGain(80);

    PDM.setBufferSize((n_samples >> 1) * sizeof(int16_t));

    // initialize PDM with:
    // - one channel (mono mode)
    // - a 16 kHz sample rate
    if (!PDM.begin(1, EI_CLASSIFIER_FREQUENCY)) {
        ei_printf("Failed to start PDM!");
    }

    record_ready = true;

    return true;
}

/**
 * @brief      Wait on new data
 *
 * @return     True when finished
 */
static bool microphone_inference_record(void)
{
    bool ret = true;

    if (inference.buf_ready == 1) {
        ei_printf(
            "Error sample buffer overrun. Decrease the number of slices per model window "
            "(EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW)\n");
        ret = false;
    }

    while (inference.buf_ready == 0) {
        delay(1);
    }

    inference.buf_ready = 0;

    return ret;
}

/**
 * Get raw audio signal data
 */
static int microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr)
{
    numpy::int16_to_float(&inference.buffers[inference.buf_select ^ 1][offset], out_ptr, length);

    return 0;
}

/**
 * @brief      Stop PDM and release buffers
 */
static void microphone_inference_end(void)
{
    PDM.end();
    free(inference.buffers[0]);
    free(inference.buffers[1]);
    free(sampleBuffer);
}

#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_MICROPHONE
#error "Invalid model for current sensor."
#endif

//---------MAIK--------------//
//---------------------------//
//---SEVEN SEGMENT DISPLAY---//

void setupSevsegDisplay() {

  byte numDigits = 4;
  byte digitPins[] = {2, 3, 4, 5}; //{2, 3, 4, 5}; // These are the PINS of the ** Arduino **
  byte segmentPins[] = {1, 2, 3, 4, 5, 6, 7}; // these are the PINs of the ** Shift register **
  bool resistorsOnSegments = false; // 'false' means resistors are on digit pins
  byte hardwareConfig = COMMON_CATHODE; // See README.md for options
  bool updateWithDelays = false; // Default 'false' is Recommended
  bool leadingZeros = false; // Use 'true' if you'd like to keep the leading zeros
  bool disableDecPoint = true; // Use 'true' if your decimal point doesn't exist or isn't connected

  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments, updateWithDelays, leadingZeros, disableDecPoint);
  sevseg.setBrightness(90);
}

void displayLoop() {
  sevseg.setNumber(uhm_count, 0);
  sevseg.refreshDisplay(); // Must run repeatedly
}

//---------------------------//
//------SERVO MOTOR----------//

void setupServo() {
  bellservo.Attach(11); // attaches the servo on pin 9 to the servo object
}

void servoLoop() {
  bellservo.Update();
}

//---------------------------//
//------BUTTON---------------//

void setupButton() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void buttonLoop() {
  currentState = digitalRead(BUTTON_PIN);

  if (currentState == LOW && pressed == false) {
    Serial.println("Button pressed!");
    uhm_count++;
    run_servo = true;
    pressed = true;
  }
  else if (currentState == HIGH && pressed == true) {
    pressed = false;
  }
}
