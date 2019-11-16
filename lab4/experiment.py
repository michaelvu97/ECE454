import os
from subprocess import Popen, PIPE
from collections import deque
import numpy as np
import datetime
import re

DEVNULL = open(os.devnull, 'wb', 0)

progs = ["randtrack", "randtrack_global_lock", "randtrack_tm", "randtrack_list_lock", "randtrack_element_lock", "randtrack_reduction"]

def run(name, num_threads, samples_to_skip):
    lines_to_read = 1
    p = Popen(['/usr/bin/time', '--format=%e'] + [name, str(num_threads), str(samples_to_skip)], stdout=DEVNULL, stderr=PIPE)
    with p.stderr:
        q = deque(iter(p.stderr.readline, b''), maxlen=lines_to_read)
    rc = p.wait()
    res = float(b''.join(q).decode().strip())
    return res

def Q9():
    samples_to_skip = 50
    num_threads = 1

    res = []

    for prog in progs:
        res.append(sum([run(prog, num_threads, samples_to_skip) for i in range(5)])/5.0)
        print(prog + " complete")
    
    # Normalize
    res_normal = [r / res[0] for r in res]
    print(res)
    print(res_normal)    

Q9()