#!/usr/bin/python

# -*- coding:utf-8 -*-
__author__ = 'wj'

import argparse
from ws4py.client.threadedclient import WebSocketClient
import time
import threading
import sys
import urllib
import Queue
import json
import time
import os

def rate_limited(maxPerSecond):
    minInterval = 1.0 / float(maxPerSecond)
    def decorate(func):
        lastTimeCalled = [0.0]
        def rate_limited_function(*args,**kargs):
            elapsed = time.clock() - lastTimeCalled[0]
            leftToWait = minInterval - elapsed
            if leftToWait>0:
                time.sleep(leftToWait)
            ret = func(*args,**kargs)
            lastTimeCalled[0] = time.clock()
            return ret
        return rate_limited_function
    return decorate


class MyClient(WebSocketClient):

    def __init__(self, filename, url):
        super(MyClient, self).__init__(url)
        self.filename = filename
        self.final_hyps = []
        self.final_hyp_queue = Queue.Queue()

    #@rate_limited(40)
    def send_data(self, data):
        self.send(data, binary=False)

    def opened(self):
        def send_data_to_ws():
            with open(self.filename, 'rb') as f:
                block = f.read(1000)
                while block != "":
                    self.send_data(block)
                    block = f.read(1000)
            self.send_data("EOS")

        t = threading.Thread(target=send_data_to_ws)
        t.start()


    def received_message(self, m):
        #print str(m)
        self.final_hyps.append(str(m))

    def get_full_hyp(self, timeout=600000):
        return self.final_hyp_queue.get(timeout)

    def closed(self, code, reason=None):
        self.final_hyp_queue.put(" ".join(self.final_hyps))

def main():

    parser = argparse.ArgumentParser(description='Command line client for server')
    parser.add_argument('-u', '--uri', default="ws://192.168.204.137:8881", dest="uri", help="Server websocket URI")
    parser.add_argument('-p', '--param', default="", dest="param", type=str, help="like {fr='test',uid='-1',cuid='',fm='mp3'}")
    parser.add_argument('-f','--filename', help="str")
    args = parser.parse_args()

    param=args.param
    i = 0
    while i < 3:
      i = i + 1 
      ws = MyClient(args.filename, args.uri + '?%s' % (urllib.urlencode([("param",param)])))
      ws.connect()
      print ws.get_full_hyp()

if __name__ == "__main__":
    main()

