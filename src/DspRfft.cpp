/*
 *  Copyright 2010 Reality Jockey, Ltd.
 *                 info@rjdj.me
 *                 http://rjdj.me/
 *
 *  This file is part of ZenGarden.
 *
 *  ZenGarden is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  ZenGarden is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with ZenGarden.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ArrayArithmetic.h"
#include "DspRfft.h"
#include "PdGraph.h"

DspRfft::DspRfft(PdMessage *initMessage, PdGraph *graph) : DspObject(0, 1, 0, 2, graph) {
  #ifdef __APPLE__
  log2n = lrintf(log2f(blockSizeFloat));
  fftSetup = vDSP_create_fftsetup(log2n, kFFTRadix2);
  #else
  graph->printErr("[rfft~] is not supported on this platform. It is only supported on Apple OS X and iOS platforms.");
  #endif // __APPLE__
}

DspRfft::~DspRfft() {
  #ifdef __APPLE__
  vDSP_destroy_fftsetup(fftSetup);
  #endif // __APPLE__
}

const char *DspRfft::getObjectLabel() {
  return "rfft~";
}

void DspRfft::processDspWithIndex(int fromIndex, int toIndex) {
  #ifdef __APPLE__
  DSPSplitComplex inputVector;
  inputVector.realp = dspBufferAtInlet0;
  inputVector.imagp = DspObject::zeroBuffer;
  DSPSplitComplex outputVector;
  outputVector.realp = dspBufferAtOutlet0;
  outputVector.imagp = dspBufferAtOutlet[1];
  vDSP_fft_zop(fftSetup, &inputVector, 1, &outputVector, 1, log2n, kFFTDirection_Forward);
  
  // vDSP_fft_zop outputs the entire series of symmetric coefficients. Pd only returns the unique values
  int halfBlockSize = blockSizeInt >> 1;
  memset(dspBufferAtOutlet0+halfBlockSize+1, 0, (halfBlockSize-1) * sizeof(float));
  memset(dspBufferAtOutlet[1]+halfBlockSize, 0, halfBlockSize * sizeof(float));
  #endif // __APPLE__
}
