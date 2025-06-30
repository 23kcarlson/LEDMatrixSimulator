#include <iostream>
#include <fstream>
#include <iomanip>
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <chrono>
#include <cmath>
#include <Magick++.h>
constexpr int matrixX = 32;
constexpr int matrixY = 16;
sf::Color matrix[matrixX][matrixY] = {};

std::string hexToBinary(const std::string& hex) {
    std::unordered_map<char, std::string> hexMap = {
        {'0', "0000"}, {'1', "0001"}, {'2', "0010"}, {'3', "0011"},
        {'4', "0100"}, {'5', "0101"}, {'6', "0110"}, {'7', "0111"},
        {'8', "1000"}, {'9', "1001"}, {'A', "1010"}, {'B', "1011"},
        {'C', "1100"}, {'D', "1101"}, {'E', "1110"}, {'F', "1111"},
        {'a', "1010"}, {'b', "1011"}, {'c', "1100"}, {'d', "1101"},
        {'e', "1110"}, {'f', "1111"}
    };

    std::string binary;
    for (char c : hex) {
        if (hexMap.contains(c)) {
            binary += hexMap[c];
        } else {
            return "Invalid hex character";
        }
    }
    return binary;
}
std::string printCurrentTime(const std::string& format = "%H:%M") {
    const auto now = std::chrono::system_clock::now();
    const std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    const std::tm* parts = std::localtime(&now_c);
    std::ostringstream oss;
    oss << std::put_time(parts, format.c_str());
    return oss.str();
}

void rectangle(int x, int y, int width, int height, sf::Color color, bool fill = true) {
    for (int i = x; i < x + width; ++i) {
        for (int j = y; j < y + height; ++j) {
            if (i >= 0 && i < matrixX && j >= 0 && j < matrixY) {
                if (fill || i == x || i == x + width - 1 || j == y || j == y + height - 1) {
                    matrix[i][j] = color;
                }
            }
        }
    }
}

void circle(int x, int y, int radius, sf::Color color, bool fill = true) {
    for (int i = x - radius; i <= x + radius; ++i) {
        for (int j = y - radius; j <= y + radius; ++j) {
            if (i >= 0 && i < matrixX && j >= 0 && j < matrixY) {
                const int distSq = (i - x) * (i - x) + (j - y) * (j - y);
                if (fill) {
                    if (distSq <= radius * radius) {
                    matrix[i][j] = color;
                }
                } else {
                    if (distSq <= radius * radius && distSq >= (radius - 1) * (radius - 1)) {
                        matrix[i][j] = color;
                    }
                }
            }
        }
    }
}
void line(int x1, int y1, int x2, int y2, sf::Color color, int style =0) {
    const int dx = abs(x2 - x1);
    const int dy = abs(y2 - y1);
    int x = x1;
    int y = y1;
    const int xIncrement = (x2 > x1) ? 1 : -1;
    const int yIncrement = (y2 > y1) ? 1 : -1;
    if (dx >= dy) {
        int error = dx / 2;
        for (int i = 0; i <= dx; ++i) {
            if (x >= 0 && x < matrixX && y >= 0 && y < matrixY) {
                if (style == 0) { // Solid line
                    matrix[x][y] = color;
                } else if (style == 1) { // Dotted line
                    if (i % 2 == 0) {
                        matrix[x][y] = color;
                    }
                } else if (style == 2) { // Dashed line
                    if (i % 4 < 2) {
                        matrix[x][y] = color;
                    }
                }
            }
            error -= dy;
            if (error < 0) {
                y += yIncrement;
                error += dx;
            }
            x += xIncrement;
        }
    } else {
        int error = dy / 2;
        for (int i = 0; i <= dy; ++i) {
            if (x >= 0 && x < matrixX && y >= 0 && y < matrixY) {
                matrix[x][y] = color; 
            }
            error -= dx;
            if (error < 0) {
                x += xIncrement;
                error += dy;
            }
            y += yIncrement;
        }
    }
}
void triangle(int x1, int y1, int x2, int y2, int x3, int y3, sf::Color color) {
    line(x1, y1, x2, y2, color);
    line(x2, y2, x3, y3, color);
    line(x3, y3, x1, y1, color);
}
int drawChar(int x, int y, char c, sf::Color color,const std::string& fontFile = "font.bdf") {
    //load bdf file and draw character
    std::ifstream bdfFile(fontFile);
    if (!bdfFile.is_open()) {
        std::cerr << "Error opening font file. "  << std::endl;

        return 0;
    }
    std::string line;
    std::string lineBinary;
    int charWidth = 0;
    bool foundChar = false;
    while (std::getline(bdfFile, line)) {
        if (line.find("ENCODING") != std::string::npos && line.find( std::to_string(c)) != std::string::npos) {
            foundChar = true;
        } else if (foundChar && line.find("ENDCHAR") != std::string::npos) {
            break;
        } else if (foundChar) {
            if (line.find("BBX") != std::string::npos) {
                // Extract the character width from the BBX line
                std::istringstream iss(line);
                std::string token;
                iss >> token; // Skip "BBX"
                iss >> charWidth;
            }
            //ignore the first lines
            if (line.find("BITMAP") != std::string::npos || line.find("DWIDTH") != std::string::npos ||
                line.find("STARTCHAR") != std::string::npos || line.find("SWIDTH") != std::string::npos) {
                continue;}
            // Process the bitmap data
            lineBinary = hexToBinary(line);
            for (int i = 0; i < lineBinary.length(); ++i) {
                if (lineBinary[i] == '1') {
                    // Draw pixel at (x + i, y)
                    if (x + i >= 0 && x + i < matrixX && y >= 0 && y < matrixY) {
                        matrix[x + i][y] = color;
                    }
                }
            }
            y++;
        }
    }
    bdfFile.close();
    return charWidth;
}

void drawText(const int x, const int y, const std::string &text, const sf::Color color, const std::string& fontFile = "font.bdf") {
    int charWidth = drawChar(x, y, text[0], color, fontFile);
    for (int i = 1; i < text.length(); ++i) {
        charWidth = drawChar(x + i * charWidth, y, text[i], color,fontFile);
    }
}
void replaceColor(const sf::Color color, const sf::Color newColor) {
    for (auto & i : matrix) {
        for (sf::Color & j : i) {
            if (j == color) {
                j = newColor;
            }
        }
    }
}
/*
 *Direction:
 *0: Horizontal
 *1: Vertical
 *2: Diagonal (Top-Left to Bottom-Right)
 */
void replaceColorLinearGradient(int x1, int y1, int x2, int y2,sf::Color replaceColor ,sf::Color startColor, sf::Color endColor, int direction) {
    for (int i = x1; i <= x2; ++i) {
        for (int j = y1; j <= y2; ++j) {
            if (matrix[i][j] == replaceColor) {
                if (i >= x1 && i < x2 && j >= y1 && j < y2) {
                    float ratio = 0.0f;
                    if (direction == 0) { // Horizontal Gradient
                        ratio = static_cast<float>(i - x1) / (x2 - x1);
                    } else if (direction == 1) { // Vertical Gradient
                        ratio = static_cast<float>(j - y1) / (y2 - y1);
                    } else if (direction == 2) { // Diagonal Gradient
                        ratio = std::sqrt(std::pow(i - x1, 2) + std::pow(j - y1, 2)) /
                                std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2));
                    }
                    sf::Color newColor(
                        static_cast<sf::Uint8>(startColor.r + ratio * (endColor.r - startColor.r)),
                        static_cast<sf::Uint8>(startColor.g + ratio * (endColor.g - startColor.g)),
                        static_cast<sf::Uint8>(startColor.b + ratio * (endColor.b - startColor.b)),
                        static_cast<sf::Uint8>(startColor.a + ratio * (endColor.a - startColor.a))
                );
                    matrix[i][j] = newColor;
                }
            }
        }
    }
}
void replaceColorRadialGradient(int x, int y, int radius, sf::Color replaceColor, sf::Color startColor, sf::Color endColor) {
    for (int i = 0; i < matrixX; ++i) {
        for (int j = 0; j < matrixY; ++j) {
            if (matrix[i][j] == replaceColor) {
                float dist = std::sqrt(std::pow(i - x, 2) + std::pow(j - y, 2));
                if (dist <= radius) {
                    float ratio = dist / radius;
                    sf::Color newColor(
                        static_cast<sf::Uint8>(startColor.r + ratio * (endColor.r - startColor.r)),
                        static_cast<sf::Uint8>(startColor.g + ratio * (endColor.g - startColor.g)),
                        static_cast<sf::Uint8>(startColor.b + ratio * (endColor.b - startColor.b)),
                        static_cast<sf::Uint8>(startColor.a + ratio * (endColor.a - startColor.a))
                    );
                    matrix[i][j] = newColor;
                }
            }
        }
    }
}
void clearScreen() {
    for (auto & i : matrix) {
        for (sf::Color & j : i) {
            j = sf::Color::Black;
        }
    }
}
void drawImage(const std::string& imagePath, int x = 0, int y = 0) {
    try {
        Magick::Image image;
        image.read(imagePath);
        image.quantizeColorSpace(Magick::RGBColorspace);
        int imageWidth = image.columns();
        int imageHeight = image.rows();

        for (int i = 0; (i + x < matrixX && i <imageWidth); ++i) {
            for (int j = 0; (j + y < matrixY && j< imageHeight); ++j) {
                // Get the pixel color from the image
                Magick::ColorRGB pixelColor = image.pixelColor(x+i, y+j);
                // Convert Magick::ColorRGB to sf::Color
                sf::Color color(
                    static_cast<sf::Uint8>(pixelColor.red() * 255),
                    static_cast<sf::Uint8>(pixelColor.green() * 255),
                    static_cast<sf::Uint8>(pixelColor.blue() * 255)
                );
                matrix[i][j] = color;
            }
        }

    } catch (const Magick::Exception& e) {
        std::cerr << "Error loading image: " << e.what() << std::endl;
    }
}
int main() {
    Magick::InitializeMagick(nullptr);
    sf::RenderWindow window(sf::VideoMode(1200, 600), "Matrix");
    sf::View view(sf::FloatRect(0, 0, 1200, 600));
    window.setView(view);
    int padding = 2;
    int squareSize = ((window.getSize().x / matrixX - padding) < (window.getSize().y / matrixY- padding)) ?
                        (window.getSize().x / matrixX - padding) : (window.getSize().y / matrixY -padding);
    while (window.isOpen()) {

        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::Resized) {
                // Update the view to match the new window size
                view.setSize(event.size.width, event.size.height);

                squareSize = ((event.size.width / matrixX) < (event.size.height / matrixY)) ?
                             (event.size.width / matrixX - padding) : (event.size.height / matrixY - padding);

                view.setCenter(event.size.width / 2.f, event.size.height / 2.f);
                std::cout << "Window resized to: " << event.size.width << "x" << event.size.height << std::endl;
                std::cout << "Square Size: " << squareSize << std::endl;
                std::cout << "Matrix Size: " << (squareSize+padding) * 32 <<", " << (squareSize+padding) *16<< std::endl;
                window.setView(view);
            }
        }
        window.clear(sf::Color(10,10,10));


        clearScreen();
        //rectangle(3,5, 10, 5, sf::Color::Red,false);
        triangle(5, 5, 15, 5, 10, 15, sf::Color::Magenta);
        //circle(10, 10, 5, sf::Color::Green);
        circle(10, 10, 4, sf::Color::Blue, false);
        //line(0, 0, 31, 10, sf::Color::Yellow);
        drawText(3, 0, printCurrentTime(), sf::Color::White);
        //replaceColorLinearGradient(0,0, matrixX, matrixY, sf::Color::Black, sf::Color::Green, sf::Color::Red, 2);
        //replaceColorGradient(sf::Color::White, sf::Color::Green, sf::Color::Red, 3);
        replaceColorRadialGradient(5, 5, 5, sf::Color::White, sf::Color::Green, sf::Color::Red);
        //Draw Matrix
        drawImage("test.png");
        for (int i = 0; i < matrixX; ++i) {
            for (int j = 0; j < matrixY; ++j) {

                sf::RectangleShape square(sf::Vector2f(static_cast<float>(squareSize), static_cast<float>(squareSize)));
                square.setPosition(i * (squareSize+padding), j * (squareSize+padding));
                square.setFillColor(sf::Color(matrix[i][j]));
                window.draw(square);
               }
        }
        window.display();
    }

    return 0;
}