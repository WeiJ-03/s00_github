#include "Tensor.h"
#include <fstream>
#include <cmath>
#include <cassert>
#include "Define.h"

using std::string;
using std::ofstream;
using std::endl;
using std::dec;

template <typename T>
Tensor<T>::Tensor(int ch, int psum, int r, int c):
    m_ch(ch), m_psumDim(psum), m_row(r), m_col(c) {
    m_buffer.resize(ch * psum * r * c, 0);
}

template <typename T>
Tensor<T>::Tensor(const Tensor & p) { // Copy constructor
    m_row = p.m_row;
    m_psumDim = p.m_psumDim;
    m_col = p.m_col;
    m_ch = p.m_ch;
    m_buffer.clear();
    m_buffer = p.m_buffer;
}

template <typename T>
bool Tensor<T>::BoundaryCheck(int chn, int psum, int r, int c) {
    return !(r >= m_row || c >= m_col || psum >= m_psumDim || chn >= m_ch || r < 0 || c < 0 || psum < 0 || chn < 0);
}

template <typename T>
bool Tensor<T>::BoundaryCheck(int chn, int psum, int r, int c) const {
    return !(r >= m_row || c >= m_col || psum >= m_psumDim || chn >= m_ch || r < 0 || c < 0 || psum < 0 || chn < 0);
}

template <typename T>
Tensor<T>& Tensor<T>::operator=(const Tensor & p) { // Assignment operator
    m_row = p.m_row;
    m_psumDim = p.m_psumDim;
    m_col = p.m_col;
    m_ch = p.m_ch;
    m_buffer.clear();
    m_buffer = p.m_buffer;
    return *this;
}

template <typename T>
Tensor<T> Tensor<T>::operator*=(Tensor const& p) { // Multiplication: element-wise
    if (!(m_ch == p.m_ch && m_psumDim == p.m_psumDim && m_row == p.m_row && m_col == p.m_col)) {
        throw "Error: Input tensor dimension does not match expected value";
    }
    int size = m_ch * m_psumDim * m_row * m_col;
    for (int i = 0; i < size; i++) {
        m_buffer[i] *= p.m_buffer[i];
    }
    return *this;
}

template <typename T>
Tensor<T> Tensor<T>::operator*=(T const& a) { // Multiplication: element-wise with broadcast
    int size = m_ch * m_psumDim * m_row * m_col;
    for (int i = 0; i < size; i++) {
        m_buffer[i] *= a;
    }
    return *this;
}

template <typename T>
Tensor<T> Tensor<T>::operator+=(Tensor const& p) { // Addition: element-wise
    if (!(m_ch == p.m_ch && m_psumDim == p.m_psumDim && m_row == p.m_row && m_col == p.m_col)) {
        throw "Error: Input tensor dimension does not match expected value";
    }
    int size = m_ch * m_psumDim * m_row * m_col;
    for (int i = 0; i < size; i++) {
        m_buffer[i] += p.m_buffer[i];
    }
    return *this;
}

template <typename T>
Tensor<T> Tensor<T>::operator+=(T const& a) { // Addition: element-wise with broadcast
    int size = m_ch * m_psumDim * m_row * m_col;
    for (int i = 0; i < size; i++) {
        m_buffer[i] += a;
    }
    return *this;
}

template <typename T>
Tensor<T> Tensor<T>::operator-=(Tensor const& p) { // Subtraction: element-wise
    if (!(m_ch == p.m_ch && m_psumDim == p.m_psumDim && m_row == p.m_row && m_col == p.m_col)) {
        throw "Error: Input tensor dimension does not match expected value";
    }
    int size = m_ch * m_psumDim * m_row * m_col;
    for (int i = 0; i < size; i++) {
        m_buffer[i] -= p.m_buffer[i];
    }
    return *this;
}

template <typename T>
Tensor<T> Tensor<T>::operator-=(T const& a) { // Subtraction: element-wise with broadcast
    int size = m_ch * m_psumDim * m_row * m_col;
    for (int i = 0; i < size; i++) {
        m_buffer[i] -= a;
    }
    return *this;
}

// ------ Friend functions
template <typename T>
std::ostream & operator<<(std::ostream &out, const Tensor<T> &p) {
    for (int ch = 0; ch < p.GetCh(); ch++) {
        out << "Channel: " << ch << "\n";

        for (int r = 0; r < p.GetRow(); r++) {
            for (int c = 0; c < p.GetCol(); c++) {
                out << p(r, c, ch) << " ";
            }
            out << "\n";
        }

        out << "\n";
    }
    return out;
}

template <typename T>
Tensor<T> operator+(Tensor<T> const& t1, Tensor<T> const& t2) {
    if (!(t1.m_ch == t2.m_ch && t1.m_psumDim == t2.m_psumDim && t1.m_row == t2.m_row && t1.m_col == t2.m_col)) {
        throw "Error: Input tensor dimension does not match expected value";
    }
    Tensor<T> result(t1.m_ch, t1.m_psumDim, t1.m_row, t1.m_col);
    int size = t1.m_ch * t1.m_psumDim * t1.m_row * t1.m_col;
    for (int i = 0; i < size; i++) {
        result.m_buffer[i] = t1.m_buffer[i] + t2.m_buffer[i];
    }
    return result;
}

template <typename T>
Tensor<T> operator+(Tensor<T> const& t1, T const& a) {
    Tensor<T> result(t1.m_ch, t1.m_psumDim, t1.m_row, t1.m_col);
    int size = t1.m_ch * t1.m_psumDim * t1.m_row * t1.m_col;
    for (int i = 0; i < size; i++) {
        result.m_buffer[i] = t1.m_buffer[i] + a;
    }
    return result;
}

template <typename T>
Tensor<T> operator+(T const& a, Tensor<T> const& t2) {
    return (t2 + a);
}

template <typename T>
Tensor<T> operator-(Tensor<T> const& t1, Tensor<T> const& t2) {
    if (!(t1.m_ch == t2.m_ch && t1.m_psumDim == t2.m_psumDim && t1.m_row == t2.m_row && t1.m_col == t2.m_col)) {
        throw "Error: Input tensor dimension does not match expected value";
    }
    Tensor<T> result(t1.m_ch, t1.m_psumDim, t1.m_row, t1.m_col);
    int size = t1.m_ch * t1.m_psumDim * t1.m_row * t1.m_col;
    for (int i = 0; i < size; i++) {
        result.m_buffer[i] = t1.m_buffer[i] - t2.m_buffer[i];
    }
    return result;
}


template <typename T>
void Tensor<T>::ClampToLSB(int bw) {
    assert(bw < 64);
    int64_t max = (int64_t) pow(2, bw - 1) - 1;
    int64_t min = -max - 1;
    for (T& n: m_buffer) {
        if (((int64_t) n) > max) n = (T) max;
        else if (((int64_t) n) < min) n = (T) min;
    }
}

template <typename T>
void Tensor<T>::ClampToLSB(T& din, int bw) {
    assert(bw < 64);
    int64_t max = (int64_t) pow(2, bw - 1) - 1;
    int64_t min = -max - 1;
    if (((int64_t) din) > max) din = (T) max;
    else if (((int64_t) din) < min) din = (T) min;
}

template <typename T>
T& Tensor<T>::operator()(int chn, int psum, int r, int c) { // Function operator: Get a particular element
    if (!BoundaryCheck(chn, psum, r, c)) {
        throw "Error: ArrayIndexOutofBound inside Tensor";
    }
    return m_buffer[chn * m_psumDim * m_row * m_col + psum * m_row * m_col + r * m_col + c];
}

template <typename T>
void Tensor<T>::Resize(int ch, int psum, int r, int c){
    m_buffer.clear();
    m_row = r;
    m_psumDim = psum;
    m_col = c;
    m_ch = ch;
    m_buffer.resize(ch * psum * r * c, 0);
}

template <typename T>
void Tensor<T>::Set(int chn, int psum, int r, int c, T value) {
    if (!BoundaryCheck(chn, psum, r, c)) {
        throw "Error: ArrayIndexOutofBound inside Tensor";
    }
    m_buffer[chn * m_psumDim * m_row * m_col + psum * m_row * m_col + r * m_col + c] = value;
}

template <typename T>
T Tensor<T>::Get(int chn, int psum, int r, int c) const {
    if (!BoundaryCheck(chn, psum, r, c)) {
        throw "Error: ArrayIndexOutofBound inside Tensor";
    }
    T value;
    value = m_buffer[chn * m_psumDim * m_row * m_col + psum * m_row * m_col + r * m_col + c];
    return value;
}

template <typename T>
void Tensor<T>::DumpSeq(string name) {
    ofstream f;
    f.open(DUMPFILE_DIR + "/" + name + "_seq.txt", std::ios::out);
//    f << "ch, psum, row, col = " << m_ch << ", " << m_psumDim << ", " << m_row << ", " << m_col << endl;
    for (int i = 0; i < m_ch * m_psumDim * m_row * m_col; i++) {
        f << dec << m_buffer[i] << endl;
    }
    f.close();
}

template <typename T>
void Tensor<T>::DumpMatrix(string name) {
    ofstream f;
    f.open(DUMPFILE_DIR + "/" + name + "_matrix.txt", std::ios::out);
    f << "ch, psum, row, col = " << m_ch << ", " << m_psumDim << ", " << m_row << ", " << m_col << endl;
    for (int i = 0; i < m_ch; i++) {
//        f << "ch = " << i << endl;
        for (int j = 0; j < m_psumDim; j++) {
            f << "ch = " << i << "     psum = " << j << endl;
            for (int k = 0; k < m_row; k++) {
                for (int t = 0; t < m_col; t++) {
                    int idx = i * m_psumDim * m_row * m_col + j * m_row * m_col + k * m_col + t;
                    f << dec << m_buffer[idx] << " ";
                }
                f << endl;
            }
        }
    }
}



template class Tensor<int>;
template class Tensor<float>;
template class Tensor<int64_t>;