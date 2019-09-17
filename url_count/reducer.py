#!/usr/bin/env python
import sys
from collections import Counter
 
if __name__ == "__main__":
    counter = Counter()
    
    for line in sys.stdin:
        url, count = line.split()
        try:
            count = int(count)
        except ValueError:
            continue

        counter.update({url:count})
    print(counter)
    for url, count in counter.items():
        print(url, count, sep='\t')