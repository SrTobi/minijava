#! /usr/bin/python3
#! -*- coding:utf-8; mode:python; -*-

import json as _json
import re as _re
import sys as _sys


class InvalidManifestError(Exception):

    """
    @brief
        Exception used to signal invalid configuration files.

    """

    def __init__(self, reason=None):
        super().__init__(
            "Invalid manifest file" + ("" if reason is None else ": " + reason)
        )


class ManifestLoader(object):

    __NAME_PATTERN = _re.compile(r'\w+')

    def __init__(self):
        pass

    def load(self, filename):
        """
        @brief
            Loads the data from the JSON file named `filename` into a Python
            data-structure and validates it.

        As an extension to the functionality provided by the `json` module from the
        standard library (and the JSON RFC), this function also allows very basic
        comments in the file.  Currently, only line-comments that are on a line of
        their own are considered.

        If `filename` is `-`, standard input is read.

        @param filename : str
            name of the file to read

        @returns
            parsed JSON data

        @raises InvalidManifestError
            if the Json could be parsed but was not a valid benchmark
            configuration

        @raises ...
            any exceptions raised by reading the file or the JSON parser

        """
        with _sys.stdin if filename == '-' else open(filename, 'r') as istr:
            # Clearly, it would be more natural to use a filter() than a map()
            # here.  But this would cause the JSON parser to report nonsensical
            # line number in case of a syntax error so we do it this way instead.
            config = _json.loads('\n'.join(map(
                lambda l : l if not l.lstrip().startswith('//') else '',
                istr
            )))
            self.__validate(config)
            return config

    def __validate(self, config):
        if type(config) is not dict:
            raise InvalidManifestError()
        for (key, value) in config.items():
            if type(key) is not str or type(value) is not dict:
                raise InvalidManifestError()
            if not ManifestLoader.__NAME_PATTERN.match(key):
                raise InvalidManifestError(key + ": Not a valid name for a benchmark")
            self._validate_stanza(key, value)

    def _validate_stanza(self, name, stanza):
        """
        @brief
            Validates the definition `stanza` for the benchmark `name`.

        Concrete classes should override this method to either pass
        successfully (if the configuration is valid) or else `raise` an
        `InvalidManifestError` and not `return`.

        @param name : str
            valid benchmark name

        @param stanza : dict
            benchmark definition to validate

        @raises InvalidManifestError
            if `config` is not a valid manifest

        """
        raise NotImplementedError('lib.manifest.ManifestLoader._validate_stanza')
