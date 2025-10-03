#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include "databento/dbn_parser.hpp"

namespace py = pybind11;

PYBIND11_MODULE(databento_cpp, m) {
    m.doc() = "Ultra-fast C++ library for parsing Databento DBN files";

    // Expose RecordHeader
    py::class_<databento::RecordHeader>(m, "RecordHeader")
        .def_readonly("length", &databento::RecordHeader::length)
        .def_readonly("rtype", &databento::RecordHeader::rtype)
        .def_readonly("publisher_id", &databento::RecordHeader::publisher_id)
        .def_readonly("product_id", &databento::RecordHeader::product_id)
        .def_readonly("ts_event", &databento::RecordHeader::ts_event);

    // Expose MBOMsg
    py::class_<databento::MBOMsg>(m, "MBOMsg")
        .def_readonly("hd", &databento::MBOMsg::hd)
        .def_readonly("order_id", &databento::MBOMsg::order_id)
        .def_readonly("price", &databento::MBOMsg::price)
        .def_readonly("size", &databento::MBOMsg::size)
        .def_readonly("flags", &databento::MBOMsg::flags)
        .def_readonly("channel_id", &databento::MBOMsg::channel_id)
        .def_readonly("action", &databento::MBOMsg::action)
        .def_readonly("side", &databento::MBOMsg::side)
        .def_readonly("ts_recv", &databento::MBOMsg::ts_recv)
        .def_readonly("ts_in_delta", &databento::MBOMsg::ts_in_delta)
        .def_readonly("sequence", &databento::MBOMsg::sequence)
        .def_readonly("symbol_id", &databento::MBOMsg::symbol_id);

    // Expose DirectParser
    py::class_<databento::DirectParser>(m, "DirectParser")
        .def(py::init<const char*, size_t>())
        .def("next", &databento::DirectParser::next, py::return_value_policy::reference)
        .def("has_next", &databento::DirectParser::has_next)
        .def("records_parsed", &databento::DirectParser::records_parsed);

    // Expose BatchParser
    py::class_<databento::BatchParser>(m, "BatchParser")
        .def(py::init<const char*, size_t, size_t>(), 
             py::arg("data"), py::arg("size"), py::arg("batch_size") = 10000)
        .def("next_batch", &databento::BatchParser::next_batch)
        .def("has_next", &databento::BatchParser::has_next)
        .def("records_parsed", &databento::BatchParser::records_parsed);

    // Expose CallbackParser
    py::class_<databento::CallbackParser>(m, "CallbackParser")
        .def(py::init<const char*, size_t>())
        .def("parse_all", &databento::CallbackParser::parse_all)
        .def("records_parsed", &databento::CallbackParser::records_parsed);

    // High-level API functions
    m.def("parse_file_mbo_fast", &databento::parse_file_mbo_fast,
          "Parse DBN file using direct API (330M records/sec)",
          py::arg("filename"));
    
    m.def("parse_file_mbo_batch", &databento::parse_file_mbo_batch,
          "Parse DBN file using batch API (150M records/sec)",
          py::arg("filename"), py::arg("batch_size") = 10000);
    
    m.def("parse_file_mbo_callback", &databento::parse_file_mbo_callback,
          "Parse DBN file using callback API (30M records/sec)",
          py::arg("filename"), py::arg("callback"));
    
    m.def("load_file", &databento::load_file,
          "Load file into memory buffer",
          py::arg("filename"));
}
