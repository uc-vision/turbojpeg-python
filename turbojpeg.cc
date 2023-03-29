
#include <iostream>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <Python.h>

#include <turbojpeg.h>

namespace py = pybind11;



class JpegException : public std::exception {
  std::string msg;

  public:
    JpegException(std::string const& context, std::string const& err) { 
      std::stringstream ss;
      ss << context << ", TurboJPEG error " << err;
      msg = ss.str();
    }
        
    const char * what () const throw () {
      return msg.c_str();
    }
};



class Jpeg {
  public:
  tjhandle handle;
  int quality = 90;


  Jpeg() {
      handle = tj3Init(TJINIT_TRANSFORM);
      tj3Set(handle, TJPARAM_FASTDCT, 0);

  }

  void check_error(std::string const &context, int result) {
    if (-1 == result){
        throw JpegException(context, tj3GetErrorStr(handle));
    }
  }


  py::array_t<uint8_t> encode8(py::array_t<uint8_t, py::array::c_style> const& input, 
    TJPF format=TJPF_RGB, int quality=90, TJSAMP chroma=TJSAMP_422);

  py::array_t<uint8_t> encode12(py::array_t<int16_t, py::array::c_style> const& input, 
    TJPF format=TJPF_RGB, int quality=90, TJSAMP chroma_sampling=TJSAMP_422);

  py::array decode(py::array_t<uint8_t, py::array::c_style> const& input, TJPF format=TJPF_RGB);    

};




inline void check_shape(py::array const& input, TJPF format) {
  if (input.ndim() != 3) {
      throw std::invalid_argument("only images with layout supported");
  }

  if (input.shape(2) == 1) {
    if (format != TJPF_GRAY) 
      throw std::invalid_argument("grayscale image must have format=GRAY");
  }  else if (input.shape(2) == 3) {
    if (format != TJPF_RGB && format != TJPF_BGR) 
      throw std::invalid_argument("3 channel image must be in format=RGB or BGR");
    
  } else if (input.shape(2) == 4) {
    if (format != TJPF_RGBX && format != TJPF_BGRX) 
      throw std::invalid_argument("4 channel image must be in format=RGBX or BGRX");
  } else {
      throw std::invalid_argument("only 3/4 channel colour(alpha ignored) or grayscale supported");
  }    
}


py::array_t<uint8_t> Jpeg::encode8(py::array_t<uint8_t, py::array::c_style> const& input, 
    TJPF format, int quality, TJSAMP chroma) {

  size_t buf_size = 0;
  unsigned char *buf_data = NULL;

  check_shape(input, format);

  ssize_t width = input.shape(1);
  ssize_t height = input.shape(0);

  ssize_t pitch = input.strides()[0];
  uint8_t const *data = input.data();
  {
    py::gil_scoped_release release;

    tj3Set(handle, TJPARAM_QUALITY, quality);
    tj3Set(handle, TJPARAM_LOSSLESS, 0);
    tj3Set(handle, TJPARAM_SUBSAMP, chroma);

    check_error("tj3Compress8", tj3Compress8(handle, data, 
      width, pitch, height, format, &buf_data, &buf_size));
  }

  py::array_t<uint8_t> output({ssize_t(buf_size)}, buf_data, py::capsule(buf_data, [](void *f) {
      tj3Free(f);
  }));

  return output;
}


py::array_t<uint8_t> Jpeg::encode12(py::array_t<int16_t, py::array::c_style> const& input, 
    TJPF format, int quality, TJSAMP chroma) {

  size_t buf_size = 0;
  unsigned char *buf_data = NULL;

  check_shape(input, format);

  ssize_t width = input.shape(1);
  ssize_t height = input.shape(0);

  ssize_t pitch = input.strides()[0] / sizeof(int16_t);

  int16_t const *data = input.data();
  {
    py::gil_scoped_release release;

    tj3Set(handle, TJPARAM_QUALITY, quality);
    tj3Set(handle, TJPARAM_LOSSLESS, 0);
    tj3Set(handle, TJPARAM_SUBSAMP, chroma);

    check_error("tj3Compress12", tj3Compress12(handle, data, 
      width, pitch, height, format, &buf_data, &buf_size));
  }

  py::array_t<uint8_t> output({ssize_t(buf_size)}, buf_data, py::capsule(buf_data, [](void *f) {
      tj3Free(f);
  }));

  return output;
}

py::array Jpeg::decode(py::array_t<uint8_t, py::array::c_style> const& input, TJPF format) {
  check_error("tj3DecompressHeader", tj3DecompressHeader(handle, input.data(), input.size()));

  int width = tj3Get(handle, TJPARAM_JPEGWIDTH);
  int height = tj3Get(handle, TJPARAM_JPEGHEIGHT);

  int prec = tj3Get(handle, TJPARAM_PRECISION);
  int colorspace = tj3Get(handle, TJPARAM_COLORSPACE);
  std::cout << "width: " << width << " height: " << height << " prec: " << prec << std::endl;
  std::cout << "colorspace: " << colorspace << std::endl;

  if (prec == 8) {
    py::array_t<uint8_t> output({height, width, 3});
    output[py::make_tuple(py::ellipsis())] = 0;

    size_t pitch = output.strides()[0];
    check_error("tj3Decompress8",  tj3Decompress8(handle, input.data(), input.size(), 
      output.mutable_data(), pitch, format));

    return output;

  } else if (prec == 12) {
    py::array_t<int16_t> output({height, width, 3});
    output[py::make_tuple(py::ellipsis())] = 0;


    size_t pitch = output.strides()[0] / sizeof(int16_t);
    check_error("tj3Decompress12",  tj3Decompress12(handle, input.data(), input.size(), 
      output.mutable_data(), pitch, format));


    return output;
  } else {
    throw std::invalid_argument("only 8/12 bit precision supported");
  }


}


PYBIND11_MODULE(turbojpeg_python, m) {
  m.doc() = "turbojpeg binding"; // optional module docstring

  auto jpeg = py::class_<Jpeg>(m, "Jpeg");

  py::enum_<TJPF>(jpeg, "InputFormat")
    .value("RGB", TJPF_RGB)
    .value("BGR", TJPF_BGR)
    .value("RGBX", TJPF_RGBX)
    .value("BGRX", TJPF_BGRX)
    .value("GRAY", TJPF_GRAY)
    .export_values();


  py::enum_<TJSAMP>(jpeg, "Chroma")
    .value("CHROMA_444", TJSAMP_444)
    .value("CHROMA_422", TJSAMP_422)
    .value("CHROMA_420", TJSAMP_420)
    .value("CHROMA_GRAY", TJSAMP_GRAY)
    .export_values();


  jpeg.def(py::init<>())
    .def("encode8", &Jpeg::encode8,
       py::arg("input"),  py::arg("format")=TJPF_RGB, py::arg("quality")=90, py::arg("chroma")=TJSAMP_422)

    .def("encode12", &Jpeg::encode12,
       py::arg("input"),  py::arg("format")=TJPF_RGB, py::arg("quality")=90, py::arg("chroma")=TJSAMP_422)

    .def("decode", &Jpeg::decode, py::arg("input"),  py::arg("format")=TJPF_RGB)


    .def("__repr__", [](const Jpeg &a) { return "Jpeg"; });



  py::register_exception<JpegException>(m, "JpegException");

}