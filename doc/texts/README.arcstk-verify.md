\page arcstk-verify

arcstk-verify, an example tool to calculate and verify AccurateRip Track
Checksums v1 and v2 of losslessly encoded audio files, either for albums or for
single tracks.


Functions:

* Compute AccurateRip v1 and v2 track checksums for all tracks in audio file
* multiple-track albums and single-track audio files are supported
* Respect CUE files for album/multiple-track files (a single audio file
  representing an entire CD)


Bugs and limitations:

* Not very well tested
* Only works on little endian systems
* CDs with data tracks are completely untested


Documentation:

* See the manpage(1)
