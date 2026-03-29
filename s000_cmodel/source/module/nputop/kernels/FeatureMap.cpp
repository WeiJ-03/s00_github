#include "FeatureMap.h"
#include "Tensor.h"
#include <string>
#include <vector>

template <class T>
FeatureMap<T>::FeatureMap(std::string name){
    name_ = name;
}

template <class T>
FeatureMap<T>::FeatureMap(const FeatureMap<T> & fm){
    valid_ = fm.valid_;
    name_ = fm.name_;
    data_bits_ = fm.data_bits;
    row_ = fm.row_;
    col_ = fm.col_;
    psum_channels_ = fm.psum_channels_;
    channels_ = fm.channels_;
    v_data_ = fm.v_data_;
    pv_data_ = fm.pv_data_;
}

template <class T>
FeatureMap<T> & FeatureMap<T>::operator=(const FeatureMap<T> & fm){
    valid_ = fm.valid_;
    name_ = fm.name_;
    data_bits_ = fm.data_bits;
    row_ = fm.row_;
    col_ = fm.col_;
    psum_channels_ = fm.psum_channels_;
    channels_ = fm.channels_;
    v_data_ = fm.v_data_;
    pv_data_ = fm.pv_data_;
}

template <class T>
void FeatureMap<T>::Reset(){}

template <class T>
void FeatureMap<T>::Resize(int ch, int p, int r, int c){
    row_ = r;
    col_ = c;
    psum_channels_ = p;
    channels_ = c;
}

template <class T>
void FeatureMap<T>::DumpMartix(){}

template <class T>
Tensor<T> FeatureMap<T>::GetTensor(){};