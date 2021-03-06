#pragma once

#include "caffe2/core/operator.h"

namespace caffe2 {

template <typename T>
struct FtrlParams {
  explicit FtrlParams(OperatorBase* op)
      : alphaInv(1.0 / op->GetSingleArgument<float>("alpha", 0.005)),
        beta(op->GetSingleArgument<float>("beta", 1.0)),
        lambda1(op->GetSingleArgument<float>("lambda1", 0.001)),
        lambda2(op->GetSingleArgument<float>("lambda2", 0.001)) {}
  T alphaInv;
  T beta;
  T lambda1;
  T lambda2;
};

// TODO(dzhulgakov): implement GPU version if necessary
template <typename T, class Context>
class FtrlOp final : public Operator<Context> {
 public:
  USE_OPERATOR_CONTEXT_FUNCTIONS;
  FtrlOp(const OperatorDef& operator_def, Workspace* ws)
      : Operator<Context>(operator_def, ws), params_(this) {}
  bool RunOnDevice() override;

 protected:
  FtrlParams<T> params_;
  INPUT_TAGS(VAR, N_Z, GRAD);
  OUTPUT_TAGS(OUTPUT_VAR, OUTPUT_N_Z);
};

template <typename T>
class SparseFtrlOp final : public Operator<CPUContext> {
 public:
  SparseFtrlOp(const OperatorDef& operator_def, Workspace* ws)
      : Operator<CPUContext>(operator_def, ws), params_(this) {}

  bool RunOnDevice() override {
    // Use run-time polymorphism
    auto& indices = Input(INDICES);
    if (indices.template IsType<int32_t>()) {
      DoRun<int32_t>();
    } else if (indices.template IsType<int64_t>()) {
      DoRun<int64_t>();
    } else {
      LOG(FATAL) << "Unsupported type of INDICES in SparseFtrlOp: "
                      << indices.meta().name();
    }
    return true;
  }

 protected:
  FtrlParams<T> params_;
  INPUT_TAGS(VAR, N_Z, INDICES, GRAD);
  OUTPUT_TAGS(OUTPUT_VAR, OUTPUT_N_Z);

 private:
  template <typename SIndex>
  void DoRun();
};

}
