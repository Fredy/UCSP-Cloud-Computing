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


def text_from_html(soup):
    texts = soup.findAll(text=True)
    visible_texts = filter(tag_visible, texts)  

    words = []
    for line in visible_texts:
        for word in line.split():
            if word.isalpha():
                words.append(word)
    
    return ' '.join(words)