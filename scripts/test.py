from turbojpeg_python import Jpeg
import argparse
import cv2

def load_rgb(path):
  image = cv2.imread(path, cv2.IMREAD_UNCHANGED)
  return cv2.cvtColor(image, cv2.COLOR_BGR2RGB)




if __name__ == "__main__":
  parser = argparse.ArgumentParser()
  parser.add_argument("input", help="input file")

  args = parser.parse_args()


  image = load_rgb(args.input)
  jpeg = Jpeg()

  result = jpeg.encode16(image)

