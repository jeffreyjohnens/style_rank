"""
Harmony perception by periodicity detection
Frieder Stolzenburg
Journal Of Mathematics And Music Vol. 9 , Iss. 3,2015
"""

import math
import numpy as np
np.warnings.filterwarnings('ignore')
from scipy.stats import hmean
from fractions import Fraction

# tuning 2
numerator = np.asarray([1, 16, 9, 6, 5, 4, 7, 3, 8, 5, 9, 15, 2])
denominator = np.asarray([1, 15, 8, 5, 4, 3, 5, 2, 5, 3, 5, 8, 1])

# tuning 1
#numerator = np.asarray([1, 16, 9, 6, 5, 4, 17, 3, 8, 5, 16, 15, 2])
#denominator = np.asarray([1, 15, 8, 5, 4, 3, 12, 2, 5, 3, 9, 8, 1])

# precompute the fraction for each semitone on the range [-128, 128]
def semitone_to_fraction(x):
  n = numerator[x % 12]
  d = denominator[x % 12]
  octave = (x // 12)
  if octave < 0:
      d *= (2 ** np.abs(octave))
  else:
      n *= (2 ** octave)
  f = Fraction(n,d)
  return [f.numerator, f.denominator]

_FRACTIONS = np.vstack([semitone_to_fraction(i) for i in range(-128,128)])

def rel(chord, log=True):
  if len(chord) < 2:
      return 0.
  chord = np.asarray(chord, dtype=np.int32)
  x = chord[None,:] - chord[:,None]
  F = _FRACTIONS[x + 128]
  H = np.min(F[:,:,0].astype(np.float32) / F[:,:,1], axis=1)
  if log:
    return np.mean(np.log2(H * np.lcm.reduce(F[:,:,1], axis=1)))
  return np.mean(H * np.lcm.reduce(F[:,:,1], axis=1))

def test_rel():
  # test this on the data from stolzberg
  assert( rel([0, 0], log=False) == 1. )
  assert( rel([0, 12], log=False) == 1. )
  assert( rel([0, 7], log=False) == 2. )
  assert( rel([0, 5], log=False) == 3. )
  assert( rel([0, 4], log=False) == 4. )
  assert( rel([0, 9], log=False) == 3. )
  assert( rel([0, 8], log=False) == 5. )
  assert( rel([0, 3], log=False) == 5. )
  assert( rel([0, 6], log=False) == 6. )
  assert( rel([0, 10], log=False) == 7. )
  assert( rel([0, 2], log=False) == 8.5 )
  assert( rel([0, 11], log=False) == 8. )
  assert( rel([0, 1], log=False) == 15. )

  
