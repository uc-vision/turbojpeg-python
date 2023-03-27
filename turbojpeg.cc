#include <stdio.h>
#include <errno.h>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <Python.h>

namespace py = pybind11;





py::array_t<uint8_t, py::array::c_style> encode_image(py::array_t<float, py::array::c_style> input_np) {
    
    std::vector<uint8_t> output;
    py::array_t<uint8_t, py::array::c_style> output_np = py::cast(output);

    return output_np;
}

PYBIND11_MODULE(turbojpeg, m) {
    m.doc() = "turbojpeg binding"; // optional module docstring
    m.def("encode_image", &encode_image, "save jxl image");
}