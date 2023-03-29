from turbojpeg_python import Jpeg
import argparse
import cv2
import numpy as np


def display(image):
  cv2.imshow("image", image)
  cv2.waitKey()

if __name__ == "__main__":
  parser = argparse.ArgumentParser()
  parser.add_argument("input", help="input file")
  parser.add_argument("--write", default=None, help="output file")


  args = parser.parse_args()


  image = cv2.imread(args.input, cv2.IMREAD_COLOR)
  image12 = image.astype(np.int16) * 16
  jpeg = Jpeg()


  # result = jpeg.encode16(image.astype(np.uint16))
  # result = jpeg.encode8(image, format=Jpeg.BGR, quality=95, chroma=Jpeg.CHROMA_422)




  # if args.write:
  #   with open(args.write, "wb") as f:
  #     f.write(result12)

  #   print(f"Written to {args.write}")

  result8 = jpeg.encode8(image, quality=94, format=Jpeg.BGR, chroma=Jpeg.CHROMA_422)
  print(f"Encoded 8-bit size: {len(result8)}")

  result12 = jpeg.encode12(image12, quality=94, format=Jpeg.BGR, chroma=Jpeg.CHROMA_422)
  print(f"Encoded 12-bit size: {len(result12)}")



  decoded12 = jpeg.decode(result12, format=Jpeg.BGR)
  psnr12 = cv2.PSNR(image12, decoded12)
  print(f"PSNR 12-bit: {psnr12}")

  decoded8 = jpeg.decode(result8, format=Jpeg.BGR)
  psnr8 = cv2.PSNR(image, decoded8)
  print(f"PSNR 8-bit: {psnr8}")


  display(decoded8)
  display((decoded12 // 16).astype(np.uint8))


  # decode12 = cv2.imdecode(result12, cv2.IMREAD_UNCHANGED)

  # print("PSNR: ", psnr)

  # cv2.imshow("image", image)
  # cv2.waitKey()

  # cv2.imshow("image", image2)
  # cv2.waitKey()