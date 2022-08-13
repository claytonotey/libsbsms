Content
=======

libsbsms is a library for high quality time and pitch scale modification.  It is based on octave subband sinusoidal modeling and resynthesis.  It stitches tracks between subbands, and it has multiple stages of analysis and resynthesis.

This branch is mostly for maintaning use in Audacity and its descendant.

I am working on a real-time version now, VST3 ready, rebuilt from the bottom up, for interactive spectrograms, which will go into a different repo.  Eventually I want a Cubase like sequencer/arranger, as well. I'm open to input for applications - just start an issue if you have any ideas.

There is a simple command line program 'sbsms-convert` in ./example which performs a sliding time stretch and pitch shift of a .wav or .mp3 file and outputs a .wav file.

API
=======
When reading the output from sbsms, you must determine a stopping condition for yourself, as the library zero pads the output ad infinitum and never returns 0 samples.  The simplest method for doing so is found in example/sbsms-convert.cpp.

