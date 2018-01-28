#!/usr/bin/python

#Print Atari 2600 Timer Values
#Value to Write to TIM64T to wait desired number of scanlines

for scanlines in range(0,262):
  print scanlines, "\t", scanlines*72/64, "\t", scanlines*72.0/64  
  
  
