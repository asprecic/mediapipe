#ifndef EXAMPLE_IMPL_H
#define EXAMPLE_IMPL_H

#include "example.h"
#include "absl/status/status.h"
#include "mediapipe/framework/calculator_framework.h"

namespace mediapipe {

class ExampleImpl : public Example {
public:
    ExampleImpl(){}
    ~ExampleImpl();

    absl::Status Init(const std::string& graph);

    uint8_t* Process(uint8_t* data, int width, int height) override;

private:
    mediapipe::CalculatorGraph m_graph;
    absl::StatusOr<OutputStreamPoller> m_poller;
    size_t m_frame_timestamp = 0;
};

}
#endif
