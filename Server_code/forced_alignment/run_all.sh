#!/bin/bash

for i in {1..20}
do
	python forced_alignment.py --debug israeli/${i}.wav israeli/${i}.phones israeli/${i}.TextGrid
done

for i in {22..110}
do
	python forced_alignment.py --debug israeli/${i}.wav israeli/${i}.phones israeli/${i}.TextGrid
done
