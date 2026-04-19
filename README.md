# SentRec
A sentence recorder specifically catered to Piper TTS training recordings.

![SentRec v0.7.3 screenshot](https://github.com/muldjord/sentrec/blob/main/icons/screenshot.png?raw=true)

Note that this project is currently undergoing heavy development! It is usable for the intended purpose described below. But keep in mind that it is in a somewhat rough state and might contain bugs.

SentRec is an audio recorder catered to an optimal workflow when recording a large sentence dataset (text + audio) that can be used to train text to speech models. SentRec is specifically designed to work with the Piper Text-To-Speech format of `ID|Sentence` lines in a CSV file (with `|` being the delimiter) with corresponding `wav/ID.wav` files.

However this software is only one part of the equation. To record a useful dataset a text sentence corpus also needs to be acquired with a meaningful distribution of sentences. For this purpose I am also working on a sentence extractor that can take basically any large text files as input and extract sentences of varying length and formats from it.

With that dataset acquired it is then a straight-forward, but timeconsuming, process of using SentRec to record all of the sentences. But SentRec will at least help you ease that process by providing an optimized workflow that includes automatically normalizing and cutting silence from the recorded sentences. The resulting recordings will be directly usable to train a Piper TTS voice.

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

### Running
Just run the `sentrec/release/SentRec` executable.

## Using SentRec
The topmost part of the screen is the `Sentence viewer`. The first thing you want to do is to load your sentences from a Piper-formatted CSV file into this view using the `Load sentences...` button on the right side.
This will load all sentences into the viewer and enable the `Audio recorder` at the bottom of the screen. If you have previously recorded some of the sentences SentRec will automatically select the first sentence it could not find a matching wav file for. You can then continue your work from there.

When the `Audio recorder` is enabled you should check and make sure that your audio device and desired samplerate is selected. It is very important that these settings are the same between sessions. Otherwise you will have mismatched wav files. SentRec saves these settings and should load them when you start the software later. But if the id of your interface has changed for some weird reason, it will revert to the default input interface. So make sure you check and set this correctly!

I usually record in 44100 Hz. Piper wants wav files in either 22050 Hz (medium and high quality voices) or 16000 Hz (low quality voice). You could just use 22050 Hz for everything you record, but I like to keep the samplerate higher in case I want to use the dataset for something else later on. You can always downsample. You cannot upsample (well you can, but you will loose quality).

SentRec records and saves samples as 32bit floating point values. This is not configurable! If your audio interface does not support it you will have problems. Most modern interfaces should have no issue though. Not making this configurable made the implementation of filters easier and ensures that there is enough resolution for amplifying the wav files later should you wish to do so.

Now we are ready to record! The first sentence will be selected and it is simply a matter of clicking the `Record` button. Always remember to wait a second before starting to speak the sentence after clicking `Record`! SentRec trims the audio recording automatically (unless you disable it) and it uses the first 20 ms of audio to determine an average noise floor. So it is VERY important that you are quiet when clicking `Record`!

When you've spoken the sentence click the `Stop` button (same as the `Record` button). SentRec will immediately process the audio and render the resulting audio into the `Waveform view`. At this point the audio has also been saved as a wav file in the `wav` subdirectory (relative to you CSV file). You now have the option of either playing back the audio using the `Play` button or clicking `Next` to move on to the next sentence.

If this is your first session I would always recommend recording a few sentences and then opening up the `wav` subdirectory and play some of the wav files. Just to make sure everything is the way you want it to be. Check that the wav files have the correct samplerate and bitrate. Check that they play fine in any other audio playing software.

Then continue to recording sentences until you have recorded all sentences from your CSV file. When you reach the end remember to celebrate! Recording sentences for TTS voice training is A LOT of work!

## F.A.Q
### Q: What is a piper-formatted CSV?
A Piper-formatted CSV file is usually called `metadata.csv` and contains `|`-separated columns of `ID|Sentence`. The `ID` corresponds to the connected wav file found in a `wav` subfolder (you don't have these, recording them is what SentRec is for). The `Sentence` is the exact sentence you should record using SentRec for that particular line. Remember to speak clearly for the best results!

### Q: Do I have to record all sentences in one session?
NO! As long as you keep your recording setup stable, you can quit SentRec at any time and come back and continue recording later. SentRec saves recorded wav files and sentence changes as they happen. When you fire SentRec back up later and load the CSV it will automatically select the first sentence that does not have a matching wav file. You can then continue your work from there.

### Q: How much audio do I need to train a Piper voice?
Depends. If you want to train from a pre-trained checkpoint you can probably get away with recording about 30-60 minutes of varied sentences. But if the pre-trained voice is bad to begin with your result will inherit those characteristics. English pre-trained checkpoints are better than most other languages for obvious reasons.
If you want to train a voice from scratch in a non-English language you will need A LOT more! Looking around the internet some say that 10 hours of varied sentences are the minimum! And that's exactly why I made SentRec. I want to train a voice from scratch and I was getting frustrated that no offline software seemed to enable an optimal workflow for this.

## Todo
* SH: Implement a "grab noise-floor" button that records 1 second silence and calculates a noise floor threshold for use with silence removal at beginning and end of audio
* CH: When playing audio, add a moving vertical line that follows audio playing in waveformwidget
* CH: Progress indicator showing what sentence is selected and how many there are in total