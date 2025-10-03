#!/usr/bin/env python3
"""
Python unit tests for databento_cpp module
"""

import sys
import os
import unittest
import tempfile

# Add build directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'build'))

import databento_cpp

class TestDatabentoFast(unittest.TestCase):
    """Test suite for databento-fast Python bindings"""
    
    @classmethod
    def setUpClass(cls):
        """Create a temporary test file"""
        cls.test_file = os.path.join(tempfile.gettempdir(), 'test_data.dbn')
        
        # Check if we can use the example file instead
        example_file = os.path.join(os.path.dirname(__file__), '..', 'examples', 'sample_data.dbn')
        if os.path.exists(example_file):
            cls.test_file = example_file
            cls.num_records = 1000
        else:
            # Create minimal test data
            cls.num_records = 10
            # Note: In production, we'd create proper test data here
    
    def test_direct_api(self):
        """Test Direct API (parse_file_mbo_fast)"""
        records = databento_cpp.parse_file_mbo_fast(self.test_file)
        self.assertEqual(len(records), self.num_records)
        
        # Verify first record
        if records:
            self.assertIsNotNone(records[0].order_id)
            self.assertIsNotNone(records[0].price)
            self.assertIsNotNone(records[0].size)
    
    def test_batch_api(self):
        """Test Batch API (parse_file_mbo_batch)"""
        records = databento_cpp.parse_file_mbo_batch(self.test_file, batch_size=100)
        self.assertEqual(len(records), self.num_records)
    
    def test_callback_api(self):
        """Test Callback API (parse_file_mbo_callback)"""
        count = [0]
        
        def callback(record):
            count[0] += 1
            self.assertIsNotNone(record.order_id)
        
        databento_cpp.parse_file_mbo_callback(self.test_file, callback)
        self.assertEqual(count[0], self.num_records)
    
    def test_record_structure(self):
        """Test record structure access"""
        records = databento_cpp.parse_file_mbo_fast(self.test_file)
        if records:
            r = records[0]
            
            # Test RecordHeader fields
            self.assertIsNotNone(r.hd.length)
            self.assertIsNotNone(r.hd.rtype)
            self.assertIsNotNone(r.hd.publisher_id)
            self.assertIsNotNone(r.hd.product_id)
            self.assertIsNotNone(r.hd.ts_event)
            
            # Test MBOMsg fields
            self.assertIsNotNone(r.order_id)
            self.assertIsNotNone(r.price)
            self.assertIsNotNone(r.size)
            self.assertIsNotNone(r.flags)
            self.assertIsNotNone(r.channel_id)
            self.assertIsNotNone(r.action)
            self.assertIsNotNone(r.side)
            self.assertIsNotNone(r.ts_recv)
            self.assertIsNotNone(r.ts_in_delta)
            self.assertIsNotNone(r.sequence)
            self.assertIsNotNone(r.symbol_id)
    
    def test_consistency(self):
        """Test that all APIs return same results"""
        records_direct = databento_cpp.parse_file_mbo_fast(self.test_file)
        records_batch = databento_cpp.parse_file_mbo_batch(self.test_file, batch_size=50)
        
        self.assertEqual(len(records_direct), len(records_batch))
        
        # Verify first and last records match
        if records_direct and records_batch:
            self.assertEqual(records_direct[0].order_id, records_batch[0].order_id)
            self.assertEqual(records_direct[-1].order_id, records_batch[-1].order_id)
    
    def test_invalid_file(self):
        """Test error handling for invalid file"""
        with self.assertRaises(Exception):
            databento_cpp.parse_file_mbo_fast("/nonexistent/file.dbn")
    
    def test_module_functions(self):
        """Test that all expected functions are available"""
        self.assertTrue(hasattr(databento_cpp, 'parse_file_mbo_fast'))
        self.assertTrue(hasattr(databento_cpp, 'parse_file_mbo_batch'))
        self.assertTrue(hasattr(databento_cpp, 'parse_file_mbo_callback'))
        self.assertTrue(hasattr(databento_cpp, 'load_file'))
    
    def test_module_classes(self):
        """Test that all expected classes are available"""
        self.assertTrue(hasattr(databento_cpp, 'DirectParser'))
        self.assertTrue(hasattr(databento_cpp, 'BatchParser'))
        self.assertTrue(hasattr(databento_cpp, 'CallbackParser'))
        self.assertTrue(hasattr(databento_cpp, 'MBOMsg'))
        self.assertTrue(hasattr(databento_cpp, 'RecordHeader'))

if __name__ == '__main__':
    unittest.main()
