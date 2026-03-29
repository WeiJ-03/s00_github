#ifndef NPU_FEATUREMAP_H
#define NPU_FEATUREMAP_H
#include "Tensor.h"
#include <string>
#include <vector>

template <class T>
class FeatureMap
{
public:
    bool valid_;
    std::string name_;
    int data_bits_;
    int row_;
    int col_;
    int psum_channels_;
    int channels_;
    std::vector<T> v_data_;
    std::vector<T> pv_data_;

    FeatureMap() = default;
    FeatureMap(std::string name);
    FeatureMap(const FeatureMap<T> & fm);
    virtual ~FeatureMap() = default;
    FeatureMap<T> & operator=(const FeatureMap<T> & fm);
    void Reset();
    void Resize(int ch, int p, int r, int c);
    void DumpMartix();

    Tensor<T> GetTensor();
};

#endif