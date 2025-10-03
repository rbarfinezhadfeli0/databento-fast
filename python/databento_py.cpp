// Python bindings using pybind11

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <databento/parser.hpp>
#include <databento/dbn.hpp>

namespace py = pybind11;

PYBIND11_MODULE(databento_cpp, m) {
  m.doc() = "Ultra-fast databento parser (200M+ records/sec) - Alternative to official databento-cpp";

  // ============================================================================
  // Enums
  // ============================================================================
  
  py::enum_<databento::Action>(m, "Action")
    .value("Add", databento::Action::Add)
    .value("Cancel", databento::Action::Cancel)
    .value("Modify", databento::Action::Modify)
    .value("Clear", databento::Action::Clear)
    .value("Trade", databento::Action::Trade)
    .value("Fill", databento::Action::Fill)
    .export_values();

  py::enum_<databento::Side>(m, "Side")
    .value("Ask", databento::Side::Ask)
    .value("Bid", databento::Side::Bid)
    .value("None_", databento::Side::None)
    .export_values();

  // ============================================================================
  // MboMsg struct
  // ============================================================================
  
  py::class_<databento::MboMsg>(m, "MboMsg")
    .def(py::init<>())
    .def_readonly("ts_event", &databento::MboMsg::ts_event, "Event timestamp (nanoseconds)")
    .def_readonly("instrument_id", &databento::MboMsg::instrument_id, "Instrument ID")
    .def_readonly("action", &databento::MboMsg::action, "Action: A=Add, C=Cancel, M=Modify")
    .def_readonly("side", &databento::MboMsg::side, "Side: B=Bid, A=Ask")
    .def_readonly("flags", &databento::MboMsg::flags, "Flags")
    .def_readonly("depth", &databento::MboMsg::depth, "Depth level")
    .def_readonly("price", &databento::MboMsg::price, "Price (fixed-point 1e-9)")
    .def_readonly("size", &databento::MboMsg::size, "Order size")
    .def_readonly("channel_id", &databento::MboMsg::channel_id, "Channel ID")
    .def_readonly("order_id", &databento::MboMsg::order_id, "Order ID")
    .def_readonly("sequence", &databento::MboMsg::sequence, "Sequence number")
    .def_property_readonly("price_float", 
      [](const databento::MboMsg& m) { return databento::price_to_double(m.price); },
      "Price as float (convenience property)")
    .def("__repr__", [](const databento::MboMsg& m) {
      return "<MboMsg instrument_id=" + std::to_string(m.instrument_id) +
             " action=" + std::string(1, m.action) +
             " side=" + std::string(1, m.side) +
             " price=" + std::to_string(databento::price_to_double(m.price)) +
             " size=" + std::to_string(m.size) + ">";
    });

  // ============================================================================
  // TradeMsg struct
  // ============================================================================
  
  py::class_<databento::TradeMsg>(m, "TradeMsg")
    .def(py::init<>())
    .def_readonly("ts_event", &databento::TradeMsg::ts_event)
    .def_readonly("instrument_id", &databento::TradeMsg::instrument_id)
    .def_readonly("action", &databento::TradeMsg::action)
    .def_readonly("side", &databento::TradeMsg::side)
    .def_readonly("price", &databento::TradeMsg::price)
    .def_readonly("size", &databento::TradeMsg::size)
    .def_readonly("order_id", &databento::TradeMsg::order_id)
    .def_property_readonly("price_float",
      [](const databento::TradeMsg& m) { return databento::price_to_double(m.price); });

  // ============================================================================
  // ParseStats struct
  // ============================================================================
  
  py::class_<databento::ParseStats>(m, "ParseStats")
    .def_readonly("total_records", &databento::ParseStats::total_records)
    .def_readonly("elapsed_seconds", &databento::ParseStats::elapsed_seconds)
    .def_readonly("records_per_second", &databento::ParseStats::records_per_second)
    .def_readonly("throughput_gbps", &databento::ParseStats::throughput_gbps)
    .def("print", &databento::ParseStats::print)
    .def("__repr__", [](const databento::ParseStats& s) {
      return "<ParseStats records=" + std::to_string(s.total_records) +
             " rate=" + std::to_string(static_cast<uint64_t>(s.records_per_second)) + "/s" +
             " throughput=" + std::to_string(s.throughput_gbps) + " GB/s>";
    });

  // ============================================================================
  // DbnParser class
  // ============================================================================
  
  py::class_<databento::DbnParser>(m, "DbnParser")
    .def(py::init<const std::string&>(), py::arg("filepath"),
         "Create parser for DBN file")
    .def("load_into_memory", &databento::DbnParser::load_into_memory,
         "Load entire file into memory (zero-copy)")
    .def("parse_mbo", &databento::DbnParser::parse_mbo, py::arg("callback"),
         "Parse MBO records with callback function")
    .def("parse_trade", &databento::DbnParser::parse_trade, py::arg("callback"),
         "Parse Trade records with callback function")
    .def("num_records", &databento::DbnParser::num_records,
         "Get number of records in file")
    .def("record_size", &databento::DbnParser::record_size,
         "Get size of each record in bytes")
    .def("size", &databento::DbnParser::size,
         "Get total file size in bytes")
    .def("get_record_mbo", [](databento::DbnParser& parser, size_t idx) {
      if (!parser.data()) {
        parser.load_into_memory();
      }
      return databento::parse_mbo(parser.get_record(idx));
    }, py::arg("index"), "Get MBO record at index")
    .def("get_record_trade", [](databento::DbnParser& parser, size_t idx) {
      if (!parser.data()) {
        parser.load_into_memory();
      }
      return databento::parse_trade(parser.get_record(idx));
    }, py::arg("index"), "Get Trade record at index")
    .def("get_all_mbo", [](databento::DbnParser& parser) {
      if (!parser.data()) {
        parser.load_into_memory();
      }
      
      std::vector<databento::MboMsg> records;
      records.reserve(parser.num_records());
      
      for (size_t i = 0; i < parser.num_records(); ++i) {
        records.push_back(databento::parse_mbo(parser.get_record(i)));
      }
      return records;
    }, "Load all MBO records into Python list (fast!)")
    .def("__len__", &databento::DbnParser::num_records)
    .def("__repr__", [](const databento::DbnParser& p) {
      return "<DbnParser records=" + std::to_string(p.num_records()) +
             " size=" + std::to_string(p.size()) + " bytes>";
    });

  // ============================================================================
  // Utility functions
  // ============================================================================
  
  m.def("price_to_float", &databento::price_to_double, py::arg("price"),
        "Convert fixed-point price to float");
  m.def("float_to_price", &databento::double_to_price, py::arg("price"),
        "Convert float to fixed-point price");
  
  // ============================================================================
  // High-level parse functions
  // ============================================================================
  
  m.def("parse_file_mbo", 
    [](const std::string& filepath, py::function callback) {
      auto cpp_callback = [&callback](const databento::MboMsg& msg) {
        callback(msg);
      };
      return databento::parse_file_mbo(filepath, cpp_callback);
    },
    py::arg("filepath"),
    py::arg("callback"),
    "Parse MBO file with callback function");

  m.def("parse_file_mbo_fast", 
    [](const std::string& filepath) {
      databento::DbnParser parser(filepath);
      parser.load_into_memory();
      
      std::vector<databento::MboMsg> records;
      records.reserve(parser.num_records());
      
      for (size_t i = 0; i < parser.num_records(); ++i) {
        records.push_back(databento::parse_mbo(parser.get_record(i)));
      }
      
      return records;
    },
    py::arg("filepath"),
    "Fast parse: Load all MBO records into list (recommended!)");

  // ============================================================================
  // Version info
  // ============================================================================
  
  m.attr("__version__") = "1.0.0";
  m.attr("__author__") = "databento-fast contributors";
  m.attr("__description__") = "Ultra-fast alternative DBN parser (200M+ rec/s)";
}
