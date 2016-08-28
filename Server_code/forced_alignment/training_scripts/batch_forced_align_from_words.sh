#!/bin/zsh

timit_dir="/Databases/timit"
#timit_dir="/share/data/speech/Datasets/timit"

foreach f (`cat config/test.wav`)
  g=$f:r
  scripts/words2phonemes.pl $timit_dir/$g.txt data/$g.pronunciations
	scripts/forced_align_timit.pl $timit_dir/$g.wav data/$g.scores data/$g.pronunciations data/$g.pred_align_confs data/$g.phn2
end

