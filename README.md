# Example Toolkit for AccurateRip checksums

[![MIT licensed](https://img.shields.io/badge/license-MIT-blue.svg)](./LICENSE)
[![C++17](https://img.shields.io/badge/C++-17-darkblue.svg)](./API.md)


## Introduction

This is an example command line frontend to [libarcstk][1]. It contains tools to
work with AccurateRip checksums of your CD rip, namely

- [arcstk-id](./doc/texts/README.arcstk-id.md) - Calculate AccurateRip id
  for requesting checksums
- [arcstk-calc](./doc/texts/README.arcstk-calc.md) - Calculate actual
  AccurateRip checksums locally for albums (single audio files) as well arcstk
  for single tracks
- [arcstk-verify](./doc/texts/README.arcstk-verify.md) - Verify local
  AccurateRip checksums against reference checksums from AccurateRip
- [arcstk-parse](./doc/texts/README.arcstk-parse.md) - Parse AccurateRip
  response to plaintext

Tool [arcstk-calc](./doc/texts/README.arcstk-calc.md) makes use of
[libarcsdec][2] and will accept nearly any losslessly encoded audio input
depending on the decoder libraries your system provides. (WMALossless is
currently not supported, but this is solely due to the current lack of a Windows
port. But wav, flac, wavpack, ALAC, ape, AIFF input is possible and currently at
least smoke-tested.)


## Examples

Calculate the AccurateRip URL of a local CD image (audiofile may be omitted if
referenced in the CUESheet):
```console
$ arcstk-id --url /path/to/metafile.cue -a /path/to/audiofile.flac
```
Calculate AccurateRip checksums from local CD images:
```console
$ arcstk-calc -m /path/to/metafile.cue /path/to/audiofile.flac
```
Fetch AccurateRip data for a CD and save it in binary format (requires curl):
```console
$ curl -o response.bin "$(arcstk-id --url /path/to/metafile.cue -a /path/to/audiofile.flac)"
```
Verify AccurateRip checksums from local CD images:
```console
$ arcstk-verify -r /path/to/dBAR-responsefile.bin -m /path/to/metafile.cue -a /path/to/audiofile.flac
```
Fetch AccurateRip data for a CD and parse it to readable text format (requires
curl):
```console
$ curl "$(arcstk-id --url /path/to/metafile.cue -a /path/to/audiofile.flac)" | arcstk-parse > response.txt
```
Fetch AccurateRip data for a CD and verify the local image (requires curl):
```console
$ curl "$(arcstk-id --url /path/to/cuefile.cue -a /path/to/audiofile.flac)" | arcstk-verify -m /path/to/cuefile.cue /path/to/audiofile
```

[1]: https://codeberg.org/tristero/libarcstk
[2]: https://codeberg.org/tristero/libarcsdec

