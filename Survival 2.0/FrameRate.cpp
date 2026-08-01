#include "FrameRate.h"
#include <iostream>
#include <string>


FrameRate::FrameRate() : timer(0.0f), frameCount(0) {
    std::cout << "FrameRate system initialized.\\n";
}


FrameRate::~FrameRate() {
    std::cout << "FrameRate system destroyed.\\n";
}


void FrameRate::Initialize() {
    timer = 0.0f;
    frameCount = 0;
}


void FrameRate::Load() {
    if (font.loadFromFile("Assets/Fonts/arial.ttf")) {
        std::cout << "Arial.ttf font loaded successfully.\\n";
        text.setFont(font);
        text.setCharacterSize(16);
        text.setFillColor(sf::Color::White);
        text.setString("");
        text.setPosition(10.f, 10.f);
    }
    else {
        std::cout << "Failed to load Arial.ttf font.\\n";
    }
}


void FrameRate::Update(float deltaTime) {
    timer += deltaTime;
    frameCount++;

    if (timer >= 1000.0f) {
        float fps = static_cast<float>(frameCount) / (timer / 1000.0f);
        std::string frameRateString = "FPS: " + std::to_string(static_cast<int>(fps)) +
            " | Frame Time: " + std::to_string(static_cast<int>(deltaTime)) + " ms";
        text.setString(frameRateString);
        timer = 0.0f;
        frameCount = 0;
    }
}

void FrameRate::Draw(sf::RenderWindow& window) {
    window.draw(text);
}