import numpy as np
import pickle

alis = open("./exp/alignments", "r").readlines()
#gt = pickle.load(open("frame_level_dict.pkl", "rb"))

alignments = {}
for utt in alis:
    out = utt.strip().split(" ")
    uttid = out[0]
    phonemes = out[1:]

    indices = []
    start_idx = 0
    
    for idx, ele in enumerate(phonemes[:-1]):
        if(phonemes[idx+1] != ele):
            end_idx = idx+1
            indices.append([start_idx, end_idx])
            start_idx = end_idx
    alignments[uttid] = indices

pickle.dump(alignments, open("frame_level_fa.pkl", "wb"))
