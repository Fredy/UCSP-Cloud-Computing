#!/usr/bin/env python3

import argparse
import os
import re
import sys
from itertools import repeat
from multiprocessing import Manager, Pool
from time import sleep
import requests
from bs4 import BeautifulSoup
from urllib.parse import urljoin, urlsplit, quote
from utils import text_from_html

MAIN_URL = 'https://en.wikipedia.org/wiki/'
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
    return {quote_url(i['href']) for i in items}


def save_html(soup_content, url_path):
    with open('./html/' + url_path + '.html' , 'w') as html:
        html.write(str(soup_content))

def save_text(soup_content, url_path):
    text = text_from_html(soup_content)

    with open('./text/' + url_path + '.txt' , 'w') as out_file:
        out_file.write(text)

def save_urls(urls, url_path):
    with open('./urls/' + url_path + '.txt' , 'w') as out_file:
        out_file.write('\n'.join(urls))

def quote_url(url_path):
    page_sub_path = url_path[6:]
    page_sub_path = quote(page_sub_path, safe='')

    return page_sub_path

def _crawl_func(args_tuple):
    url_base, url_path, do_save_html, do_save_text, do_save_urls = args_tuple

    url = urljoin(url_base, url_path)
    page = get_page(url)
    if not page:
        return []

    remove_recurrent_links(page)
    if do_save_html:
        save_html(page, url_path)
    if do_save_text:
        save_text(page, url_path)

    urls = get_all_hrefs(page)
    if do_save_urls:
        save_urls(urls, url_path)

    return urls

def crawl_wikipedia(start_url, deepness, save_html, save_text, save_urls):
    if not save_html and not save_text and not save_urls:
        save_html = True
    if save_html:
        os.makedirs('html', exist_ok=True)
    if save_text:
        os.makedirs('text', exist_ok=True)
    if save_urls:
        os.makedirs('urls', exist_ok=True)

    start_path = urlsplit(start_url).path
    start_path = quote_url(start_path)
    visited = {start_path}

    new_paths = _crawl_func((MAIN_URL, start_path, save_html, save_text, save_urls))

    for i in range(deepness - 1):
        new_paths.difference_update(visited)
        visited.update(new_paths)

        with Pool() as pool:
            tmp = pool.map(
                _crawl_func, 
                zip(
                    repeat(MAIN_URL), 
                    new_paths,
                    repeat(save_html),
                    repeat(save_text),
                    repeat(save_urls)
                    )
                )
        
        new_paths = set()
        for href_set in tmp:
            new_paths.update(href_set)
        
    return visited


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    
    parser.add_argument('-u', '--url', dest='save_url', action='store_true',
                    help='determines if url files will be saved')

    parser.add_argument('-html', '--html', dest='save_html', action='store_true',
                    help='determines if htmls files will be saved')

    parser.add_argument('-t', '--text', dest='save_text', action='store_true',
                    help='determines if text files will be saved')
                    
    parser.add_argument('-d', '--deepness', action='store', dest='deepness', default=3 ,
                    help='the crawl deepness', type=int)
                    
    parser.add_argument('-s', '--start_url', action='store', dest='start_url', default=START_URL,
                    help='start url for the crawler')
    
    values = parser.parse_args()

    
    paths = crawl_wikipedia(
        values.start_url,
        values.deepness,
        values.save_html,
        values.save_text,
        values.save_url
    )

    with open('crawled_paths.txt', 'w') as out_file:
        for i in paths:
            out_file.write(i + '\n')