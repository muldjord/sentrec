# SentRec
A sentence recorder specifically catered to Piper TTS training recordings.

Note that this project is currently in heavy development. Code might not even compile at this stage. It might not even function in any meaningful way. I will update this notice when the software reaches a state where it is somewhat useful.

The aim is to create an audio recorder catered to the workflow required when recording a large sentence dataset (text + audio) that can be used to train text to speech models. This software is specifically designed to work with the Piper Text-To-Speech format of `ID|Sentence` lines in a CSV file (with `|` being the delimiter) with corresponding `wav/ID.wav` files.

However this software is only one part of the equation. To record a useful dataset a text sentence corpus also needs to be acquired with a meaningful distribution of sentences. For this purpose I am also working on a sentence extractor that can take basically any large text files as input and extract sentences of varying length and formats from it.

With that dataset acquired it is then a straight-forward, but timeconsuming, process of using SentRec to record all of the sentences. But SentRec will at least help you ease that process by providing a minimum-mouse-clicks workflow that includes automatically normalizing and cutting silence from the recorded sentences. In other words, the output will be directly usable to train a Piper TTS voice.

## Building SentRec
### Pre-requisites
```
$ sudo apt install build-essential git cmake qt6-base-dev qt6-tools-dev qt6-multimedia-dev libsndfile-dev
```

### Compile
```
$ mkdir build
$ cd build
$ cmake ..
$ make
$ make install
```
The SentRec executable and peripheral files are installed in `sentrec/release`.