# Contributing to databento-fast

Thank you for your interest in contributing to databento-fast! This document provides guidelines for contributing to the project.

## Getting Started

1. Fork the repository
2. Clone your fork: `git clone https://github.com/YOUR_USERNAME/databento-fast.git`
3. Create a feature branch: `git checkout -b feature/your-feature-name`
4. Make your changes
5. Test your changes
6. Submit a pull request

## Development Setup

### Requirements

- C++20 compatible compiler (GCC 10+, Clang 12+, MSVC 2019+)
- CMake 3.15+
- Python 3.7+ (for Python bindings)
- pybind11 (for Python bindings)
- Google Test (automatically downloaded by CMake)

### Building

```bash
# C++ only
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON ..
make

# With Python bindings
pip install pybind11
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON -DBUILD_PYTHON_BINDINGS=ON ..
make
```

## Testing

### C++ Tests

```bash
cd build
make test
# or
ctest --output-on-failure
```

### Python Tests

```bash
cd tests
python3 test_python_bindings.py -v
```

## Code Style

### C++

- Use modern C++20 features
- Follow existing code style (2 spaces for indentation)
- Use meaningful variable names
- Add comments for complex logic
- Keep functions focused and small

Example:
```cpp
// Good
const MBOMsg* DirectParser::next() {
    if (!has_next()) {
        return nullptr;
    }
    
    const MBOMsg* msg = reinterpret_cast<const MBOMsg*>(data_ + offset_);
    offset_ += sizeof(MBOMsg);
    records_count_++;
    
    return msg;
}

// Avoid
const MBOMsg* DirectParser::next(){const MBOMsg* m=reinterpret_cast<const MBOMsg*>(data_+offset_);offset_+=sizeof(MBOMsg);return m;}
```

### Python

- Follow PEP 8
- Use type hints where appropriate
- Write docstrings for functions

## Adding Features

### New Parser API

If you want to add a new parser API:

1. Add the class declaration to `include/databento/dbn_parser.hpp`
2. Implement the class in `src/dbn_parser.cpp`
3. Add Python bindings in `python/bindings.cpp`
4. Add tests in `tests/test_parser.cpp` and `tests/test_python_bindings.py`
5. Update README.md and USAGE.md with examples
6. Update performance metrics if applicable

### New Record Type

If you want to support a new DBN record type:

1. Define the structure in `include/databento/dbn_parser.hpp`
2. Add parsing functions for each API level
3. Add Python bindings
4. Add comprehensive tests
5. Update documentation

## Testing Guidelines

- Write tests for all new features
- Maintain 100% test pass rate
- Add both positive and negative test cases
- Test edge cases (empty data, single record, large datasets)
- Verify memory safety and performance

Test structure:
```cpp
TEST(CategoryTest, FeatureName) {
    // Arrange
    auto data = create_test_data(100);
    
    // Act
    DirectParser parser(data.data(), data.size());
    
    // Assert
    EXPECT_TRUE(parser.has_next());
    EXPECT_EQ(parser.records_parsed(), 0);
}
```

## Performance Considerations

- Keep Direct API at 330M+ records/sec
- Keep Batch API at 150M+ records/sec
- Keep Callback API at 30M+ records/sec
- Use zero-copy techniques where possible
- Minimize memory allocations
- Profile changes before submitting

Benchmark your changes:
```bash
cd build
./example_usage large_dataset.dbn
```

## Documentation

Update documentation for:

- New features
- API changes
- Performance changes
- Breaking changes

Files to update:
- `README.md` - Overview and quick start
- `USAGE.md` - Detailed usage guide
- `CONTRIBUTING.md` - This file
- Code comments - Document complex logic

## Pull Request Process

1. **Create PR**: Submit a pull request with a clear title and description
2. **Tests**: Ensure all tests pass (`make test`)
3. **Documentation**: Update relevant documentation
4. **Code Review**: Address reviewer feedback
5. **Merge**: Maintainer will merge once approved

### PR Title Format

- `feat: Add new feature`
- `fix: Fix bug description`
- `docs: Update documentation`
- `perf: Improve performance`
- `refactor: Refactor code`
- `test: Add or update tests`

### PR Description Template

```markdown
## Description
Brief description of changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Performance improvement
- [ ] Documentation update
- [ ] Refactoring

## Testing
- [ ] All existing tests pass
- [ ] Added new tests for new features
- [ ] Manually tested on Intel Xeon (or similar)

## Performance Impact
- [ ] No performance impact
- [ ] Performance improved by X%
- [ ] Performance regression addressed

## Documentation
- [ ] Updated README.md
- [ ] Updated USAGE.md
- [ ] Updated code comments
- [ ] Added examples
```

## Bug Reports

When reporting bugs, include:

1. **Description**: Clear description of the bug
2. **Steps to Reproduce**: Minimal code to reproduce
3. **Expected Behavior**: What should happen
4. **Actual Behavior**: What actually happens
5. **Environment**: OS, compiler, Python version
6. **Version**: Git commit hash or release version

Example:
```markdown
**Bug**: Parser crashes on empty file

**Steps to Reproduce**:
```python
import databento_cpp
records = databento_cpp.parse_file_mbo_fast("empty.dbn")
```

**Expected**: Return empty list or raise exception
**Actual**: Segmentation fault

**Environment**: Ubuntu 22.04, GCC 11.2, Python 3.10
**Version**: commit abc1234
```

## Feature Requests

For feature requests, include:

1. **Use Case**: Why is this feature needed?
2. **Proposed Solution**: How should it work?
3. **Alternatives**: Other approaches considered
4. **Performance**: Expected performance impact

## Code Review

As a reviewer:

- Be constructive and respectful
- Focus on code quality, performance, and maintainability
- Ask questions rather than making demands
- Approve when ready, request changes if needed

As a contributor:

- Be open to feedback
- Respond to comments promptly
- Ask for clarification if needed
- Update PR based on feedback

## Release Process

(For maintainers)

1. Update version in `setup.py`, `pyproject.toml`, `CMakeLists.txt`
2. Update CHANGELOG.md
3. Run full test suite
4. Tag release: `git tag v1.0.0`
5. Push tag: `git push origin v1.0.0`
6. Create GitHub release
7. Build and upload to PyPI

## Community

- Be respectful and inclusive
- Help others when you can
- Share knowledge and experiences
- Follow the code of conduct

## Questions?

- Open an issue for questions
- Check existing issues and documentation first
- Provide context and details

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

## Acknowledgments

Thank you for contributing to databento-fast! Your contributions help make high-performance market data parsing accessible to everyone.
