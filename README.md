Content
=======

libsbsms is a library for high quality time and pitch scale modification.  It uses octave subband sinusoidal modeling.

There is a simple command line program 'sbsms-convert` in ./example which performs a sliding time stretch and pitch shift of a .wav or .mp3 file and outputs a .wav file.

NOTE:
When reading the output from sbsms, you must determine a stopping condition for yourself, as the library zero pads the output ad infinitum and never returns 0 samples.  The simplest method for doing so is found in example/sbsms-convert.cpp.

