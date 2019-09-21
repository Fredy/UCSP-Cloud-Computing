#!/usr/bin/env python3

import os
import re
import sys
from itertools import repeat
from multiprocessing import Manager, Pool
from time import sleep
import requests
from bs4 import BeautifulSoup
from urllib.parse import urljoin, urlsplit, quote

MAIN_URL = 'https://en.wikipedia.org'
START_URL = 'https://en.wikipedia.org/wiki/Plant'

HEADERS = {'user-agent': 'Mozilla/5.0 (X11; Linux x86_64; rv:68.0) Gecko/20100101 Firefox/68.0'}

def request_sleep(url, seconds=2):
    try:
        response = requests.get(url)
    except requests.ConnectionError:
        print('Sleeping for {} seconds'.format(seconds))
        sleep(seconds)
        response = request_sleep(url, seconds * 2)
    
    return response
    

def get_page(url):
    response = request_sleep(url)
    if response.status_code != 200:
        return None
    return  BeautifulSoup(response.content, features='html.parser')


def remove_recurrent_links(soup_content):
    id_list = ['mw-head', 'mw-panel', 'footer']
    for i in id_list:
        soup_content.find(id=i).decompose()


def get_all_hrefs(soup_content):
    items = soup_content.find_all('a', href=re.compile(r'^/wiki/(?!\w+:)'))
    return {i['href'] for i in items}


def save_html(soup_content, url_path):
    with open('.' + url_path + '.html' , 'w') as html:
        html.write(str(soup_content))


def _crawl_func(args_tuple):
    url_base, url_path = args_tuple
    page_sub_path = url_path[6:]
    page_sub_path = quote(page_sub_path, safe='')
    url_path = '/wiki/' + page_sub_path

    url = urljoin(url_base, url_path)
    page = get_page(url)
    if not page:
        return []

    remove_recurrent_links(page)
    save_html(page, url_path)

    return get_all_hrefs(page)


def crawl_wikipedia(start_url, deepness):
    os.makedirs('wiki', exist_ok=True)
    start_path = urlsplit(start_url).path 
    visited = {start_path}

    new_paths = _crawl_func((MAIN_URL, start_path))

    for i in range(deepness - 1):
        new_paths.difference_update(visited)
        visited.update(new_paths)

        with Pool() as pool:
            tmp = pool.map(_crawl_func, zip(repeat(MAIN_URL), new_paths))
        
        new_paths = set()
        for href_set in tmp:
            new_paths.update(href_set)
        
    return visited


if __name__ == "__main__":
    argv = sys.argv
    if len(argv) > 1:
        deepness = int(argv[1])
    else:
        deepness = 3

    if len(argv) > 2:
        start_url = argv[2]
    else:
        start_url = START_URL
    
    paths = crawl_wikipedia(start_url, deepness)

    with open('crawled_paths.txt', 'w') as out_file:
        for i in paths:
            out_file.write(i + '\n')