#include "steganography.hpp"
#include <bitset>
#include <deque>
#include <algorithm>
#include <map>
#include <random>

/**
 * \file
 * \brief File, where steganography functions realised
 */



 
std::vector<bool> messageToBits(const std::string& message) {
    std::vector<bool> bits;
    for (char c : message) {
        std::bitset<8> b(static_cast<unsigned char>(c));
        for (int i = 7; i >= 0; --i)
            bits.push_back(b[i]);
    }
    return bits;
}

std::string bitsToMessage(const std::vector<bool>& bits) {
    std::string message;
    for (size_t i = 0; i + 7 < bits.size(); i += 8) {
        std::bitset<8> b;
        for (int j = 0; j < 8; ++j)
            b[7 - j] = bits[i + j]; 
        message += static_cast<char>(b.to_ulong());
    }
    return message;
}

void embedLSB(const std::string& imagePath, const std::string& message, const std::string& stegoFileName) {
    cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR);
    if (image.empty()) {
        std::cerr << "Ошибка загрузки изображения!\n";
        return;
    }
    std::vector<bool> bits = messageToBits(message);

    size_t capacity = image.rows * image.cols * image.channels(); 
    if (bits.size() > capacity) {
        std::cerr << "Сообщение слишком длинное для этого изображения! Максимум символов: " << (capacity / 8) << "\n";
        return;
    }

    cv::Mat stego = image.clone();
    size_t bitIdx = 0;
    for (int y = 0; y < stego.rows; ++y) {
        for (int x = 0; x < stego.cols; ++x) {
            cv::Vec3b& pix = stego.at<cv::Vec3b>(y, x);
            for (int c = 0; c < 3; ++c) {
                if (bitIdx < bits.size()) {
                    pix[c] = (pix[c] & ~1) | bits[bitIdx++];
                }
            }
        }
    }
    std::string stegoFile = "../" + stegoFileName;
    if (!cv::imwrite(stegoFile, stego)) {
        std::cerr << "Ошибка при сохранении изображения!\n";
        return;
    }
    std::cout << "Встраивание по LSB завершено! Файл сохранён в: " << stegoFileName << "\n";
    std::cout << "Длина встроенного сообщения: " << message.size() << " символов\n";
}

void extractLSB(const std::string& imagePath, size_t msgLen) {
    std::string stegoimage = "../" + imagePath;
    cv::Mat image = cv::imread(stegoimage, cv::IMREAD_COLOR);
    if (image.empty()) {
        std::cerr << "Ошибка загрузки изображения!\n";
        return;
    }
    size_t total_bits = msgLen * 8;
    std::vector<bool> bits;
    bits.reserve(total_bits);

    for (int y = 0; y < image.rows && bits.size() < total_bits; ++y) {
        for (int x = 0; x < image.cols && bits.size() < total_bits; ++x) {
            cv::Vec3b pix = image.at<cv::Vec3b>(y, x);
            for (int c = 0; c < 3 && bits.size() < total_bits; ++c) {
                bits.push_back(pix[c] & 1);
            }
        }
    }
    std::string message = bitsToMessage(bits);
    std::cout << "Извлечённое сообщение:\n" << message << "\n";
}

void maxCapacityLSB(const std::string& imagePath) {
    cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR);
    if (image.empty()) {
        std::cerr << "Ошибка загрузки изображения!\n";
        return;
    }
    size_t capacity = image.rows * image.cols * image.channels();
    size_t maxBytes = capacity / 8;
    std::cout << "Максимальная длина сообщения для LSB: " << maxBytes << " символов\n";
}


// ==== QIM ====
void embedQIM(const std::string& imagePath, const std::string& message, int q, const std::string& stegoFileName) {
    if (q % 2 != 0 || q < 2) {
        std::cerr << "Шаг квантования (q) должен быть чётным и >= 2!\n";
        return;
    }

    cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR);
    if (image.empty()) {
        std::cerr << "Ошибка загрузки изображения!\n";
        return;
    }
    int h = image.rows, w = image.cols;

    std::vector<bool> header_bits;
    int message_len = static_cast<int>(message.size());
    for (int i = 15; i >= 0; --i)
        header_bits.push_back((message_len >> i) & 1);    //не поняла разобрать 

    std::vector<bool> message_bits = messageToBits(message);
    std::vector<bool> all_bits = header_bits;
    all_bits.insert(all_bits.end(), message_bits.begin(), message_bits.end());

    int msg_index = 0;
    cv::Mat stego = image.clone();
    cv::Mat reshaped = stego.reshape(1, h * w);

    for (int i = 0; i < reshaped.rows && msg_index < (int)all_bits.size(); ++i) {
        cv::Vec3b& pix = reshaped.at<cv::Vec3b>(i, 0);
        for (int c = 0; c < 3 && msg_index < (int)all_bits.size(); ++c) {
            int m = all_bits[msg_index++] ? 1 : 0;
            int pixel_val = pix[c];
            int quantized = (pixel_val / q) * q + (q / 2) * m;
            pix[c] = cv::saturate_cast<uchar>(quantized);
        }
    }
    stego = reshaped.reshape(3, h);

    std::string stegoFile = "../" + stegoFileName;
    if (!cv::imwrite(stegoFile, stego)) {
        std::cerr << "Ошибка при сохранении изображения!\n";
        return;
    }
    std::cout << "Встраивание по QIM завершено! Файл сохранён в: " << stegoFileName << "\n";
    
}

void extractQIM(const std::string& imagePath, int q) {
    if (q % 2 != 0 || q < 2) {
        std::cerr << "Шаг квантования (q) должен быть чётным и >= 2!\n";
        return;
    }

    std::string stegoimage = "../" + imagePath;
    cv::Mat image = cv::imread(stegoimage, cv::IMREAD_COLOR);
    if (image.empty()) {
        std::cerr << "Ошибка загрузки изображения!\n";
        return;
    }
    int h = image.rows, w = image.cols;
    cv::Mat reshaped = image.reshape(1, h * w);

    std::vector<bool> bits;
    int msg_len = -1, total_bits = -1;
    std::string msg;

    for (int i = 0; i < reshaped.rows; ++i) {
        cv::Vec3b pix = reshaped.at<cv::Vec3b>(i, 0);
        for (int c = 0; c < 3; ++c) {
            int p = pix[c];
            int base = (p / q) * q;
            int p0 = base;
            int p1 = base + (q / 2);

            bits.push_back(std::abs(p - p0) < std::abs(p - p1) ? 0 : 1);

            if (msg_len == -1 && bits.size() >= 16) {
                
                msg_len = 0;
                for (int k = 0; k < 16; ++k)
                    msg_len = (msg_len << 1) | (bits[k] ? 1 : 0);
                total_bits = 16 + msg_len * 8;
            }
            if (total_bits != -1 && (int)bits.size() >= total_bits) {
                std::vector<bool> data_bits(bits.begin() + 16, bits.begin() + 16 + msg_len * 8);
                msg = bitsToMessage(data_bits);
                std::cout << "Извлечённое сообщение:\n" << msg << std::endl;
                return;
            }
        }
    }
    std::cerr << "Сообщение не найдено или изображение повреждено!\n";
}

void maxCapacityQIM(const std::string& imagePath, int q) {
    cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR);
    if (image.empty()) {
        std::cerr << "Ошибка загрузки изображения!\n";
        return;
    }
    int h = image.rows, w = image.cols;
    int capacity = h * w * 3; 
    int maxBytes = (capacity - 16) / 8; 
    std::cout << "Максимальная длина сообщения для QIM (q=" << q << "): " << maxBytes << " символов\n";
}


// ==== Histogram Shifting ====
void findPZ(const cv::Mat& channel, int& P, int& Z) {
    CV_Assert(channel.type() == CV_8UC1);
    if (channel.empty()) {
        std::cerr << "findPZ: канал пуст!\n";
        P = 0; Z = 0;
        return;
    }
    int hist[256] = {0};
    for (int y = 0; y < channel.rows; ++y)
        for (int x = 0; x < channel.cols; ++x)
            hist[channel.at<uchar>(y, x)]++;

    P = std::max_element(hist, hist + 256) - hist;

    
    int left = P - 1, right = P + 1;
    int Z_left = -1, Z_right = -1;
    while (left >= 0) {
        if (hist[left] == 0) { Z_left = left; break; }
        --left;
    }
    while (right < 256) {
        if (hist[right] == 0) { Z_right = right; break; }
        ++right;
    }
    
    if (Z_left == -1 && Z_right == -1) Z = 0;
    else if (Z_left == -1) Z = Z_right;
    else if (Z_right == -1) Z = Z_left;
    else Z = (abs(Z_left - P) < abs(Z_right - P)) ? Z_left : Z_right;
}

void shiftHistogram(cv::Mat& channel, int P, int Z) {
    CV_Assert(channel.type() == CV_8UC1);
    if (P == Z) return;
    if (P < Z) {
        for (int y = 0; y < channel.rows; ++y)
            for (int x = 0; x < channel.cols; ++x) {
                uchar& pix = channel.at<uchar>(y, x);
                if (pix > P && pix < Z && pix < 255)
                    ++pix;
            }
    } else {
        for (int y = 0; y < channel.rows; ++y)
            for (int x = 0; x < channel.cols; ++x) {
                uchar& pix = channel.at<uchar>(y, x);
                if (pix > Z && pix < P && pix > 0)
                    --pix;
            }
    }
}

void unshiftHistogram(cv::Mat& channel, int P, int Z) {
    CV_Assert(channel.type() == CV_8UC1);
    if (P == Z) return;
    if (P < Z) {
        for (int y = 0; y < channel.rows; ++y)
            for (int x = 0; x < channel.cols; ++x) {
                uchar& pix = channel.at<uchar>(y, x);
                if (pix > P && pix <= Z && pix > 0)
                    --pix;
            }
    } else {
        for (int y = 0; y < channel.rows; ++y)
            for (int x = 0; x < channel.cols; ++x) {
                uchar& pix = channel.at<uchar>(y, x);
                if (pix >= Z && pix < P && pix < 255)
                    ++pix;
            }
    }
}

void embedHS(const std::string& imagePath, const std::string& message, const std::string& stegoFileName) {
    cv::Mat img = cv::imread(imagePath, cv::IMREAD_COLOR);
    if (img.empty()) {
        std::cerr << "Ошибка загрузки изображения!\n";
        return;
    }

    
    std::vector<cv::Mat> channels;
    cv::split(img, channels);

    
    if (channels.size() != 3) {
        std::cerr << "Ошибка: изображение не содержит 3 цветовых каналов!\n";
        return;
    }
    cv::Size sz = channels[0].size();
    for (int c = 0; c < 3; ++c) {
        if (channels[c].size() != sz || channels[c].type() != CV_8UC1) {
            std::cerr << "Ошибка: каналы разного размера или типа!\n";
            return;
        }
    }
    
    std::vector<bool> bits = messageToBits(message);
    size_t bitIdx = 0;
    int cap_total = 0;

    int P[3], Z[3];

    for (int c = 0; c < 3; ++c) {
        findPZ(channels[c], P[c], Z[c]);
        shiftHistogram(channels[c], P[c], Z[c]);
      
        int pCount = 0;
        for (int y = 0; y < channels[c].rows; ++y)
            for (int x = 0; x < channels[c].cols; ++x)
                if (channels[c].at<uchar>(y, x) == P[c])
                    ++pCount;
        cap_total += pCount;
    }

    if (bits.size() > static_cast<size_t>(cap_total)) {
        std::cerr << "Сообщение слишком длинное для встраивания этим методом! Максимум символов: " << (cap_total / 8) << "\n";
        return;
    }

    for (int c = 0; c < 3; ++c) {
        for (int y = 0; y < channels[c].rows; ++y) {
            for (int x = 0; x < channels[c].cols; ++x) {
                uchar& pix = channels[c].at<uchar>(y, x);
                if (pix == P[c] && bitIdx < bits.size()) {
                  
                    if (bits[bitIdx++]) {
                        if (P[c] < Z[c] && pix < 255)
                            pix += 1;
                        else if (P[c] > Z[c] && pix > 0)
                            pix -= 1;
                    }
                    
                }
            }
        }
    }

    
    cv::Mat stego(sz, CV_8UC3);
    for (int y = 0; y < sz.height; ++y) {
        for (int x = 0; x < sz.width; ++x) {
            
            stego.at<cv::Vec3b>(y, x)[0] = channels[0].at<uchar>(y, x); 
            stego.at<cv::Vec3b>(y, x)[1] = channels[1].at<uchar>(y, x); 
            stego.at<cv::Vec3b>(y, x)[2] = channels[2].at<uchar>(y, x); 
        }
    }


    std::string stegoFile = "../" + stegoFileName;
    if (!cv::imwrite(stegoFile, stego)) {
        std::cerr << "Ошибка при сохранении изображения!\n";
        return;


    }
    std::cout << "Встраивание завершено (Histogram Shifting)! Файл сохранён в: " << stegoFileName << "\n";
    std::cout << "P и Z для встраивания (запишите для извлечения):\n";
    std::cout << "  R: " << P[2] << "/" << Z[2] << "\n";
    std::cout << "  G: " << P[1] << "/" << Z[1] << "\n";
    std::cout << "  B: " << P[0] << "/" << Z[0] << "\n";
    std::cout << "Длина встроенного сообщения: " << message.size() << " символов\n";
}

void extractHS(const std::string& imagePath, int P_r, int Z_r, int P_g, int Z_g, int P_b, int Z_b) {
    

    std::string stegoimage = "../" + imagePath;
    cv::Mat img = cv::imread(stegoimage, cv::IMREAD_COLOR);
    if (img.empty()) {
        std::cerr << "Ошибка загрузки изображения!\n";
        return;
    }

    
    std::vector<cv::Mat> channels;
    cv::split(img, channels);

    int P[3] = {P_b, P_g, P_r};
    int Z[3] = {Z_b, Z_g, Z_r};

    
    std::vector<bool> bits;
    for (int c = 0; c < 3; ++c) {
        for (int y = 0; y < channels[c].rows; ++y) {
            for (int x = 0; x < channels[c].cols; ++x) {
                uchar pix = channels[c].at<uchar>(y, x);
                if (P[c] < Z[c]) {
                    if (pix == P[c])
                        bits.push_back(0);
                    else if (pix == P[c] + 1)
                        bits.push_back(1);
                } else if (P[c] > Z[c]) {
                    if (pix == P[c])
                        bits.push_back(0);
                    else if (pix == P[c] - 1)
                        bits.push_back(1);
                }
            }
        }
    }

    
    
    std::cout << "Укажите длину сообщения (в символах, <= " << bits.size() / 8 << "): ";
    size_t msgLen;
    std::cin >> msgLen;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    size_t total_bits = msgLen * 8;

    if (total_bits > bits.size()) {
        std::cerr << "Ошибка: слишком большая длина сообщения!\n";
        return;
    }
    std::vector<bool> msgBits(bits.begin(), bits.begin() + total_bits);
    std::string message = bitsToMessage(msgBits);
    std::cout << "Извлечённое сообщение:\n" << message << std::endl;

    
}

void maxCapacityHS(const std::string& imagePath) {
    cv::Mat img = cv::imread(imagePath, cv::IMREAD_COLOR);
    if (img.empty()) {
        std::cerr << "Ошибка загрузки изображения!\n";
        return;
    }
    std::vector<cv::Mat> channels;
    cv::split(img, channels);
    int total = 0;
    int P[3], Z[3];
    for (int c = 0; c < 3; ++c) {
        findPZ(channels[c], P[c], Z[c]);
        int count = 0;
        for (int y = 0; y < channels[c].rows; ++y)
            for (int x = 0; x < channels[c].cols; ++x)
                if (channels[c].at<uchar>(y, x) == P[c])
                    ++count;
        total += count;
    }
    std::cout << "Максимальная длина сообщения для Histogram Shifting: " << (total / 8) << " символов\n";
}


// ==== PM1 (Plus-Minus One) ====
void embedPM1(const std::string& imagePath, const std::string& message, const std::string& stegoFileName) {
    cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR);
    if (image.empty()) {
        std::cerr << "Ошибка загрузки изображения!\n";
        return;
    }
    std::vector<bool> bits = messageToBits(message);
    size_t capacity = image.rows * image.cols * image.channels();
    if (bits.size() > capacity) {
        std::cerr << "Сообщение слишком длинное для этого изображения! Максимум символов: " << (capacity / 8) << "\n";
        return;
    }

    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> rnd(0, 1);

    cv::Mat stego = image.clone();
    size_t bitIdx = 0;
    for (int y = 0; y < stego.rows; ++y) {
        for (int x = 0; x < stego.cols; ++x) {
            cv::Vec3b& pix = stego.at<cv::Vec3b>(y, x);
            for (int c = 0; c < 3 && bitIdx < bits.size(); ++c) {
                uchar& val = pix[c];
                bool mi = bits[bitIdx++];
                if ((val % 2) != mi) {
                    int r = rnd(gen);
                    int delta = (r == 0) ? 1 : -1;
                    if ((delta == -1 && val > 0) || (delta == 1 && val < 255))
                        val = static_cast<uchar>(val + delta);
                    else
                        val = static_cast<uchar>(val - delta); // если граничное значение
                }
            }
        }
    }
    std::string stegoFile = "../" + stegoFileName;
    if (!cv::imwrite(stegoFile, stego)) {
        std::cerr << "Ошибка при сохранении изображения!\n";
        return;
    }
    std::cout << "Встраивание по PM1 завершено! Файл сохранён в: " << stegoFileName << "\n";
    std::cout << "Длина встроенного сообщения: " << message.size() << " символов\n";
}

void extractPM1(const std::string& imagePath, size_t msgLen) {
    cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR);
    if (image.empty()) {
        std::cerr << "Ошибка загрузки изображения!\n";
        return;
    }
    size_t total_bits = msgLen * 8;
    std::vector<bool> bits;
    bits.reserve(total_bits);

    for (int y = 0; y < image.rows && bits.size() < total_bits; ++y) {
        for (int x = 0; x < image.cols && bits.size() < total_bits; ++x) {
            cv::Vec3b pix = image.at<cv::Vec3b>(y, x);
            for (int c = 0; c < 3 && bits.size() < total_bits; ++c) {
                bits.push_back(pix[c] & 1);
            }
        }
    }
    std::string message = bitsToMessage(bits);
    std::cout << "Извлечённое сообщение:\n" << message << "\n";
}

void maxCapacityPM1(const std::string& imagePath) {
    cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR);
    if (image.empty()) {
        std::cerr << "Ошибка загрузки изображения!\n";
        return;
    }
    size_t capacity = image.rows * image.cols * image.channels();
    size_t maxBytes = capacity / 8;
    std::cout << "Максимальная длина сообщения для PM1: " << maxBytes << " символов\n";
}



// ==== Вспомогательные функции для пользовательского ввода ====
void inputImagePath(std::string& imagePath) {
    std::cout << "Введите путь к изображению: ";
    std::getline(std::cin >> std::ws, imagePath);
}

void inputMessage(std::string& message) {
    std::cout << "Введите сообщение для встраивания: ";
    std::getline(std::cin, message);
}

void inputOutputFileName(std::string& fileName) {
    std::cout << "Введите имя выходного изображения: ";
    std::getline(std::cin, fileName);
}

void inputHSParams(int& P_r, int& Z_r, int& P_g, int& Z_g, int& P_b, int& Z_b) {
    auto inputPZ = [](const char* prompt, int& P, int& Z) {
        std::string line;
        while (true) {
            std::cout << prompt;
            std::getline(std::cin, line);
            std::replace(line.begin(), line.end(), '/', ' ');
            std::istringstream iss(line);
            if (iss >> P >> Z) break;
            std::cout << "Ошибка ввода! Введите два числа через слэш (например, 255/0)\n";
        }
    };
    inputPZ("Введите P и Z для R-канала: ", P_r, Z_r);
    inputPZ("Введите P и Z для G-канала: ", P_g, Z_g);
    inputPZ("Введите P и Z для B-канала: ", P_b, Z_b);
}



void runLSB() {
    std::cout << "Выберите действие для LSB:\n";
    std::cout << " 1 - Встроить сообщение\n";
    std::cout << " 2 - Извлечь сообщение\n";
    std::cout << " 3 - Оценить вместимость\n";
    std::cout << "Ваш выбор: ";
    int choice;
    std::cin >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::string imagePath, stegoFileName, message;
    switch (choice) {
        case 1:
            inputImagePath(imagePath);
            inputMessage(message);
            inputOutputFileName(stegoFileName);
            embedLSB(imagePath, message, stegoFileName);
            break;
        case 2:
            inputImagePath(imagePath);
            size_t msgLen;
            std::cout << "Введите длину сообщения (в символах): ";
            std::cin >> msgLen;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            extractLSB(imagePath, msgLen);
            break;
        case 3:
            inputImagePath(imagePath);
            maxCapacityLSB(imagePath);
            break;
        default: std::cerr << "Неверный выбор.\n"; break;
    }
}

void runHS() {
    std::cout << "Выберите действие для метода Histogram Shifting:\n";
    std::cout << " 1 - Встроить сообщение\n";
    std::cout << " 2 - Извлечь сообщение\n";
    std::cout << " 3 - Оценить вместимость\n";
    std::cout << "Ваш выбор: ";
    int choice;
    std::cin >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::string imagePath, stegoFileName, message;
    switch (choice) {
        case 1:
            inputImagePath(imagePath);
            inputMessage(message);
            inputOutputFileName(stegoFileName);
            embedHS(imagePath, message, stegoFileName);
            break;
        case 2: {
            inputImagePath(imagePath);
            int P_r, Z_r, P_g, Z_g, P_b, Z_b;
            inputHSParams(P_r, Z_r, P_g, Z_g, P_b, Z_b);
            extractHS(imagePath, P_r, Z_r, P_g, Z_g, P_b, Z_b);
            break;
        }
        case 3:
            inputImagePath(imagePath);
            maxCapacityHS(imagePath);
            break;
        default: std::cerr << "Неверный выбор.\n"; break;
    }
}

void runQIM() {
    std::cout << "Выберите действие для QIM:\n";
    std::cout << " 1 - Встроить сообщение\n";
    std::cout << " 2 - Извлечь сообщение\n";
    std::cout << " 3 - Оценить вместимость\n";
    std::cout << "Ваш выбор: ";
    int choice;
    std::cin >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::string imagePath, stegoFileName, message;
    int q;
    switch (choice) {
        case 1:
            inputImagePath(imagePath);
            inputMessage(message);
            std::cout << "Введите шаг квантования (чётное число): ";
            std::cin >> q;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            inputOutputFileName(stegoFileName);
            embedQIM(imagePath, message, q, stegoFileName);
            break;
        case 2:
            inputImagePath(imagePath);
            std::cout << "Введите шаг квантования (тот же, что при встраивании): ";
            std::cin >> q;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            extractQIM(imagePath, q);
            break;
        case 3:
            inputImagePath(imagePath);
            std::cout << "Введите шаг квантования (чётное число): ";
            std::cin >> q;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            maxCapacityQIM(imagePath, q);
            break;
        default:
            std::cerr << "Неверный выбор.\n"; break;
    }
}

void runPM1() {
    std::cout << "Выберите действие для PM1:\n";
    std::cout << " 1 - Встроить сообщение\n";
    std::cout << " 2 - Извлечь сообщение\n";
    std::cout << " 3 - Оценить вместимость\n";
    std::cout << "Ваш выбор: ";
    int choice;
    std::cin >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::string imagePath, stegoFileName, message;
    switch (choice) {
        case 1:
            inputImagePath(imagePath);
            inputMessage(message);
            inputOutputFileName(stegoFileName);
            embedPM1(imagePath, message, stegoFileName);
            break;
        case 2:
            inputImagePath(imagePath);
            size_t msgLen;
            std::cout << "Введите длину сообщения (в символах): ";
            std::cin >> msgLen;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            extractPM1(imagePath, msgLen);
            break;
        case 3:
            inputImagePath(imagePath);
            maxCapacityPM1(imagePath);
            break;
        default: std::cerr << "Неверный выбор.\n"; break;
    }
}















