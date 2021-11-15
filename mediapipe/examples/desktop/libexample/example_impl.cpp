#include "example_impl.h"
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/file_helpers.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/port/status.h"
#include "mediapipe/framework/formats/image_frame.h"

namespace mediapipe {
constexpr char kInputStream[] = "input_video";
constexpr char kOutputStream[] = "output_video";

ExampleImpl::~ExampleImpl() 
{
    LOG(INFO) << "Shutting down.";
    absl::Status status = m_graph.CloseInputStream(kInputStream);
    if (status.ok()){
    	absl::Status status1 = m_graph.WaitUntilDone();
        if (!status1.ok()) {
            LOG(INFO) << "Error in WaitUntilDone(): " << status1.ToString();
        }
    } else {
        LOG(INFO) << "Error in CloseInputStream(): " << status.ToString();
    }
}

absl::Status ExampleImpl::Init(const std::string& graph) 
{
    LOG(INFO) << "Parsing graph config " << graph;
    mediapipe::CalculatorGraphConfig config = mediapipe::ParseTextProtoOrDie<mediapipe::CalculatorGraphConfig>(graph);

    LOG(INFO) << "Initialize the calculator graph.";
    MP_RETURN_IF_ERROR(m_graph.Initialize(config));

    LOG(INFO) << "Start running the calculator graph.";
    ASSIGN_OR_RETURN(m_poller, m_graph.AddOutputStreamPoller(kOutputStream));
    MP_RETURN_IF_ERROR(m_graph.StartRun({}));

    return absl::OkStatus();
}

uint8_t* ExampleImpl::Process(uint8_t* data, int width, int height) 
{
    if (data == nullptr){
        LOG(INFO) << __FUNCTION__ << " input data is nullptr!";
        return nullptr;
    }

    int width_step = width * ImageFrame::ByteDepthForFormat(ImageFormat::SRGB) * ImageFrame::NumberOfChannelsForFormat(ImageFormat::SRGB);

    auto input_frame_for_input = absl::make_unique<ImageFrame>(ImageFormat::SRGB, width, height, width_step, 
                                                                (uint8*)data, ImageFrame::PixelDataDeleter::kNone);

    m_frame_timestamp++;

    if (!m_graph.AddPacketToInputStream(kInputStream, mediapipe::Adopt(input_frame_for_input.release()).At(mediapipe::Timestamp(m_frame_timestamp))).ok()) {
        LOG(INFO) << "Failed to add packet to input stream. Call m_graph.WaitUntilDone() to see error (or destroy Example object)";
        return nullptr;
    }

    mediapipe::Packet packet;
    if (!m_poller->Next(&packet)){
        LOG(INFO) << "Poller didnt give me a packet, sorry. Call m_graph.WaitUntilDone() to see error (or destroy Example object). Error probably is that models are not available under mediapipe/models or mediapipe/modules";
        return nullptr;
    }

    const ImageFrame &output_frame = packet.Get<mediapipe::ImageFrame>();
    size_t output_bytes = output_frame.PixelDataSizeStoredContiguously();

    // This could be optimized to not copy but return output_frame.PixelData()
    uint8_t* out_data = new uint8_t[output_bytes];
    output_frame.CopyToBuffer((uint8*)out_data, output_bytes);
    return out_data; 
}

}
