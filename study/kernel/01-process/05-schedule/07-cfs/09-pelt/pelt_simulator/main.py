# -*- coding: utf-8 -*-

# https://gist.github.com/derkling/6d1d3a3164b0291ef64fa409f61a81cb
# https://gist.github.com/derkling/a0c6e03310a03e9721edbf453e131d25
from task import *
from pelt import *

if __name__ == "__main__":
    t = Task(period_ps=1000, run_ps=1000)
    pelt = PELT(t, init_util=0, half_life_ms=32, decay_cap_ms=None, verbose=True)
    pelt.plot(end_s = 0.1);
