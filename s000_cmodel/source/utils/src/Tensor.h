#ifndef NPU_TENSOR_H
#define NPU_TENSOR_H

#include <cstdint>
#include <vector>
#include <iostream>
/*****************************************************************************
*  Change History :                                                          *
*  <Date>     | <Version> | <Author>       | <Description>                   *
*----------------------------------------------------------------------------*
*  2022/10/09 |  1.0      | Ning           | add operation > < >= <=         *
*----------------------------------------------------------------------------*
*****************************************************************************/
template <typename T>
class Tensor {
private:
    int m_row;
    int m_psumDim;
    int m_col;
    int m_ch;

    std::vector<T> m_buffer;

private:
    bool BoundaryCheck(int chn, int psum, int r, int c);
    bool BoundaryCheck(int chn, int psum, int r, int c) const;

public:
    // Constructor and destructor
    Tensor() = default; // Default
    Tensor(int chn, int psum, int r, int c); // Constructor with parameter
    Tensor(const Tensor & p); // Copy constructor
    virtual ~Tensor() = default; // Destructor

    // Arithmetic operator overloading
    Tensor& operator=(Tensor const& p); // Assignment operator
    Tensor operator*=(Tensor const& p); // Multiplication: element-wise
    Tensor operator*=(T const& a); // Multiplication: element-wise with broadcast
    Tensor operator+=(Tensor const& p); // Addition: element-wise
    Tensor operator+=(T const& a); // Addition: element-wise with broadcast
    Tensor operator-=(Tensor const& p); // Subtraction: element-wise
    Tensor operator-=(T const& a); // Subtraction: element-wise with broadcast

    // Friend functions
    friend std::ostream & operator<<(std::ostream & out, Tensor const& p);
    friend Tensor<T> operator+(Tensor const& t1, Tensor const& t2);
    friend Tensor<T> operator+(Tensor const& t1, T const& a);   // with broadcast
    friend Tensor<T> operator+(T const& a, Tensor const& t2);   // with broadcast
    friend Tensor<T> operator-(Tensor const& t1, Tensor const& t2);

    // Fix point related operation
    void ClampToLSB(int bw=16);
    static void ClampToLSB(T& din, int bw);

    // Accessor and Modifier
    T& operator()(int ch, int psum, int r, int c); // Function operator: Get a particular element as lvalue
    int GetSize() const {return m_row * m_ch * m_col * m_psumDim;}
    int GetRow() const {return m_row;}
    int GetCol() const {return m_col;}
    int GetCh() const {return m_ch;}
    int GetPsumDim() const {return m_psumDim;}
    void Resize(int ch, int psum, int r, int c);
    void Set(int chn, int psum, int r, int c, T value);
    T Get(int chn, int psum, int r, int c) const;
    void DumpSeq(std::string name);
    void DumpMatrix(std::string name);


};

#endif //NPU_TENSOR_H
