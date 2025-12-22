#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem> // C++17 标准，如需兼容旧标准可使用 <experimental/filesystem> 或 boost

namespace fs = std::filesystem;

/**
 * @brief 压缩单张图片为JPEG格式
 * @param input_path 输入图片路径 (如: /path/to/image.png)
 * @param output_path 输出图片路径 (如: /path/to/image_compressed.jpg)
 * @param quality JPEG压缩质量 (0-100, 越高图像质量越好文件越大，默认85是良好平衡点)
 * @return 成功返回true，失败返回false
 */
bool compressImageToJPEG(const std::string& input_path, 
                         const std::string& output_path, 
                         int quality = 75) {
    // 1. 使用OpenCV读取图片
    cv::Mat image = cv::imread(input_path, cv::IMREAD_UNCHANGED); // 保留原始通道数
    
    if (image.empty()) {
        std::cerr << "错误：无法读取图片文件 '" << input_path << "'" << std::endl;
        return false;
    }
    
    // 可选：如果原图是带透明通道的PNG，需要转换（JPEG不支持透明）
    if (image.channels() == 4) {
        std::cout << "提示：输入图片带Alpha通道，将转换为RGB格式。" << std::endl;
        cv::cvtColor(image, image, cv::COLOR_BGRA2BGR);
    } else if (image.channels() == 1) {
        // 如果是灰度图，转换为BGR（JPEG标准）
        cv::cvtColor(image, image, cv::COLOR_GRAY2BGR);
    }
    
    // 2. 准备JPEG压缩参数
    std::vector<int> compression_params;
    compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
    compression_params.push_back(quality); // 质量参数
    // 可选：启用渐进式JPEG（网络加载体验更好）
    compression_params.push_back(cv::IMWRITE_JPEG_PROGRESSIVE);
    compression_params.push_back(1); // 1启用，0禁用
    // 可选：设置色度子采样（默认即可，进一步减小体积）
    // compression_params.push_back(cv::IMWRITE_JPEG_CHROMA_QUALITY);
    // compression_params.push_back(quality); // 通常与主质量一致或略低

    // 3. 保存为JPEG
    bool success = cv::imwrite(output_path, image, compression_params);
    
    if (success) {
        // 计算压缩率
        auto original_size = fs::file_size(input_path);
        auto compressed_size = fs::file_size(output_path);
        double ratio = (1.0 - (double)compressed_size / original_size) * 100.0;
        
        std::cout << "成功压缩: " << input_path 
                  << "\n  原始大小: " << original_size / 1024 << " KB"
                  << "\n  压缩大小: " << compressed_size / 1024 << " KB"
                  << "\n  压缩率: " << std::fixed << std::setprecision(1) << ratio << "%"
                  << "\n  输出: " << output_path 
                  << std::endl;
    } else {
        std::cerr << "错误：保存压缩图片失败 '" << output_path << "'" << std::endl;
    }
    
    return success;
}

/**
 * @brief 批量处理目录中的图片
 * @param input_dir 输入目录
 * @param output_dir 输出目录 (会自动创建)
 * @param quality 压缩质量
 * @param extensions 要处理的图片扩展名列表
 */
void batchCompressImages(const std::string& input_dir,
                         const std::string& output_dir,
                         int quality = 85,
                         const std::vector<std::string>& extensions = {".png", ".jpg", ".jpeg", ".bmp", ".tiff"}) {
    
    // 创建输出目录
    fs::create_directories(output_dir);
    
    int processed = 0, succeeded = 0;
    
    // 遍历输入目录
    for (const auto& entry : fs::directory_iterator(input_dir)) {
        if (!entry.is_regular_file()) continue;
        
        std::string input_path = entry.path().string();
        std::string ext = entry.path().extension().string();
        
        // 检查扩展名
        bool should_process = false;
        for (const auto& valid_ext : extensions) {
            if (strcasecmp(ext.c_str(), valid_ext.c_str()) == 0) {
                should_process = true;
                break;
            }
        }
        
        if (!should_process) continue;
        
        processed++;
        
        // 生成输出路径
        std::string filename = entry.path().stem().string() + "_compressed.jpg";
        std::string output_path = (fs::path(output_dir) / filename).string();
        
        // 压缩图片
        if (compressImageToJPEG(input_path, output_path, quality)) {
            succeeded++;
        }
    }
    
    std::cout << "\n批量处理完成: " << succeeded << "/" << processed << " 张图片处理成功" << std::endl;
}

int main(int argc, char** argv) {
    // 示例1：压缩单张图片
    // std::string input_image = "/home/jym/code/cpp/personal-project/gw-server/bin/web/uploads/home.png";
    // std::string output_image = "/home/jym/code/cpp/personal-project/gw-server/bin/web/uploads/home_compressed.jpg";
    
    // // 尝试不同的质量参数，找到最佳平衡点
    // std::vector<int> quality_levels = {90, 85, 75, 65};
    
    // for (int quality : quality_levels) {
    //     std::string specific_output = output_image;
    //     size_t dot_pos = specific_output.find_last_of(".");
    //     if (dot_pos != std::string::npos) {
    //         specific_output.insert(dot_pos, "_q" + std::to_string(quality));
    //     }
        
    //     std::cout << "\n=== 测试质量参数: " << quality << " ===" << std::endl;
    //     compressImageToJPEG(input_image, specific_output, quality);
    // }
    
    // 示例2：批量处理目录（取消注释使用）
    
    std::string input_dir = "/home/jym/code/cpp/personal-project/gw-server/bin/web/uploads/";
    std::string output_dir = "/home/jym/code/cpp/personal-project/gw-server/bin/web/uploads/compressed/";
    batchCompressImages(input_dir, output_dir, 85);
    
    
    return 0;
}