import time
from threading import Thread
from queue import Queue
import torch


class Timer:     
    def __enter__(self):
        self.start = time.time()
        return self

    def __exit__(self, *args):
        self.end = time.time()
        self.interval = self.end - self.start

class Threaded(object):
  def __init__(self, create_processor, size=8, args=[]):
        # Image file writers
    self.queue = Queue(size)
    self.threads = [Thread(target=self.work_thread, args=()) 
        for _ in range(size)]

    self.create_processor = create_processor  
    self.args = args
    for t in self.threads:
        t.start()


  def work_thread(self):
    processor = self.create_processor(*self.args)
    data = self.queue.get()
    while data is not None:

      result = processor(data)
      data = self.queue.get()


  def encode(self, data):
    self.queue.put(data)


  def stop(self):
      for _ in self.threads:
          self.queue.put(None)

      for t in self.threads:
        t.join()

def bench_threaded(create_worker, images, threads, args=[], finalise=None):
  threads = Threaded(create_worker, threads, args=args)

  with Timer() as t:
    for image in images:
      threads.encode(image)

    threads.stop()

    if finalise is not None:
      finalise()

  return len(images) / t.interval


def bench(create_worker, images, finalise=None):
  encoder = create_worker()

  with Timer() as t:
    for image in images:
      encoder.encode(image)

    if finalise is not None:
      finalise()      

  return len(images) / t.interval