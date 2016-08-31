Discriminative Phoneme Alignment for Pronunciation Feedback
===========================================================
This repository contains our final project at 3rd year of CS in Bar-Ilan University.

## Project Discription
This project proposes an Android application that provides visual feedback on the quality of pronunciation in a foreign language. In the application we built, the learner of a foreign language is prompted to pronounce a word out of a list, his voice is recorded and analyzed, and he is given feedback on his pronunciation per syllable or letter. 
For example, a learner of American English is asked to pronounced the word aluminum that should be pronounced canonically as [ah l uw m ah n ah m]. If the learner pronounced the word as [ah l uw m ae n ux m], he will be prompted visually that two letters aluminum were mispronounced.

## System Requirements
OS Linux Ubuntu (3.13+) is the current supported host development operating systems.

## Other Requierments
- Python 2.7+
- SoX - Sound eXchange platform
- Bottle: Python Web Framework

## Project Parts and Running Instructions
* The forced (phoneme) alignment module - It is given as input the speech .wav file, the corresponding .phoneme file containing the speech phonemes, and name of an output file .TextGrid. It output into the TextGrid file all the audio information including the predicted start times of each phoneme, and into a 'final_scores.txt' file all the scores and phonemes calculations and information, including the final scored to each of the phoneme in the given speech.
To run it you should execute the python file forced_alignmet.py under the path Server_code/forced_alignment, adding the .wav file, its corresponding .phoneme file, and name of an output file .TextGrid. If desired, a --debug flag can be used to track some of the code flow.
Commandline example:
python forced_alignment.py example.wav example.phones example.TextGrid

* The server - A Python Bottle server which: 
  * Listens to http post requests containing a serialized speech .wav file and its corresponding .phoneme file.
  * Removes silence from the beginning and the end of the .wav file.
  * Change the wav file format to the required format.
  * Runs the forced alignment module and return the final scores.
To run in you should execute the python file server.py under the path Server_code/forced_alignment .

* The Android Application - Attached the apk which you can install on your android device. 
  The code for the application can be founed under the directory Client_Application_code.



#### Written by: Einav Saad, Hadas Cohen and  Nitzan Zeira
##### Supervised by Dr. [Joseph Keshet](http://u.cs.biu.ac.il/~jkeshet/)

For any question, suggestion etc:

einav.saad27@gmail.com

hadas10071992@gmail.com

nzeira@gmail.com

