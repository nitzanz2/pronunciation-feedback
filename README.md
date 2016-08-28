Discriminative Phoneme Alignment for Pronunciation Feedback
===========================================================
This repository contains our final project at 3rd year of CS in Bar-Ilan University under the supervision of Dr. Joseph Keshet.

## Project Discription
This project proposes an Android application that provides visual feedback on the quality of pronunciation in a foreign language. In the application we built the learner of a foreign language is prompted to pronounce a word out of a list, his voice is recorded and analyzed, and he is given feedback on his pronunciation per syllable or letter. 
For example, a learner of American English is asked to pronounced the word aluminum that should be pronounced canonically as [ah l uw m ah n ah m]. If the learner pronounced the word as [ah l uw m ae n ux m], he will be prompted visually that two letters aluminum were mispronounced.

## System Requirements
OS Linux Ubuntu (3.13+) is the current supported host development operating systems.

## Other Requierments
- Python 2.7+
- SoX - Sound eXchange platform
- Bottle: Python Web Framework

## Project Parts and Running Instructions
* The forced (phoneme) alignment module is execute the forced_alignmet.py file under the path Server_code/forced_alignment, adding the speech .wav file, the corresponding .phoneme file containing the speech phonemes, and and name of an output file .TextGrid. If desired, a --debug flag can be used to track some of the code flow.
Commandline example:
python forced_alignment.py example.wav example.phones example.TextGrid
* To run e server 
* listens to http requests containing a serialized speech .wav file and its corresponding .phoneme file, 

