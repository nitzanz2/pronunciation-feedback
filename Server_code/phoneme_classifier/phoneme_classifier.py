
import argparse
import tempfile
import os
from subprocess import call
from textgrid import *
import wave
from shutil import copyfile

def easy_call(command):
    try:
        print command
        call(command, shell=True)
    except Exception as exception:
        print "Error: could not execute the following"
        print ">>", command
        print type(exception)     # the exception instance
        print exception.args      # arguments stored in .args
        exit(-1)


if __name__ == "__main__":

    # command line arguments
    parser = argparse.ArgumentParser()
    parser.add_argument("wav_filename", help="input WAV file name")
    parser.add_argument("textgrid_filename", help="output TextGrid file name")
    parser.add_argument("--scores_filename", default="", help="output scores matrix")
    args = parser.parse_args()

    # binaries
    sox_bin = "sbin/sox"
    hcopy_bin = "sbin/HCopy"
    phoneme_classifier_bin = "bin/PhonemeFrameBasedDecode"
    htk_config = "config/htk.config"
    mfcc_stats_file = "config/mfcc.stats"
    phoneme_list_filename = "config/phonemes_39"

    # frame-base phoneme classifier parameters
    phoneme_classifier_pad = "5"
    phoneme_classifier_SIGMA = "4.3589"
    phoneme_classifier_C = "1"
    phoneme_classifier_B = "0.8"
    phoneme_classifier_epochs = "1"
    phoneme_classifier_model = "models/pa_phoeneme_frame_based.C_%s.B_%s.sigma_%s.pad_%s.epochs_%s.model" % \
            (phoneme_classifier_C, phoneme_classifier_B, phoneme_classifier_SIGMA, phoneme_classifier_pad, phoneme_classifier_epochs)

    # generate intermediate files from a temp filename
    (tmp_fd, tmp_filename) = tempfile.mkstemp()
    wav_filename = tmp_filename + ".16kHz.wav"
    mfc_filename = tmp_filename + ".mfc"

    # read Wav file parameters
    wave_file = wave.Wave_read(args.wav_filename)
    wave_sampling_rate = wave_file.getframerate()
    wave_file.close()

    # converts WAV to 16kHz
    if wave_sampling_rate != 16000:
        cmd = "%s %s -r 16k %s remix 1" % (sox_bin, args.wav_filename, wav_filename)
        easy_call(cmd)
        rm_wav_file = True
    else:
        wav_filename = args.wav_filename
        rm_wav_file = False

    # extract MFCC features using HCopy utility
    cmd_params = "%s -C %s %s %s" % (hcopy_bin, htk_config, wav_filename, mfc_filename)
    easy_call(cmd_params)

    # predict phonemes from MFCCs
    if args.scores_filename != "":
        scores_filename = args.scores_filename
    else:
        scores_filename = tmp_filename + ".scores"
    mfcc_filelist = tmp_filename + ".mfc_list"
    fid = open(mfcc_filelist, 'w')
    fid.write(mfc_filename)
    fid.close()
    scores_filelist = tmp_filename + ".scores_list"
    fid = open(scores_filelist, 'w')
    fid.write(scores_filename)
    fid.close()
    cmd_params = "%s -n %s -kernel_expansion rbf3 -sigma %s -mfcc_stats %s -averaging -scores %s %s " \
                 "null %s %s" % (phoneme_classifier_bin, phoneme_classifier_pad, phoneme_classifier_SIGMA,
                                 mfcc_stats_file, scores_filelist, mfcc_filelist, phoneme_list_filename,
                                 phoneme_classifier_model)
    easy_call(cmd_params)

    # read phoneme list
    phoneme_list_file = open(phoneme_list_filename)
    phoneme_map = [line.rstrip() for line in phoneme_list_file]
    phoneme_list_file.close()

    # load matrix of scores and build phoneme with start and end times
    scores_file = open(scores_filename)
    header_read = False
    prev_max_phoneme = ''
    prev_max_score = 0
    prev_start_frame = 0
    current_frame = 0
    phonemes = list()
    for line in scores_file:
        line.rstrip()
        # skip the first line
        if not header_read:
            header_read = True
        else:
            scores = map(float, line.split())
            scores_arg_max = max(enumerate(scores), key=lambda x: x[1])[0]
            max_phoneme = phoneme_map[scores_arg_max]
            if max_phoneme != prev_max_phoneme:
                if prev_max_phoneme != "":
                    phonemes.append((prev_max_phoneme, prev_max_score, prev_start_frame))
                prev_max_phoneme = max_phoneme
                prev_max_score = max(scores)
                prev_start_frame = current_frame
            else:
                prev_max_score += max(scores)
            current_frame += 1
    phonemes.append((prev_max_phoneme, prev_max_score, prev_start_frame))

    # read number of samples from wav
    wave_file = wave.Wave_read(wav_filename)
    wave_duration = wave_file.getnframes() / 16000.0
    wave_file.close()

    # build TextGrid
    textgrid = TextGrid()
    phonemes_tier = IntervalTier(name='Phonemes', xmin=0.0, xmax=wave_duration)
    for i in range(len(phonemes) - 1):
        phonemes_tier.append(Interval(phonemes[i][2] * 0.01, phonemes[i + 1][2] * 0.01, phonemes[i][0]))
    phonemes_tier.append(Interval(phonemes[len(phonemes) - 1][2] * 0.01, wave_duration, phonemes[len(phonemes) - 1][0]))
    textgrid.append(phonemes_tier)
    textgrid.write(args.textgrid_filename)

    # remove leftovers
    if rm_wav_file:
        os.remove(wav_filename)
    os.remove(mfc_filename)
    if args.scores_filename == "":
        os.remove(scores_filename)
    os.remove(mfcc_filelist)
    os.remove(scores_filelist)

