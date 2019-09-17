#!/usr/bin/env python
import sys
import re

URL_RE = re.compile(r'MENTION.*(http.*)')

if __name__ == "__main__":
    for line in sys.stdin:
        match = URL_RE.fullmatch(line.strip())
        if not match:
            continue
        print(match.group(1), 1, sep='\t')
