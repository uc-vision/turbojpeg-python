
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


  Jpeg() {
      handle = tj3Init(TJINIT_COMPRESS);
  }

  void check_error(std::string const &context, int result) {
    if (-1 == result){
        throw JpegException(context, tj3GetErrorStr(handle));
    }
  }

  py::array_t<uint8_t> encode8(py::array_t<uint8_t> const& input, int quality=90);
  py::array_t<uint8_t> encode12(py::array_t<uint16_t> const& input, int quality=90);



};



// int tj3Compress16(tjhandle handle, const unsigned short *srcBuf,
//                             int width, int pitch, int height, int pixelFormat,
//                             unsigned char **jpegBuf, size_t *jpegSize);
py::array_t<uint8_t> Jpeg::encode12(py::array_t<uint16_t> const& input) {
    if (input.ndim() != 3) {
        throw std::invalid_argument("encode12: only 3d arrays supported");
    }

    if (input.shape(2) != 3) {
        throw std::invalid_argument("encode12: only 3 channel rgb supported");
    }    

    int height = input.shape(0);
    int width = input.shape(1);

    size_t buf_size = 0;
    unsigned char *buf_data = NULL;

    tj3Set(handle, TJPARAM_QUALITY, quality);
    tj3Set(handle, TJPARAM_SUBSAMP, TJSAMP_422);
    tj3Set(handle, TJPARAM_LOSSLESS, 0);


    check_error("tj3Compress12", tj3Compress12(handle, input.data(), 
      width, 1, height, TJPF_RGB, &buf_data, &buf_size));

    py::array_t<uint8_t> output({buf_size}, buf_data, py::capsule(buf_data, [](void *f) {
        tj3Free(f);
    }));

    return output;
}

py::array_t<uint8_t> Jpeg::encode8(py::array_t<uint8_t> const& input, int quality) {
    if (input.ndim() != 3) {
        throw std::invalid_argument("encode8: only 3d arrays supported");
    }

    if (input.shape(2) != 3) {
        throw std::invalid_argument("encode8: only 3 channel rgb supported");
    }    

    int height = input.shape(0);
    int width = input.shape(1);

    size_t buf_size = 0;
    unsigned char *buf_data = NULL;

    tj3Set(handle, TJPARAM_QUALITY, quality);
    tj3Set(handle, TJPARAM_SUBSAMP, TJSAMP_422);
    tj3Set(handle, TJPARAM_LOSSLESS, 0);


    check_error("tj3Compress8", tj3Compress8(handle, input.data(), 
      width, 1, height, TJPF_RGB, &buf_data, &buf_size));

    py::array_t<uint8_t> output({buf_size}, buf_data, py::capsule(buf_data, [](void *f) {
        tj3Free(f);
    }));

    return output;
}

PYBIND11_MODULE(turbojpeg_python, m) {
  m.doc() = "turbojpeg binding"; // optional module docstring

  auto jpeg = py::class_<Jpeg>(m, "Jpeg");
  jpeg.def(py::init<>())
    .def("encode8", &Jpeg::encode8)

    .def("encode12", &Jpeg::encode12)
    .def("encode16", &Jpeg::encode16)
    .def("__repr__", [](const Jpeg &a) { return "Jpeg"; });


  py::enum_<TJPF>(jpeg, "InputFormat")
    .value("RGB", TJPF_RGB)
    .value("BGR", TJPF_BGR)
    .value("RGBX", TJPF_RGBX)
    .value("BGRX", TJPF_BGRX)
    .value("GRAY", TJPF_GRAY)
    .export_values();


  py::register_exception<JpegException>(m, "JpegException");

}