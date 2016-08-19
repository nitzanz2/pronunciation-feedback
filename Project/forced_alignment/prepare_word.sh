#!/bin/bash

python phone_converter_61to39.py considerable
cd single_words
sox considerable1-youtube.wav -r 16k considerable1_youtube.wav remix 1
rm considerable1-youtube.wav
sox considerable2-youtube.wav -r 16k considerable2_youtube.wav remix 1
rm considerable2-youtube.wav
#sox deaf-considerable.wav -r 16k considerable_deaf.wav remix 1
cd ..
python forced_alignment.py --debug single_words/considerable_deaf.wav considerable39.phones out.TextGrid
cp final_scores.txt single_words/scores/deaf/considerable_deaf_scores.txt

python forced_alignment.py --debug single_words/considerable1_youtube.wav considerable39.phones out.TextGrid
cp final_scores.txt single_words/scores/considerable1_youtube_scores.txt

python forced_alignment.py --debug single_words/considerable2_youtube.wav consumption39.phones out.TextGrid
cp final_scores.txt single_words/scores/considerable2_youtube_scores.txt

