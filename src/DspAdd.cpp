/*
 *  Copyright 2009,2010,2011,2012 Reality Jockey, Ltd.
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
#include "DspAdd.h"

class PdGraph;

MessageObject *DspAdd::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspAdd(initMessage, graph);
}

DspAdd::DspAdd(PdMessage *initMessage, PdGraph *graph) : DspObject(2, 2, 0, 1, graph) {
  constant = initMessage->isFloat(0) ? initMessage->getFloat(0) : 0.0f;
  processFunctionNoMessage = &processScalar;
}

DspAdd::~DspAdd() {
  // nothing to do
}

void DspAdd::onInletConnectionUpdate(unsigned int inletIndex) {
  processFunction = (incomingDspConnections[0].size() > 0 && incomingDspConnections[1].size() > 0)
      ? &processSignal : &processScalar;
}

std::string DspAdd::toString() {
  const char *fmt = (constant == 0.0f) ? "%s" : "%s %g";
  char* str(static_cast<char*>(alloca(snprintf(NULL, 0, fmt, getObjectLabel(), constant) + 1)));
  snprintf(str, sizeof(str), fmt, getObjectLabel(), constant);
  return str;
}

void DspAdd::processMessage(int inletIndex, PdMessage *message) {
  if (inletIndex == 1 && message->isFloat(0)) {
    constant = message->getFloat(0);
  }
}

void DspAdd::processSignal(DspObject *dspObject, int fromIndex, int toIndex) {
  DspAdd *d = reinterpret_cast<DspAdd *>(dspObject);
  ArrayArithmetic::add(d->dspBufferAtInlet[0] , d->dspBufferAtInlet[1],
      d->dspBufferAtOutlet[0], 0, toIndex);
}

void DspAdd::processScalar(DspObject *dspObject, int fromIndex, int toIndex) {
  DspAdd *d = reinterpret_cast<DspAdd *>(dspObject);
  ArrayArithmetic::add(d->dspBufferAtInlet[0] , d->constant,
      d->dspBufferAtOutlet[0], fromIndex, fromIndex);
}
