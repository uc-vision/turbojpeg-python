import turbojpeg 
import turbojpeg_python 
import torch

import cv2
import time

from functools import partial


from threading import Thread
from queue import Queue

import gc

import argparse

class Timer:     
    def __enter__(self):
        self.start = time.time()
        return self

    def __exit__(self, *args):
        self.end = time.time()
        self.interval = self.end - self.start


class CvJpeg(object):
  def encode(self, image, quality):
    result, compressed = cv2.imencode('.jpg', image, quality)


class Threaded(object):
  def __init__(self, create_jpeg, quality=90, size=8):
        # Image file writers
    self.queue = Queue(size)
    self.threads = [Thread(target=self.encode_thread, args=()) 
        for _ in range(size)]

    self.create_jpeg = create_jpeg  
    self.quality=quality
    for t in self.threads:
        t.start()


  def encode_thread(self):
    jpeg = self.create_jpeg(self.quality)
    image = self.queue.get()
    while image is not None:

      result = jpeg.encode(image)
      image = self.queue.get()


  def encode(self, image):
    self.queue.put(image)


  def stop(self):
      for _ in self.threads:
          self.queue.put(None)

      for t in self.threads:
        t.join()
      

class EncoderV3:
  def __init__(self, quality=90):
    self.jpeg = turbojpeg_python.Jpeg()
    self.quality = quality

  def encode(self, image):
    return self.jpeg.encode8(image, quality=self.quality, chroma=self.jpeg.CHROMA_422)

class EncoderTJ:
  def __init__(self, quality=90):
    self.jpeg = turbojpeg.TurboJPEG()
    self.quality = quality

  def encode(self, image):
    return self.jpeg.encode(image, quality=self.quality, jpeg_subsample=turbojpeg.TJSAMP_422)


def bench_threaded(create_encoder, quality, images, threads):
  threads = Threaded(create_encoder, quality, threads)

  with Timer() as t:
    for image in images:
      threads.encode(image)

    threads.stop()
    torch.cuda.synchronize()

  return len(images) / t.interval


def bench_encoder(create_encoder, images):
  encoder = create_encoder()

  with Timer() as t:
    for image in images:
      encoder.encode(image)

  return len(images) / t.interval


def main(args):
  image = cv2.imread(args.filename, cv2.IMREAD_COLOR)

  images = [image] * args.n
  num_threads = args.j


  print(f'turbojpeg threaded j={num_threads}: {bench_threaded(EncoderTJ, args.q, images, num_threads):>5.1f} images/s')
  print(f'turbojpeg(v3) threaded j={num_threads}: {bench_threaded(EncoderV3, args.q, images, num_threads):>5.1f} images/s')


if __name__=='__main__':
  parser = argparse.ArgumentParser(description='Jpeg encoding benchmark.')
  parser.add_argument('filename', type=str, help='filename of image to use')

  parser.add_argument('-j', default=6, type=int, help='run multi-threaded')
  parser.add_argument('-n', default=100, type=int, help='number of images to encode')
  parser.add_argument('-q', default=100, type=int, help='quality to encode')


  args = parser.parse_args()
  main(args)
  gc.collect()

  # main(args)
  

