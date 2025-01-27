/*
 *  Copyright 2009,2010,2011 Reality Jockey, Ltd.
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
#include "DspEnvelope.h"
#include "PdGraph.h"

/** By default, the analysis window size is 1024 samples. */
#define DEFAULT_WINDOW_SIZE 1024

MessageObject *DspEnvelope::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspEnvelope(initMessage, graph);
}

DspEnvelope::DspEnvelope(PdMessage *initMessage, PdGraph *graph) : DspObject(0, 1, 1, 0, graph) {
  if (initMessage->isFloat(0)) {
    if (initMessage->isFloat(1)) {
      // if two parameters are provided, set the window size and window interval
      windowSize = initMessage->getFloat(0);
      windowInterval = initMessage->getFloat(1);
    } else {
      // if one parameter is provided, set the window size
      windowSize = (int) initMessage->getFloat(0);
      setWindowInterval(windowSize/2);
    }
  } else {
    // otherwise, use default values for the window size and interval
    windowSize = DEFAULT_WINDOW_SIZE;
    windowInterval = windowSize / 2;
  }
  
  // NOTE(mhroth): I haven't thought very much if this fix could be better done. The issue is that
  // if the blocksize is large (e.g., larger than the windowInterval), then env~ will never send
  // a message. With more logic, a block size larger than the window size could be accomodated. But
  // I am too lazy to consider this option at the moment. Thus, currently the window size and interval
  // must be at least as large as the block size.
  if (windowSize < graph->getBlockSize()) {
    graph->printErr("env~ window size must be at least as large as the block size. %i reset to %i.",
        windowSize, graph->getBlockSize());
    windowSize = graph->getBlockSize();
  }
  if (windowInterval < graph->getBlockSize()) {
    graph->printErr("env~ window interval must be at least as large as the block size. %i reset to %i.",
        windowInterval, graph->getBlockSize());
    windowInterval = graph->getBlockSize();
  }
  
  processFunction = &processSignal;
  
  initBuffers();
}

DspEnvelope::~DspEnvelope() {
  free(signalBuffer);
  free(hanningCoefficients);
}

string DspEnvelope::toString() {
  char* str(static_cast<char*>(alloca(snprintf(NULL, 0, "%s %i %i", getObjectLabel(), windowSize, windowInterval)+1)));
  snprintf(str, sizeof(str), "%s %i %i", getObjectLabel(), windowSize, windowInterval);
  return string(str);
}

void DspEnvelope::setWindowInterval(int newInterval) {
  int i = newInterval % graph->getBlockSize();
  if (i == 0) {
    // windowInterval is a multiple of blockSize. Awesome :)
    this->windowInterval = newInterval;
  } else if (i <= graph->getBlockSize()/2) {
    // windowInterval is closer to the smaller multiple of blockSize
    this->windowInterval = (newInterval/graph->getBlockSize())*graph->getBlockSize();
  } else {
    // windowInterval is closer to the larger multiple of blockSize
    this->windowInterval = ((newInterval/graph->getBlockSize())+1)*graph->getBlockSize();
  }
}

void DspEnvelope::initBuffers() {
  // ensure that the buffer is big enough to take the number of whole blocks needed to fill it
  numSamplesReceived = 0;
  numSamplesReceivedSinceLastInterval = 0;
  int numBlocksPerWindow = (windowSize % graph->getBlockSize() == 0) ? (windowSize/graph->getBlockSize()) : (windowSize/graph->getBlockSize()) + 1;
  int bufferSize = numBlocksPerWindow * graph->getBlockSize();
  signalBuffer = (float *) malloc(bufferSize * sizeof(float));
  hanningCoefficients = (float *) malloc(bufferSize * sizeof(float));
  float N_1 = (float) (windowSize - 1); // (N == windowSize) - 1
  float hanningSum = 0.0f;
  for (int i = 0; i < windowSize; i++) {
    // calcualte the hanning window coefficients
    hanningCoefficients[i] = 0.5f * (1.0f - cosf((2.0f * M_PI * (float) i) / N_1));
    hanningSum += hanningCoefficients[i];
  }
  for (int i = 0; i < windowSize; i++) {
    // normalise the hanning coefficients such that they represent a normalised weighted averaging
    hanningCoefficients[i] /= hanningSum;
  }
}

// windowSize and windowInterval are constrained to be multiples of the block size
void DspEnvelope::processSignal(DspObject *dspObject, int fromIndex, int toIndex) {
  DspEnvelope *d = reinterpret_cast<DspEnvelope *>(dspObject);
  
  // copy the input into the signal buffer
  memcpy(d->signalBuffer + d->numSamplesReceived, d->dspBufferAtInlet[0], toIndex*sizeof(float));
  d->numSamplesReceived += toIndex;
  d->numSamplesReceivedSinceLastInterval += toIndex;
  if (d->numSamplesReceived >= d->windowSize) {
    d->numSamplesReceived = 0;
  }
  if (d->numSamplesReceivedSinceLastInterval == d->windowInterval) {
    d->numSamplesReceivedSinceLastInterval -= d->windowInterval;
    // apply hanning window to signal and calculate Root Mean Square
    float rms = 0.0f;
    #if __APPLE__
    float rmsBuffer[d->windowSize];
    vDSP_vsq(d->signalBuffer, 1, rmsBuffer, 1, d->windowSize); // signalBuffer^2 
    vDSP_vmul(rmsBuffer, 1, d->hanningCoefficients, 1, rmsBuffer, 1, d->windowSize); // * hanning window
    vDSP_sve(rmsBuffer, 1, &rms, d->windowSize); // sum the result
    #else
    for (int i = 0; i < d->windowSize; ++i) {
      rms += d->signalBuffer[i] * d->signalBuffer[i] * d->hanningCoefficients[i];
    }
    #endif
    // finish RMS calculation. sqrt is removed as it can be combined with the log operation.
    // result is normalised such that 1 RMS == 100 dB
    rms = 10.0f * log10f(rms) + 100.0f;

    PdMessage *outgoingMessage = PD_MESSAGE_ON_STACK(1);
    // graph will schedule this at the beginning of the next block because the timestamp will be
    // behind the block start timestamp
    outgoingMessage->initWithTimestampAndFloat(0.0, (rms < 0.0f) ? 0.0f : rms);
    d->graph->scheduleMessage(d, 0, outgoingMessage);
  }
}
