## An Open Image Pipeline Convolution Matrix plugin

A plugin used for convolution matrix based image processing with the
Open Image Pipeline. Image processing using convolution matrices is a
technique that's very versatile since it makes it possible to add many
different kinds of effects onto an image using just one plugin. The
effects are determined based on a square matrix and a divisor that
both have an effect on the resulting image. The easiest way to find
suitable convolution matrices and their divisors is to search the internet.

## Plugin arguments

`kernel`  
* This argument specifies the convolution matrix. The value should be
  a list of nine integers separated by commas. Each consequent set of
  three integers is one line from top to bottom ie. 1,2,3,4,5,6,7,8,9
  corresponds to the matrix

	`| 1 2 3 |
	 | 4 5 6 |
	 | 7 8 9 |`

`divisor`  
* This argument specifies the divisor used with the kernel. The value
  should be a floating point number.

`channels`  
* This argument specifies the channels that will be affected by the
  convolution matrix. The value of this argument should be a
  combination of the letters R, G, B and A where  

	`R = Red channel  
	 G = Green channel  
	 B = Blue channel  
	 A = Alpha channel`  

  For example, the value RB would enable the red and blue channels.  

## Compiling

To configure the local build environment, first execute the file named
`config-build-env.sh`. The shell script will prompt you for some information
that's needed by the makefile. After this script has finished, you can run `make`
and then `make install`, which will copy the created shared library file to
the `plugins` directory in the main OIP directory. The makefile also has the
following targets  

#### clean

Clean all the files created by the compilation.  

#### clean-hard

Clean all the files created by the compilation and also delete the build environment
configuration file.  

#### LOC

Count the total lines of code for this plugin.  

## Open Source libraries used

### Freeimage

This software uses the FreeImage open source image library.
See http://freeimage.sourceforge.net for details.
FreeImage is used under the GNU GPL, version 3.

You can find the full license in the file FreeImage-License-GNU-GPLv3.txt
in the source root of Open Image Pipeline Convolution Matrix plugin.

## License

Copyright 2017 Eero Talus

This file is part of Open Image Pipeline Convolution Matrix plugin.

Open Image Pipeline Convolution Matrix plugin is free software: you
can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.

Open Image Pipeline Convolution Matrix plugin is distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Open Image Pipeline Convolution Matrix plugin.
If not, see <http://www.gnu.org/licenses/>.
