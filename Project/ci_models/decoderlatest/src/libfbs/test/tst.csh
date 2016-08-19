#!/bin/csh -f

#set ind = /homes/reyes/sphinx/WSJ/training/model_parameters/WSJ_clean_13Cepstra.cd_continuous_1gau
#set mdeffn = /homes/reyes/sphinx/WSJ/training/model_architecture/WSJ_clean_13Cepstra.2800.mdef 
#set cepdir = /projects/speech-db/featurefiles/WSJ/WSJ0/channel1_cep13 
#set cepext = cep
#set ceplen = 13  
#set feat = 1s_c_d_dd 

#set ind = /homes/reyes/sphinx/WSJ/training/model_parameters/WSJ_clean_40Dlogspec.cd_continuous_1gau
#set mdeffn = /homes/reyes/sphinx/WSJ/training/model_architecture/WSJ_clean_40Dlogspec.2800.mdef 
#set cepdir = /projects/speech-db/featurefiles/WSJ/WSJ0/channel1_40dlspc
#set cepext = log
#set ceplen = 40
#set feat = 1s_c

set ind = /projects/speech-raj/expts/WSJ/training/model_parameters/WSJ_clean_40Dlogspec.cd_continuous_8gau
set mdeffn = /homes/reyes/sphinx/WSJ/training/model_architecture/WSJ_clean_13Cepstra.2800.mdef 
set cepdir = /projects/speech-db/featurefiles/WSJ/WSJ0/channel1_40dlspc
set cepext = log
set ceplen = 40
set feat = 1s_c

../s3decode-anytopo \
-logbase 1.0001  \
-senmgaufn .cont.  \
\
-mdeffn $mdeffn \
\
-meanfn $ind/means \
-varfn $ind/variances \
-mixwfn $ind/mixture_weights \
-tmatfn $ind/transition_matrices \
\
-feat $feat \
-topn 32  \
-beam 1e-80  \
-dictfn /homes/reyes/sphinx/WSJ/training/lists/cmudict.0.6d  \
-fdictfn /homes/reyes/sphinx/WSJ/training/lists/fillerdict  \
-ctlfn /homes/reyes/sphinx/WSJ/training/lists/complete_FILELIST.cleanwsj0  \
-cepdir $cepdir \
-cepext $cepext \
-ceplen $ceplen \
-agc none  \
-cmn current \
#-langwt 9.5 \
-langwt  0 \
-inspen 1.0 \
-lmfn  ~/expts/BN/FROMCMU/fastdecoder/language_model.arpaformat.DMP
