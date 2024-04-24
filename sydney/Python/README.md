# About

py-doquedb is python client library for DoqueDB. Its implementation is based on [PEP 249 – Python Database API Specification v2.0](https://peps.python.org/pep-0249/).

# Environment

- Python 3

# Download

Download latest wheel file from [Release](https://github.com/DoqueDB/doquedb/releases). The file name is py_doquedb-<version>-py3-none-any.whl.

# Installation

Install Python3 and execute command below to install py-doquedb.
`pip` shall be read as `pip3` depending on your local environment.

```
$ pip install py_doquedb-<version>-py3-none-any.whl
```

# Sample Code

You can check basic usage of py-doquedb in `sample.py`.
Execute command shown below to check how `sample.py` works.

```
$ cd sydney/Python
$ python sample.py
[(1, 'BTC', 10200), (2, 'ETH', 5000), (3, 'XEM', 2500), (4, 'XRP', 1000), (5, 'MONA', 3000), (6, 'XP', 1000)]
```

Also, check sample code in `sydney/tools/setup/package/doc/sample/Python` to learn further information about py-doquedb. It uses data from Aozora Bunko (Japanese free online library). Japanese description for how to use this sample code is available [here](https://doquedb.github.io/doquedb/howtouse.html#sec6).

# Development and Test Procedure

## Directory Details

```
sydney/Python
├── dist            // place of install packages
├── docs            // place for document files
|   └── source      // place for sphinx configuration files and auto generated reST files
├── src/doquedb     // place where source code for Python client are stored
├── test            // place where test code for Python client are stored
├── tools           // place for tools such as script for auto-generating error files
├── .gitigonre      // standard gitignore file
├── Pipfile         // file for managing install package and script used in pipenv
├── pytest.ini      // file for setting pytest
├── README_ja.md    // japanese README file
├── README.md       // english README file
├── sample.py       // sample code for py-doquedb
├── setup.cfg       // file for setting setuptools used for creating install package
└── setup.py        // script for creating install package
```

## Creating Virtual Environment

py-doquedb uses pipenv to create Python venv. About pipenv, see [https://pipenv-ja.readthedocs.io/ja/translate-ja/](https://pipenv-ja.readthedocs.io/ja/translate-ja/) for details.
Py-doquedb should work with any Python3, but development package like pytest has Python version requirement. So you might need to update Python version to fill the requirement of dev packages.

```
$ pip install pipenv
$ cd sydney/Python
$ pipenv install --dev
```

## Creating Error Files

Some files are auto generated and are not included in DoqueDB repository. So, you need to create error files before starting your own development. Run the script to create error files. By executing this command, files shown below will be created under `sydney/Python/src/exception`.

- database_exception.py
- errorcode.py
- message_format_eg.py
- message_format_jp.py
- raise_error.py

```
$ pipenv run make_exception
```

## Test

Execute test by running the script shown below.

```
$ pipenv run test
```

When all the tests succeed, you should get equivalent result as shown below. If failed test exists, `.` shown next to the test file name will be changed to `F` and details of failed tests will be added to the result.

```
$ pipenv run test
============================================== test session starts ===============================================
platform linux -- Python 3.11.7, pytest-8.1.1, pluggy-1.4.0
rootdir: <your rootdir will be shown here>
configfile: pytest.ini
testpaths: ./test
collected 378 items

test/client/test_connection.py ......ssss...                                                               [  3%]
test/client/test_datasource.py ................s..sssss..                                                  [ 10%]
test/client/test_port.py .................                                                                 [ 14%]
test/client/test_resultset.py ........s.....                                                               [ 18%]
test/client/test_session.py ................s.......                                                       [ 24%]
test/common/test_abstracts.py ..........                                                                   [ 27%]
test/common/test_arraydata.py .................                                                            [ 32%]
test/common/test_data.py ...............                                                                   [ 35%]
test/common/test_instance.py ....                                                                          [ 37%]
test/common/test_iostream.py ..........................                                                    [ 43%]
test/common/test_language.py ...................                                                           [ 48%]
test/common/test_scalardata.py .....................ss..................................ss....             [ 65%]
test/common/test_serialdata.py ......................s..                                                   [ 72%]
test/common/test_unicodestr.py ........                                                                    [ 74%]
test/driver/test_connection.py ..................................                                          [ 83%]
test/driver/test_cursor.py ........................ssss..                                                  [ 91%]
test/exception/test_error_message.py .                                                                     [ 91%]
test/exception/test_raise_error.py .                                                                       [ 91%]
test/port/test_connection.py ......                                                                        [ 93%]
test/port/test_constants.py .                                                                              [ 93%]
test/test_doquedb.py ........                                                                              [ 95%]
test/test_scenario.py ................                                                                     [100%]

================================== 357 passed, 21 skipped, 0 warnings in 29.16s ==================================
```

## Packaging

You can create py-doquedb package by executing script shown below. `dist/py_doquedb-<version>-py3-none-any.whl`, build files under `build/`, and `src/py_doquedb.egg-info` will be created by executing the command. Informations written in `setup.cfg` are used to create package. If needed, update informations in `setup.cfg` such as version before executing the command.

```
pipenv run make_package
```

## Creating Documents

You can create documents by executing scripts shown below. Sphinx is used for creating documents. `sphinx-apidoc -f -o ./docs/source/ ./src` creates reST files under `docs/source` from docstring written in source codes. `sphinx-build -b html docs/source docs/build` creates HTML under `docs/build/` from reST files. See details in [Sphinx Documentation](https://www.sphinx-doc.org/en/master/index.html).

```
$ cd sydney/Python
$ pipenv shell
$ export LC_ALL=C # avoid locale error
$ pip install -e . # avoid module import error
$ sphinx-apidoc -f -o ./docs/source/ ./src # generate reST files under source
$ sphinx-build -b html docs/source docs/build # generate HTML under build
```

# Document

- [Python Driver](https://doquedb.github.io/doquedb-pythondoc/index.html)
