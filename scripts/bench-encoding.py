import turbojpeg
from scripts.threaded import bench_threaded 
import turbojpeg_python 

import cv2
import numpy as np

import gc
import argparse



class EncoderCV2:
  def __init__(self, quality=90):
    self.quality = [int(cv2.IMWRITE_JPEG_QUALITY), quality] 

  def __call__(self, image):
    return cv2.imencode('.jpg', image, self.quality)


class EncoderV3:
  def __init__(self, quality=90):
    self.jpeg = turbojpeg_python.Jpeg()
    self.quality = quality

  def __call__(self, image):
    return self.jpeg.encode8(image, quality=self.quality, chroma=self.jpeg.CHROMA_422)

class Encoder12Bit:
  def __init__(self, quality=90):
    self.jpeg = turbojpeg_python.Jpeg()
    self.quality = quality

  def __call__(self, image):
    return self.jpeg.encode12(image, quality=self.quality, chroma=self.jpeg.CHROMA_422)


class EncoderTJ:
  def __init__(self, quality=90):
    self.jpeg = turbojpeg.TurboJPEG()
    self.quality = quality

  def __call__(self, image):
    return self.jpeg.encode(image, quality=self.quality, jpeg_subsample=turbojpeg.TJSAMP_422)



def main(args):
  image = cv2.imread(args.filename, cv2.IMREAD_COLOR)
  if image is None:
    raise ValueError(f'Could not read image {args.filename}')

  images = [image] * args.n
  num_threads = args.j

  image12 = image.astype(np.int16) * 16
  images12 = [image12] * args.n

  print(f'opencv threaded j={num_threads}: {bench_threaded(EncoderCV2, images, num_threads, args=[args.q]):>5.1f} images/s')
  print(f'turbojpeg threaded j={num_threads}: {bench_threaded(EncoderTJ, images, num_threads, args=[args.q]):>5.1f} images/s')
  print(f'turbojpegv3 threaded j={num_threads}: {bench_threaded(EncoderV3, images, num_threads, args=[args.q]):>5.1f} images/s')

  print(f'turbojpegv3 (12bit) threaded j={num_threads}: {bench_threaded(Encoder12Bit, args.q, images12, num_threads):>5.1f} images/s')



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
  

