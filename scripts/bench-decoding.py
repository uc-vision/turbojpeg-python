import turbojpeg
from scripts.threaded import bench_threaded 
import turbojpeg_python 
import numpy as np

import cv2

import gc
import argparse

      

class DecodeCV2:
  def __call__(self, data):
    return cv2.imdecode(data, cv2.IMREAD_UNCHANGED)


class DecoderV3:
  def __init__(self):
    self.jpeg = turbojpeg_python.Jpeg()

  def __call__(self, data):
    return self.jpeg.decode(data)


class DecoderTJ:
  def __init__(self, quality=90):
    self.jpeg = turbojpeg.TurboJPEG()
    self.quality = quality

  def __call__(self, data):
    return self.jpeg.decode(data)



def main(args):
  with open(args.filename, 'rb') as f:
    data = np.frombuffer(f.read(), dtype=np.uint8)

  data = [data] * args.n
  num_threads = args.j


  # print(f'opencv threaded j={num_threads}: {bench_threaded(DecodeCV2, data, num_threads):>5.1f} images/s')
  print(f'turbojpeg threaded j={num_threads}: {bench_threaded(DecoderTJ,  data, num_threads):>5.1f} images/s')
  print(f'turbojpegv3 threaded j={num_threads}: {bench_threaded(DecoderV3,  data, num_threads):>5.1f} images/s')



if __name__=='__main__':
  parser = argparse.ArgumentParser(description='Jpeg encoding benchmark.')
  parser.add_argument('filename', type=str, help='filename of image to use')

  parser.add_argument('-j', default=6, type=int, help='run multi-threaded')
  parser.add_argument('-n', default=100, type=int, help='number of images to decode')


  args = parser.parse_args()
  main(args)
  gc.collect()

  # main(args)
  

