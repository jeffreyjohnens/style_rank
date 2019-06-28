import numpy as np
from tqdm import tqdm
import itertools
from calculate_dissonance import _FRACTIONS

power = (2**np.arange(12))
interval_class = np.array([0,1,2,3,4,5,6,5,4,3,2,1])

major_scale = [1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1]
minor_scale = [1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1]
all_scales = (np.stack([np.roll(major_scale, i) for i in range(12)] + [np.roll(minor_scale, i) for i in range(12)]) * power[None,:]).sum(1)

def nbitrep(x,n):
	assert np.all([xx < 2**n for xx in x])
	return sum((xx << i*n) for i,xx in enumerate(x))

def get_pcd(x, n=12):
	power = (2**np.arange(n))
	r = np.array([np.roll((x & power).astype(np.bool), j) for j in range(n)])
	return np.min((r * power[None,:]).sum(1))

def get_rot(x, n=12):
	power = (2**np.arange(n))
	r = np.array([np.roll((x & power).astype(np.bool), j) for j in range(n)])
	return np.argmin((r * power[None,:]).sum(1))

def get_ic_count(x):
	from itertools import combinations
	pitches = [i for i in range(12) if ((x >> i) & 1)]
	ics = [interval_class[(a-b)% 12] for a,b in combinations(pitches,2)]
	return nbitrep(np.bincount(ics), 8)

def get_scale_rep(x):
	return ((all_scales & x == x) * (2**np.arange(24))).sum()

def roll_bit(x, n=12):
	return ((x << np.arange(n)) | (x >> (n - np.arange(n)))) & 4095

if __name__ == "__main__":

	pcd = np.array([get_pcd(i) for i in np.arange(4096)])
	rot = np.array([get_rot(i) for i in np.arange(4096)])
	pcscale = np.array([get_scale_rep(i) for i in np.arange(4096)])
	pcbool = (np.arange(4096)[:,None] & power[None,:]).astype(np.bool)
	pcsize = pcbool.sum(1)
	iccount = np.array([get_ic_count(i) for i in np.arange(4096)])
	istriad = np.zeros((4096,), dtype=np.int32)
	path_length = np.load("tonnetz_path_lengths.npz")["data"]

	prefix = "static const uint64_t "
	with open("pcd.hpp", "w") as f:
		f.write("#ifndef STYLE_RANK_PCD_H\n")
		f.write("#define STYLE_RANK_PCD_H\n")
		f.write(prefix + "npcd = " + str(np.max(pcd)+1) + ";\n")
		f.write(prefix + "pcd[4096] = {" + repr(list(pcd))[1:-1] + "};\n")
		f.write(prefix + "rot[4096] = {" + repr(list(rot))[1:-1] + "};\n")
		f.write(prefix + "iccount[4096] = {" + repr(list(iccount))[1:-1] + "};\n")
		f.write(prefix + "pcsize[4096] = {" + repr(list(pcsize))[1:-1] + "};\n")
		f.write(prefix + "pcscale[4096] = {" + repr(list(pcscale))[1:-1] + "};\n")
		f.write(prefix + "istriad[4096] = {" + repr(list(istriad))[1:-1] + "};\n")
		f.write(prefix + "tonnetz[4096] = {" + repr(list(path_length))[1:-1] + "};\n")
		f.write(prefix + "dissfracnum[256] = {" + repr(list(_FRACTIONS[:,0]))[1:-1] + "};\n")
		f.write(prefix + "dissfracden[256] = {" + repr(list(_FRACTIONS[:,1]))[1:-1] + "};\n")
		f.write("#endif\n")
