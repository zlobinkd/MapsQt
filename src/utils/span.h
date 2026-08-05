#pragma once

#include "src/core.h"

template<class T>
class Span {
public:
    Span(T* startElement, const size_t numElements) : _startElement(startElement), _numElements(numElements) {}

    T* begin() { return _startElement; }
    const T* begin() const { return _startElement; }
    T* end() { return _startElement + _numElements; }
    const T* end() const { return _startElement + _numElements; }

private:
    T* _startElement;
    size_t _numElements;
};
