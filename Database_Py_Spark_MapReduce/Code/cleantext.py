#!/usr/bin/env python3

"""Clean comment text for easier parsing."""

from __future__ import print_function

import re
import string
import argparse
import sys

__author__ = ""
__email__ = ""

# Depending on your implementation,
# this data may or may not be useful.
# Many students last year found it redundant.
_CONTRACTIONS = {
    "tis": "'tis",
    "aint": "ain't",
    "amnt": "amn't",
    "arent": "aren't",
    "cant": "can't",
    "couldve": "could've",
    "couldnt": "couldn't",
    "didnt": "didn't",
    "doesnt": "doesn't",
    "dont": "don't",
    "hadnt": "hadn't",
    "hasnt": "hasn't",
    "havent": "haven't",
    "hed": "he'd",
    "hell": "he'll",
    "hes": "he's",
    "howd": "how'd",
    "howll": "how'll",
    "hows": "how's",
    "id": "i'd",
    "ill": "i'll",
    "im": "i'm",
    "ive": "i've",
    "isnt": "isn't",
    "itd": "it'd",
    "itll": "it'll",
    "its": "it's",
    "mightnt": "mightn't",
    "mightve": "might've",
    "mustnt": "mustn't",
    "mustve": "must've",
    "neednt": "needn't",
    "oclock": "o'clock",
    "ol": "'ol",
    "oughtnt": "oughtn't",
    "shant": "shan't",
    "shed": "she'd",
    "shell": "she'll",
    "shes": "she's",
    "shouldve": "should've",
    "shouldnt": "shouldn't",
    "somebodys": "somebody's",
    "someones": "someone's",
    "somethings": "something's",
    "thatll": "that'll",
    "thats": "that's",
    "thatd": "that'd",
    "thered": "there'd",
    "therere": "there're",
    "theres": "there's",
    "theyd": "they'd",
    "theyll": "they'll",
    "theyre": "they're",
    "theyve": "they've",
    "wasnt": "wasn't",
    "wed": "we'd",
    "wedve": "wed've",
    "well": "we'll",
    "were": "we're",
    "weve": "we've",
    "werent": "weren't",
    "whatd": "what'd",
    "whatll": "what'll",
    "whatre": "what're",
    "whats": "what's",
    "whatve": "what've",
    "whens": "when's",
    "whered": "where'd",
    "wheres": "where's",
    "whereve": "where've",
    "whod": "who'd",
    "whodve": "whod've",
    "wholl": "who'll",
    "whore": "who're",
    "whos": "who's",
    "whove": "who've",
    "whyd": "why'd",
    "whyre": "why're",
    "whys": "why's",
    "wont": "won't",
    "wouldve": "would've",
    "wouldnt": "wouldn't",
    "yall": "y'all",
    "youd": "you'd",
    "youll": "you'll",
    "youre": "you're",
    "youve": "you've"
}


# both are used in pruning:
# allowed punctuation splits
allowed_punctuation = ['.', '!', '?', ',', ';', ':']

# delete punctuation deletes from string ensd
delete_punctuation =['&', '@', '-','_', '*', '"', '(', ')', '^', '`']

def generate_ngrams_v2(parsed_tokens, n):
    global allowed_punctuation
    return [
            '_'.join(parsed_tokens[i:i+n]) for i in range(len(parsed_tokens) - n + 1)
            if not any(p in allowed_punctuation for p in parsed_tokens[i:i+n])
        ]

# You may need to write regular expressions.
def prune_tokens(tokens):
    parsed_tokens = []
    for token in tokens:
        #print (token)
        j = len(token)

        # check from beginning
        while j > 0:
            if token[0] in allowed_punctuation:
                parsed_tokens.append(token[0])
                token = token[1:]
                j -= 1
            elif token[0] in delete_punctuation:
                token = token[1:]
                j -= 1
            else:
                break

        # check from end. use same j value
        post_arr = []
        while j > 0:
            if token[-1] in allowed_punctuation:
                post_arr.insert(0, token[-1])
                token = token[:-1]
                j -= 1
            elif token[-1] in delete_punctuation:
                token = token[:-1]
                j -= 1
            else:
                break
        if len(token) > 0:
            parsed_tokens.append(token)

        parsed_tokens = parsed_tokens + post_arr
    return parsed_tokens

def sanitize(text):
    global allowed_punctuation, delete_punctuation
    """Do parse the text in variable "text" according to the spec, and return
    a LIST containing FOUR strings 
    1. The parsed text.
    2. The unigrams
    3. The bigrams
    4. The trigrams
    """
    # YOUR CODE GOES BELOW:
    result = re.sub(r'\\n', ' ', text)  # newline to spcae
    result = re.sub(r'\\t', ' ', result) # tab to space

    # lowercase
    result = result.lower()

    # [](url) removal
    result = re.sub(r'\[(?P<text>.*?)\]\((.*?)\)','\g<text>', result) # extract the text out of square and normal brackets.

    # urls removal
    #TODO: can't just remove urls like this.
    # What if we have something like http://www.google.com/quot&29s; ?
    #result = re.sub(r'(https?:\/\/\S+)', '', result)
    #result = re.sub(r'(www\.\S+)', '', result)
    
    #remove illegal symbols
    result = re.sub(r'[^a-zA-Z0-9 ,!:;\-\.\?\'\/\$#&@%^*"=\)\(`\+~…]', ' ', result) # remove illegal symbols like ~

    # take care of ampersand.
    result = re.sub(r'\s&\s',' ', result)

    # character by character 
    tokens = [token for token in result.split(' ') if token != '']
    parsed_tokens = []
    parsed_tokens = prune_tokens(tokens)

    parsed_tokens = [re.sub(r'(https?:\/\/\S+)', '', token) for token in parsed_tokens]
    parsed_tokens = [re.sub(r'(www\.\S+)', '', token) for token in parsed_tokens]
    parsed_tokens = [token for token in parsed_tokens if token != '']

    parsed_tokens = prune_tokens(parsed_tokens)
    parsed_text = ' '.join(parsed_tokens)

    ngrams = generate_ngrams_v2(parsed_tokens, 1) + generate_ngrams_v2(parsed_tokens, 2) + generate_ngrams_v2(parsed_tokens, 3)
    return ngrams

if __name__ == "__main__":
    # This is the Python main function.
    # You should be able to run
    # python cleantext.py <filename>
    # and this "main" function will open the file,
    # read it line by line, extract the proper value from the JSON,
    # pass to "sanitize" and print the result as a list.

    # YOUR CODE GOES BELOW.
    result_text = { 0: "Parsed text is: ", 1: "Unigram is: ", 2: "Bigram is: ", 3: "Trigram is: "}
    if len(sys.argv) == 2:
        with open(sys.argv[1]) as f:
            result = sanitize(f.read())
        print(result)
    else:
        print("USAGE: python3 cleantext.py line")
