#pragma once

class KinectDevice : public Freenect::FreenectDevice {
public:
    KinectDevice(freenect_context* ctx, int index)
        : Freenect::FreenectDevice(ctx, index),
          _video_buffer(freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_RGB).bytes),
          _depth_buffer(freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_REGISTERED).bytes / 2) {
        setDepthFormat(FREENECT_DEPTH_REGISTERED);
    }

    void VideoCallback(void* rgb, uint32_t) override {
        std::lock_guard lock(_rgb_mutex);
        auto* src = static_cast<uint8_t*>(rgb);
        std::copy(src, src + getVideoBufferSize(), _video_buffer.begin());
        _new_rgb_frame = true;
    }
    void DepthCallback(void* depth, uint32_t) override {
        std::lock_guard lock(_depth_mutex);
        auto* src = static_cast<uint16_t*>(depth);
        std::copy(src, src + getDepthBufferSize() / 2, _depth_buffer.begin());
        _new_depth_frame = true;
    }

    auto get_rgb(std::vector<uint8_t>& out) -> bool {
        std::lock_guard lock(_rgb_mutex);
        if (!_new_rgb_frame) return false;
        out.swap(_video_buffer);
        _new_rgb_frame = false;
        return true;
    }
    auto get_depth(std::vector<uint16_t>& out) -> bool {
        std::lock_guard lock(_depth_mutex);
        if (!_new_depth_frame) return false;
        out.swap(_depth_buffer);
        _new_depth_frame = false;
        return true;
    }

private:
    std::mutex _rgb_mutex;
    std::mutex _depth_mutex;
    std::vector<uint8_t> _video_buffer;
    std::vector<uint16_t> _depth_buffer;
    bool _new_rgb_frame = false;
    bool _new_depth_frame = false;
};

struct KinectStream {
    void init(int index = 0) {
        _device = &_freenect.createDevice<KinectDevice>(index);
        _device->startVideo();
        _device->startDepth();
    }

    void destroy() {
        if (_device == nullptr) return;
        _device->stopDepth();
        _device->stopVideo();
        _device = nullptr;
    }

    auto fetch(std::vector<uint8_t>& rgb, std::vector<uint16_t>& depth) -> bool {
        bool has_rgb = _device != nullptr && _device->get_rgb(rgb);
        bool has_depth = _device != nullptr && _device->get_depth(depth);
        return has_rgb || has_depth;
    }

    Freenect::Freenect _freenect;
    KinectDevice* _device = nullptr;
};
