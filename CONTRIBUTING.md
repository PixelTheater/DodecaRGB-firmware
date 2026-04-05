# Contributing to DodecaRGB

Pull requests are welcome! Here's how to get started.

## Development Setup

1. Clone the repo and open it in VSCode/Cursor with the PlatformIO extension
2. For Python utilities, see the [Development Guide](docs/guides/development.md#python-environment) for environment setup
3. Review the [Coding Guidelines](docs/guides/coding_guidelines.md) for project standards

## Running Tests

Before submitting a PR, make sure tests pass:

```bash
# C++ unit tests
pio test -e native

# Python utility tests
python -m util.tests.run_tests
```

## What to Contribute

- New animation scenes (see [Creating Animations Guide](docs/guides/creating_animations.md))
- Bug fixes and improvements to the PixelTheater library
- Documentation updates
- Python utility improvements
- Web simulator enhancements

## Submitting Changes

1. Fork the repository and create a feature branch
2. Make your changes
3. Run the test suites
4. Submit a pull request with a clear description of the changes

## Questions?

Open an issue on GitHub if you have questions or want to discuss a feature idea before starting work.
