#!/usr/bin/env python3

import os
import sys
from bs4 import BeautifulSoup
from bs4.element import Comment

def tag_visible(element):
    if element.parent.name in ['style', 'script', 'head', 'title', 'meta', '[document]']:
        return False
    if isinstance(element, Comment):
        return False
    return True


def text_from_html(body):
    soup = BeautifulSoup(body, 'html.parser')
    texts = soup.findAll(text=True)
    visible_texts = filter(tag_visible, texts)  
    print('xd')
    return ' '.join(t.strip() for t in visible_texts)


if __name__ == '__main__':
    os.makedirs('text', exist_ok=True)

    argv = sys.argv
    if len(argv) < 2:
        print('Enter a file name')
        exit(-1)
    
    file_path = argv[1]
    with open(file_path) as html:
        text = text_from_html(html)
    file_name = os.path.basename(file_path)
    output_name =  os.path.join('text', file_name + '.txt')

    with open(output_name, 'w') as text_file:
        text_file.write(text)
    
