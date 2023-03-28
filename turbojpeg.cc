#include <stdio.h>
#include <errno.h>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <Python.h>

#include <turbojpeg.h>

namespace py = pybind11;






class JpegException : public std::exception {
  std::string context;
  std::string err;


  public:
    JpegException(std::string const& _context, std::string const& _err) :
      err(_err), context(_context)
    { }
        
    const char * what () const throw () {
      std::stringstream ss;
      ss << context << ", TurboJPEG error " << err;
      return ss.str().c_str();

    }
};




class Jpeg {
  public:
  

  Jpeg() {
      handle = tj3Init(TJINIT_COMPRESS);
  }

  void check_error(std::string const &context) {
    auto code = tj3GetErrorCode(handle);
    if (-1 == code){
        throw JpegException(context, tj3GetErrorStr(handle));
    }
  }


  py::array_t<uint8_t> encode12(py::array_t<uint8_t> const& input);
  py::array_t<uint8_t> encode16(py::array_t<uint16_t> const& input);



  tjhandle handle;
};

py::array_t<uint8_t> Jpeg::encode12(py::array_t<uint8_t> const& input) {
   
// int tj3Compress12(tjhandle handle, const short *srcBuf, int width,
//                             int pitch, int height, int pixelFormat,
//                             unsigned char **jpegBuf, size_t *jpegSize); 
    

    py::array_t<uint8_t> output;
    return output;
}

py::array_t<uint8_t> Jpeg::encode16(py::array_t<uint16_t> const& input) {
    
    auto shape = input.shape();
    int height = shape[0];
    int width = shape[1];
    int channels = shape[2];

    
    py::array_t<uint8_t> output;
    return output;
}

PYBIND11_MODULE(turbojpeg_python, m) {
  m.doc() = "turbojpeg binding"; // optional module docstring
  auto jpeg = py::class_<Jpeg>(m, "Jpeg");
  jpeg.def(py::init<>())
    .def("encode12", &Jpeg::encode12)
    .def("encode16", &Jpeg::encode16)
    .def("__repr__", [](const Jpeg &a) { return "Jpeg"; });


  py::register_exception<JpegException>(m, "JpegException");

}