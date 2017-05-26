## An Open Image Pipeline convolution matrix plugin

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

#### Copyright Eero Talus 2017
