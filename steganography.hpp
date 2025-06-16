#ifndef HS_STEGANOGRAPHY_HPP
#define HS_STEGANOGRAPHY_HPP

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>


/**
 * \file steganography.hpp
 * \brief Header file containing declarations for steganography functions
 */



/**
 * \brief Converts a string message into a vector of bits
 * \param message The input string message to convert
 * \return Vector of boolean values representing the message in binary form
 */
std::vector<bool> messageToBits(const std::string& message);

/**
 * \brief Converts a vector of bits back into a string message
 * \param bits Vector of boolean values representing the message in binary form
 * \return The reconstructed string message
 */
std::string bitsToMessage(const std::vector<bool>& bits);

/**
 * \brief Embeds a message into an image using LSB (Least Significant Bit) method
 * \param imagePath Path to the input image
 * \param message The message to embed
 * \param stegoFileName Name of the output stego image file
 */
void embedLSB(const std::string& imagePath, const std::string& message, const std::string& stegoFileName);

/**
 * \brief Extracts a message from an image using LSB method
 * \param imagePath Path to the stego image
 * \param msgLen Length of the embedded message in bits
 */
void extractLSB(const std::string& imagePath, size_t msgLen);

/**
 * \addindex Calculates and displays the maximum message capacity for LSB method
 * \param imagePath Path to the input image
 */
void maxCapacityLSB(const std::string& imagePath);

/**
 * \brief Embeds a message into an image using QIM (Quantization Index Modulation) method
 * \param imagePath Path to the input image
 * \param message The message to embed
 * \param q Quantization step size
 * \param stegoFileName Name of the output stego image file
 */
void embedQIM(const std::string& imagePath, const std::string& message, int q, const std::string& stegoFileName);

/**
 * \brief Extracts a message from an image using QIM method
 * \param imagePath Path to the stego image
 * \param q Quantization step size used during embedding
 */
void extractQIM(const std::string& imagePath, int q);

/**
 * \brief Calculates and displays the maximum message capacity for QIM method
 * \param imagePath Path to the input image
 * \param q Quantization step size
 */
void maxCapacityQIM(const std::string& imagePath, int q);

/**
 * \brief Finds P (peak) and Z (zero) points in a channel's histogram
 * \param channel Input image channel
 * \param P Reference to store the peak point
 * \param Z Reference to store the zero point
 */
void findPZ(const cv::Mat& channel, int& P, int& Z);

/**
 * \brief Shifts the histogram of a channel for embedding
 * \param channel Image channel to modify
 * \param P Peak point
 * \param Z Zero point
 */
void shiftHistogram(cv::Mat& channel, int P, int Z);

/**
 * \brief Reverses the histogram shift for extraction
 * \param channel Image channel to modify
 * \param P Peak point
 * \param Z Zero point
 */
void unshiftHistogram(cv::Mat& channel, int P, int Z);

/**
 * \brief Embeds a message into an image using Histogram Shifting method
 * \param imagePath Path to the input image
 * \param message The message to embed
 * \param stegoFileName Name of the output stego image file
 */
void embedHS(const std::string& imagePath, const std::string& message, const std::string& stegoFileName);

/**
 * \brief Extracts a message from an image using Histogram Shifting method
 * \param imagePath Path to the stego image
 * \param P_r Peak point for red channel
 * \param Z_r Zero point for red channel
 * \param P_g Peak point for green channel
 * \param Z_g Zero point for green channel
 * \param P_b Peak point for blue channel
 * \param Z_b Zero point for blue channel
 */
void extractHS(const std::string& imagePath, int P_r, int Z_r, int P_g, int Z_g, int P_b, int Z_b);

/**
 * \brief Calculates and displays the maximum message capacity for Histogram Shifting method
 * \param imagePath Path to the input image
 */
void maxCapacityHS(const std::string& imagePath);

/**
 * \brief Embeds a message into an image using PM1 (Plus-Minus One) method
 * \param imagePath Path to the input image
 * \param message The message to embed
 * \param stegoFileName Name of the output stego image file
 */
void embedPM1(const std::string& imagePath, const std::string& message, const std::string& stegoFileName);

/**
 * \brief Extracts a message from an image using PM1 method
 * \param imagePath Path to the stego image
 * \param msgLen Length of the embedded message in bits
 */
void extractPM1(const std::string& imagePath, size_t msgLen);

/**
 * \brief Calculates and displays the maximum message capacity for PM1 method
 * \param imagePath Path to the input image
 */
void maxCapacityPM1(const std::string& imagePath);

/**
 * \brief Prompts user to input an image path
 * \param imagePath Reference to store the input image path
 */
void inputImagePath(std::string& imagePath);

/**
 * \brief Prompts user to input a message
 * \param message Reference to store the input message
 */
void inputMessage(std::string& message);

/**
 * \brief Prompts user to input an output file name
 * \param fileName Reference to store the output file name
 */
void inputOutputFileName(std::string& fileName);

/**
 * \brief Prompts user to input Histogram Shifting parameters
 * \param P_r Reference to store red channel peak point
 * \param Z_r Reference to store red channel zero point
 * \param P_g Reference to store green channel peak point
 * \param Z_g Reference to store green channel zero point
 * \param P_b Reference to store blue channel peak point
 * \param Z_b Reference to store blue channel zero point
 */
void inputHSParams(int& P_r, int& Z_r, int& P_g, int& Z_g, int& P_b, int& Z_b);

/**
 * \brief Enumeration of available steganography methods
 */
enum class Method { LSB = 1, HS = 2, QIM = 3, PM1 = 4 };

/**
 * \brief Runs the LSB steganography workflow
 */
void runLSB();

/**
 * \brief Runs the Histogram Shifting steganography workflow
 */
void runHS();

/**
 * \brief Runs the QIM steganography workflow
 */
void runQIM();

/**
 * \brief Runs the PM1 steganography workflow
 */
void runPM1();

#endif