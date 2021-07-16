# Mind the “Uuh” for Arduino Nano 33 BLE Sense

By [Benedikt Groß](https://benedikt-gross.de), [Maik Groß](https://twitter.com/thatsmaik), [Thibault Durand](http://thibault-durand.fr/)

<a href="https://www.youtube.com/watch?v=dL7eOMNSxFU">
<img width="1198" alt="youtube-preview" src="https://user-images.githubusercontent.com/480224/125923954-603a0654-5f60-4715-a07b-063f04d0fd37.png">
</a>

Mind the “Uuh” is an experimental training device helping everyone to become a better public speaker. The cute little compagnon is listening  to your speech aiming to make you aware of “uuh” fill words, because  these fillers are easy to avoid once you start noticing them. Now  everytime you give a presentation and you say “uuh” – you will be aware  :)

The prototype of Mind the “Uuh” was carefully designed as simple as  possible: There is a bell, a volume knob which controls how hard the  bell is hit or to turn it silent, a counter for your “uuh” stats and a  reset button. The product design is deliberately making references to a  classic alarm clock and a volume knob, to convey the nature of Mind the  “Uuh” intuitively.

For the detection of the “uuh”s the device runs a custom trained  machine learning model, trained on 1500 samples of various durations  from 300ms to 1 sec. This proof of concept model will notice distinctive “uuh” fillers but ignore very short utterances. All speech data is  processed directly on device, nothing is sent to the cloud.

Mind the “Uuh” is a fun project for Google’s [TensorFlow Microcontroller Challenge](https://experiments.withgoogle.com/tfmicrochallenge).


## 💻 Hardware pre-requisite (part list)

Part list:

- Microcontroller: Arduino Nano 33 BLE Sense
- Bell: we are using a standard bike bell for handle bars
- Servo: ???
- Display: ???
- Poti: ???
- Housing: 3d printed

<img width="917" alt="Schaltplan" src="https://user-images.githubusercontent.com/22634579/125507002-233a45cb-7864-49ac-b12a-1f4a21d7f469.png">


## 📦 Software pre-requisite

1. Add the Edge Impulse library through the Arduino IDE via: `Sketch` > `Include Library` > `Add .ZIP Library` ... see .zip file `ei-um-uh-ah-detector-...something.zip` in this repo.
2. Upload the `mind-the-uuh-arduino.ino` onto your Arduino.
3. Done :)


## 💪 Training your own model

The "uuh" model was completely trained with [Edge Impulse Studio](https://studio.edgeimpulse.com/) on a free account. If you want to train your own hotword, we recommend following carefully the super great [Responding to your voice](https://docs.edgeimpulse.com/docs/responding-to-your-voice) video tutorial. You will have to prepare ca. 1500 samples of 1 sec duration as mono .wav files of 16000 hz for the training.


## Acknowledgment

- Training of the model: [Edge Impulse Studio](https://studio.edgeimpulse.com/)
- Understanding how to train the model: Edge Impulse Tutorial [Responding to your voice](https://docs.edgeimpulse.com/docs/responding-to-your-voice)
- Web App: based on Edge Impulse example [Demo-Shower-Timer]( https://github.com/edgeimpulse/demo-shower-timer)
