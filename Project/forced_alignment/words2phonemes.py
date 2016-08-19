
import argparse
from string import digits
import sys
import re

def main(words, cmu_dictionary_filename, debug_mode):

    # read filename to patter mapping
    cmu_to_39 = dict()
    cmu_to_39['aa'] = 'ao'
    cmu_to_39['l'] = 'el'
    cmu_to_39['n'] = 'en'
    cmu_to_39['zh'] = 'sh'

    # read CMU dictionary to the memory
    cmu_dictionary = dict()
    with open(cmu_dictionary_filename) as cmu_dictionary_file:
        for row in cmu_dictionary_file:
            if row.startswith(";;;"):
                continue
            [word, phoneme_string] = row.rstrip().split(" ", 1)
            # remove the digits (which mark stress)
            phoneme_string = phoneme_string.lower().translate(None, digits)
            #convert CMU phonemes to LeeHun39 set
            for key in cmu_to_39:
                phoneme_string = phoneme_string.replace(key, cmu_to_39[key])
            #print word, "-->", phoneme_string
            cmu_dictionary[word.lower()] = phoneme_string

    # remove all non letter signs from words
    words = re.sub("[\W\d]", " ", words.strip())
    print words

    # run over words in input
    phoneme_string = "sil "
    for word in words.split():
        if word not in cmu_dictionary:
            print >>sys.stderr, "Error: could not find the word %s in CMU dictionary %s." % \
                                (word, cmu_dictionary_filename)
            exit(-1)
        phoneme_string += cmu_dictionary[word] + " "
    phoneme_string += "sil"

    return phoneme_string


if __name__ == "__main__":

    # command line arguments
    parser = argparse.ArgumentParser(description="This utility find the phoneme string of a sequence of words.")
    parser.add_argument("words", help="sequence of words")
    parser.add_argument("cmu_dictionary", help="location of CMU dictionary file")
    parser.add_argument("--debug", dest='debug_mode', help="extra verbosity", action='store_true')
    args = parser.parse_args()
    print args.words
    phoneme_string = main(args.words, args.cmu_dictionary, args.debug_mode)
    print phoneme_string
