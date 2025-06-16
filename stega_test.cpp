#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "steganography.hpp"
#include <opencv2/opencv.hpp>
#include <fstream>
#include <filesystem>




TEST_CASE("messageToBits and bitsToMessage") {
    SUBCASE("Basic conversion") {
        std::string testMessage = "Hello, World!";
        std::vector<bool> bits = messageToBits(testMessage);
        std::string reconstructed = bitsToMessage(bits);
        CHECK(reconstructed == testMessage);
    }

    SUBCASE("Empty string") {
        std::string testMessage = "";
        std::vector<bool> bits = messageToBits(testMessage);
        std::string reconstructed = bitsToMessage(bits);
        CHECK(reconstructed == testMessage);
    }

    SUBCASE("Special characters") {
        std::string testMessage = "!@#$%^&*()";
        std::vector<bool> bits = messageToBits(testMessage);
        std::string reconstructed = bitsToMessage(bits);
        CHECK(reconstructed == testMessage);
    }
}




TEST_CASE("Histogram manipulation functions") {
    SUBCASE("findPZ with empty channel") {
        cv::Mat empty;
        int P, Z;
        findPZ(empty, P, Z);
        CHECK(P == 0);
        CHECK(Z == 0);
    }

    SUBCASE("findPZ with uniform channel") {
        cv::Mat uniform(100, 100, CV_8UC1, cv::Scalar(128));
        int P, Z;
        findPZ(uniform, P, Z);
        CHECK(P == 128);
        
        CHECK(Z != 128);
    }

    SUBCASE("findPZ with two peaks") {
        cv::Mat image(100, 100, CV_8UC1);
       
        for (int y = 0; y < 50; y++) {
            for (int x = 0; x < 100; x++) {
                image.at<uchar>(y, x) = 50;
            }
        }
        for (int y = 50; y < 100; y++) {
            for (int x = 0; x < 100; x++) {
                image.at<uchar>(y, x) = 200;
            }
        }
        int P, Z;
        findPZ(image, P, Z);
       
        CHECK((P == 50 || P == 200));
        
        CHECK(Z != P);
    }

    SUBCASE("shiftHistogram and unshiftHistogram") {
        cv::Mat image(100, 100, CV_8UC1);
        
        for (int y = 0; y < 100; y++) {
            for (int x = 0; x < 100; x++) {
                image.at<uchar>(y, x) = 100;
            }
        }
        
        int P = 100;
        int Z = 150;
        
        
        cv::Mat shifted = image.clone();
        shiftHistogram(shifted, P, Z);
        
        
        for (int y = 0; y < 100; y++) {
            for (int x = 0; x < 100; x++) {
                uchar pix = shifted.at<uchar>(y, x);
                if (pix > P && pix < Z) {
                    CHECK(pix == image.at<uchar>(y, x) + 1);
                }
            }
        }
        
        
        unshiftHistogram(shifted, P, Z);
        
       
        for (int y = 0; y < 100; y++) {
            for (int x = 0; x < 100; x++) {
                CHECK(shifted.at<uchar>(y, x) == image.at<uchar>(y, x));
            }
        }
    }
}

