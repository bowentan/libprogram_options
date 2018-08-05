# libprogram\_options

A C library for adding and parsing command line options without any third party libraries. The library provides the following features:
- Internally checks the argument and value types for validation.
- Allows dependencies between options (single or multiple).

Since this library is under development, many features are not included now and some bugs may occur. We apologize for that and we are glad to receive any bug reports and feature desires.

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

### Prerequisites

You will need a linux system with the gcc compiler compile and install the library.

### Installing

Clone this repository or download the distribution package in [GitHub releases](https://github.com/bowentan/libprogram_options/releases) to your local directory. We recommend to download the source package in the release pages since it is more robust in configuring and compiling.

Unzip or untar the file if necessary and begin compiling and install as follows.

```bash
cd libprogram_options_vX.X.X
./configure --prefix=/path/to/your/install/directory
make
make install
```

Then you are done with the installation if no error occurs. In the case that any error occurs that you cannot resolve, please feel free to approach us and we can help you as far as we can.

## Usage

We include some usage examples for this library. Please check them in the `example` directory.

## Versioning

Current version is v0.0.0. 

We use [SemVer](http://semver.org/) for versioning. For the versions available, see the tags on this repository and [the release page](https://github.com/bowentan/libprogram_options/releases).

## Bug reports and functionality adding

If you find any bugs or you have some ideas about the library functionality, please feel free to tell us in the issues.

## Authors

* **Bowen Tan** - *Initial work* - [GitHub](https://github.com/bowentan), Eamil: notebowentan@gmail.com

## License

Not licensed yet, future consideration.
